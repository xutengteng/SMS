#include "MQTT.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "app_blufi.h"
#include "NVS.h"

#ifdef CONFIG_SOCKET_10A
    #define ACCOUNT CONFIG_SOCKET_ACCOUNT
    #define PASSWORD CONFIG_SOCKET_PASSWORD
#elif  CONFIG_SOCKET_16A
    #define ACCOUNT CONFIG_SOCKET_ACCOUNT
    #define PASSWORD CONFIG_SOCKET_PASSWORD
#endif

static const char *TAG = "COMM";
static const int CONNECTED_BIT = BIT0;
static const int EVENT_DATA_BIT = BIT0;

static esp_mqtt_client_handle_t client;
static EventGroupHandle_t mqtt_event_group = NULL;
static EventGroupHandle_t mqtt_event_data  = NULL;

esp_mqtt_event_handle_t data_event;

topic topic_t;

char    res_message[200];//接受到的数据
char    res_topic[76];//拆分出的主题
int     msg_len = 0;
int     mqtt_flag;
char    ProductKey[9];
char    deviceNo[8] = {0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x31};
char    new_host[16];
int16_t new_port;

/*主题组装*/
static void topic_init(const char *BE_CODE)
{
    printf("%s\n",BE_CODE);
    printf("%9.9s\n",BE_CODE);
    sprintf(topic_t.SUB_TOPIC,"/%9.9s/%s/user/set",BE_CODE,BE_CODE);
    sprintf(topic_t.SUB_RESP_TOPIC,"/%9.9s/%s/user/setResp",BE_CODE,BE_CODE);
    sprintf(topic_t.PUB_TOPIC,"/%9.9s/%s/user/update",BE_CODE,BE_CODE);
    sprintf(topic_t.PUB_RESP_TOPIC,"/%9.9s/%s/user/updateResp",BE_CODE,BE_CODE);
    sprintf(topic_t.BEAT_TOPIC,"/%9.9s/%s/user/heartbeat",BE_CODE,BE_CODE);
    sprintf(topic_t.BEAT_RESP_TOPIC,"/%9.9s/%s/user/heartbeatResp",BE_CODE,BE_CODE);
    strcpy(deviceNo,BE_CODE+9);
    printf("%s\n",topic_t.SUB_TOPIC);
    printf("%s\n",topic_t.PUB_TOPIC);
    printf("%s\n",topic_t.BEAT_TOPIC);
    
    // strcpy(topic_t.SUB_TOPIC,"/BE17C9M01/BE17C9M0100000000001/user/set");
    // strcpy(topic_t.SUB_RESP_TOPIC,"/BE17C9M01/BE17C9M0100000000001/user/setResp");
    // strcpy(topic_t.PUB_TOPIC,"/BE17C9M01/BE17C9M0100000000001/user/update");
    // strcpy(topic_t.PUB_RESP_TOPIC,"/BE17C9M01/BE17C9M0100000000001/user/updateResp");
    // strcpy(topic_t.BEAT_TOPIC,"/BE17C9M01/BE17C9M0100000000001/user/heartbeat");
    // strcpy(topic_t.BEAT_RESP_TOPIC,"/BE17C9M01/BE17C9M0100000000001/user/heartbeatResp");
}
/*MQTT事件回调*/
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            xEventGroupClearBits(mqtt_event_group, CONNECTED_BIT);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGI(TAG,"TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG,"DATA=%.*s", event->data_len, event->data);

            sprintf(res_topic,"%.*s",event->topic_len, event->topic);//主题
            memcpy(res_message,event->data,event->data_len);
            msg_len = event->data_len;
            xEventGroupSetBits(mqtt_event_data, CONNECTED_BIT);//事件标志

            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
            }
            break;
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    
    return ;
}

//esp_read_mac_ReturnThruPtr_mac(mac);

/**
关闭或跳过证书认证呢？
这里我们进入menuconfig界面-component config-ESP-TLS
勾选Allow potentially insecure options-Skip server certificate verification by default (WARNING: ONLY FOR TESTING PURPOSE, READ HELP)
原文链接：https://blog.csdn.net/xierbazi1/article/details/119873235 
*/
void app_mqtt_client_init(int parm)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri       = "mqtt://msgtest.haierbiomedical.com",//海尔生物医疗大数据平台
        .port      = 1777,                               // MQTT服务器端口
        .username  = ACCOUNT,
        .password  = PASSWORD,
        .client_id = ACCOUNT,
        //.host = "106.15.47.64", // MQTT服务器地址
        //.keepalive=60
        //.event_handle = mqtt_event_handler,
        //.cert_pem =  (const char *)server_cert_pem_start,
        //.client_cert_pem = (const char *)client_cert_pem_start,
        //.client_key_pem = NULL,
        //.ds_data = ds_data,
    };
    const esp_mqtt_client_config_t mqtt_new_cfg = {
        .host      = new_host,
        .port      = new_port,
        .username  = ACCOUNT,
        .password  = PASSWORD,
        .client_id = ACCOUNT,
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());

    if(parm == 0)
    {
        client = esp_mqtt_client_init(&mqtt_cfg);
    }else{
        client = esp_mqtt_client_init(&mqtt_new_cfg);
    }
    
    //esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);//注册MQTT事件。
   
    esp_err_t err = esp_mqtt_client_start(client); //启动客户端  //使用已创建的客户端句柄启动MQTT客户端。 参数：客户端– MQTT客户端句柄

    ESP_LOGI(TAG, "mqtt client[%p] created for connection, err[%d]... ", client, err);
}

static void app_mqtt_client_reconnect(void)
{
    /*char uri[64];
    sprintf(uri, "mqtts://%s:%d", host, port);
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = uri
    };
    esp_mqtt_set_config(client, &mqtt_cfg);
    */
    esp_mqtt_client_disconnect(client); //此 api 通常用于强制断开与代理的连接。
    esp_mqtt_client_reconnect(client);  //此 api 通常用于在特定事件时强制重新连接。
    ESP_LOGI(TAG, "reconnect");
}

void app_mqtt_client_disconnect()
{
    esp_mqtt_client_disconnect(client);
    ESP_LOGI(TAG, "disconnect");
}

int app_mqtt_client_publish(const char * topic, const char * publish_string,int len ,int qos)
{
    if(1){
        int msg_id = esp_mqtt_client_publish(client, topic, publish_string, len, qos, 0);
        if(msg_id < 0)
        {
            return -1;
            ESP_LOGI(TAG, "sent publish returned msg_id=%d", msg_id);
        }
        ESP_LOGI(TAG, "sent publish returned msg_id=%d", msg_id);
        return 1;
    }
    return 0;
}

void app_mqtt_client_subscribe(const char * topic, int qos)
{
    
    vTaskDelay(100);

    int msg_id = esp_mqtt_client_subscribe(client, topic, qos);//使用定义的 qos 为客户端订阅定义的主题(topic)。
    while(msg_id == -1) //函数返回：成功时返回订阅消息的 message_id   失败时 -1
    {
            ESP_LOGI(TAG, "msg_id = -1");
            vTaskDelay(100);
            if(client){  
                msg_id = esp_mqtt_client_subscribe(client, topic, qos);
            }
    }
    ESP_LOGI(TAG, "sent subscribe returned msg_id=%d", msg_id);

}

void app_mqtt_client_deinit()
{
    if (client) {
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
        client = NULL;
        ESP_LOGI(TAG, "mqtt client for connection destroyed");
    }
}

static bool app_mqtt_connected()
{
    if(mqtt_event_group == NULL){
        return false;
    }

    EventBits_t bits = xEventGroupWaitBits(mqtt_event_group,CONNECTED_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           0);

    return (bits & CONNECTED_BIT) ? true : false;
}
static bool app_mqtt_event_data()
{
    if(mqtt_event_data == NULL){
        return false;
    }

    EventBits_t bits = xEventGroupWaitBits(mqtt_event_data,EVENT_DATA_BIT, //事件组正在接受测试。 //事件组中要等待的位。
                                           pdFALSE, //事件组中设置的位不会更改。
                                           pdFALSE,//指定的块时间到期时返回。块时间由 xTicksToWait 参数指定。
                                           0);

    return (bits & EVENT_DATA_BIT) ? true : false;
}

static void comm_task(void *arg)
{
    static uint8_t steps = 0;
    bool first_in = true;
    int ret=0;
    int ip_flag = 0;

    mqtt_event_group = xEventGroupCreate();
    mqtt_event_data  = xEventGroupCreate();
    app_blufi_init();
    topic_init(ACCOUNT); 

    ip_flag = NVS_READ_INT8(IP_FLAG);//启用新IP
    if(ip_flag < 0)
    {
        ip_flag = 0;
    }
    if(ip_flag){
        first_in = true;
    }

    while(1){

        switch(steps)
        {
            case 0:
            if(app_blufi_connected()){
                ESP_LOGE(TAG, "-------blufi connect ok---------- \n");
                if(first_in){//第一次连接
                    ESP_LOGE(TAG, "-------mqtt connect---------- \n");
                    app_mqtt_client_init(ip_flag);
                    first_in = false;
                }else{//以后重连
                    app_mqtt_client_reconnect();
                    ESP_LOGE(TAG, "-------mqtt reconnect---------- \n");
                }
                steps = 1;
                vTaskDelay(100);

            }
            break;

            case 1:
            if(app_mqtt_connected()){
                ESP_LOGE(TAG, "-------mqtt connect ok---------- \n");
                mqtt_flag = 1;
                app_mqtt_client_subscribe(topic_t.SUB_TOPIC, 1);
                //app_mqtt_client_publish("/123456789/12345678/user/heartbeat","123",1); 
                steps = 2;
            }else{
                mqtt_flag = 0;
                steps = 0;
            }
            break;

            case 2:
            if(!app_mqtt_connected()){
                steps = 0;
                mqtt_flag = 0;
                ESP_LOGE(TAG, "-------mqtt disconnect ---------- \n");
            }

            break;    
            default:break;
        };
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
static void process_task(void *arg)
{
    while(1){
        if(app_mqtt_event_data()){
            ESP_LOGE(TAG, "----------Data_Event---------- \n");
            mqtt_subscribed_process(res_topic,res_message);
            
            xEventGroupClearBits(mqtt_event_data, EVENT_DATA_BIT); 
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
void app_comm_init()
{
    //const int cpuid_1 = portNUM_PROCESSORS - 1;
    xTaskCreatePinnedToCore(&comm_task, "comm", 4096*10, NULL, 8, NULL, 0);//联网任务
    vTaskDelay(1);
    xTaskCreatePinnedToCore(&process_task, "process", 4096*10, NULL, 8, NULL, 1);//解析任务
    vTaskDelay(1);
}
