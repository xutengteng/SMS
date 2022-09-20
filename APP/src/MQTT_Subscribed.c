#include "MQTT.h"
#include "mqtt_client.h"
#include "string.h"
#include "esp_log.h"
#include "GPIO.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <time.h>
#include <sys/time.h>
#include "function.h"
#include "NVS.h"

static const char *TAG = "COMM_SUB";

/*来自MQTT.c*/
extern topic topic_t;
extern char     res_message[200];//接受到的数据ascii
extern char     res_topic[76];//拆分出的主题
extern int      msg_len;
extern char     new_host[16];
extern int16_t  new_port;

/*来自Character.c*/
extern char  AsciiToHex(char * pAscii,char * pHex, int nLen);
extern int   HexToAscii(char *pHexStr, char *pAscStr,int Len);

/*静态变量*/
freq freq_t;
static uint8_t  data_hex[30] = {0};//hex数据
static char     frame_hex[60] = {0};//hex数据包

/*静态函数*/
static void  mqtt_cmd_sw();
static void  mqtt_cmd_ip();
static void  mqtt_cmd_frequence();

/*全局变量*/
EventGroupHandle_t app_event_key  = NULL;
const int EVENT_KEY_BIT = BIT0;
static int ip_cmd_len=0;

/**************************指令解析****************************/
void mqtt_subscribed_process(char* res_topic,char* res_message)
{
    char * sub_msg = res_message;

    int      i,j;
    uint8_t  check;
    uint8_t  cmd;            //指令
    uint16_t data_len  = 0;  //数据帧长
    uint16_t checksum  = 0;  //校验码

    ESP_LOGI(TAG, "sub_topic:%s",res_topic);

    /*【格式转换】*/
    //AsciiToHex(sub_msg,frame_hex, strlen(sub_msg));//"A115……" --> 0xA1 0x15……

    /*【复制数据】*/
    for(j=0;j<msg_len;j++)
    {
        frame_hex[j] = sub_msg[j];
    }
    /*【为订阅的主题】*/
    if(strstr(res_topic,"/set")!=NULL)
    {
        /*【________________________Message数据帧__________________________________】
        **【9E】 【BF】 【Length】 【Type】 【Cmd】 【Data】 【CheckSum】 【EE】 【BA】
        *						  【____________帧长计算范围__________】
        */
        
        /*【Length】*/

        data_len = (frame_hex[2]<<8) + frame_hex[3];
        ESP_LOGI(TAG, "data_len:%d",data_len);
        /*【CheckSum】*/
        checksum = frame_hex[data_len-1];
        check = data_len + 2 + 2 - 1;
        ip_cmd_len = data_len + 2 + 2 + 2;
        ESP_LOGI(TAG, "checksum:%d",checksum);
        /*【Data】*/
        memset(data_hex,0,30);
        for(i=0;i<data_len-3;i++)
        {
            data_hex[i] = frame_hex[i+6];
        }
        ESP_LOGI(TAG, "data:%s",data_hex);      

        /*【Cmd】*/
        cmd = frame_hex[5];
        ESP_LOGI(TAG, "cmd:%d",cmd);
            /***【开关】***/
            if(cmd == 0x15)
            {
                mqtt_cmd_sw();
            }
            /***【IP】****/
            if(cmd == 0x04)
            {
                frame_hex[check] += 0x02;
                mqtt_cmd_ip();
            }
            /***【频率】***/
            if(cmd == 0x01)
            {
                mqtt_cmd_frequence();
            }
    }
    memset(res_message,0,200);
    memset(res_topic,0,76);
    memset(frame_hex,0,60);
    data_len = 0;
}

/********开关控制********/
static void mqtt_cmd_sw()
{
    uint8_t channel;
    uint8_t state;

    struct tm open_time;
    //struct tm now_time;
    time_t t_now,t_open;
    long int t;

    channel = data_hex[9];
    state   = data_hex[10];

    switch(state)
    {
        case 0://关
            relay_off();
            NVS_WRITE_INT8(SWITCH_FLAG,0);
            ESP_LOGI(TAG, "---------close-------- \n");
            break;
        case 1://开
            relay_on();
            NVS_WRITE_INT8(SWITCH_FLAG,1);
            ESP_LOGI(TAG, "---------open-------- \n");
            break;
        case 2://预约开
            //预约时间
            open_time.tm_year = data_hex[11]+100;
            open_time.tm_mon  = data_hex[12];
            open_time.tm_mday = data_hex[13];
            open_time.tm_hour = data_hex[14];
            open_time.tm_min  = data_hex[15];
            open_time.tm_sec  = data_hex[16];
            t_open = mktime(&open_time);
            ESP_LOGI(TAG, "--------open_time:%ld S---------- \n",t_open);
            
            //xEventGroupSetBits(app_event_key, EVENT_KEY_BIT);//事件标志,按键使能

            // //当前时间
            // t_now = time(NULL); 
            // ESP_LOGI(TAG, "--------now:%ld S---------- \n",t_now);
            // //now_time = localtime(t_now);
            // xEventGroupSetBits(app_event_key, EVENT_KEY_BIT);//事件标志,按键使能

            // //n秒后开启
            // t = t_open - t_now; 
            // ESP_LOGI(TAG, "--------open after:%ld S---------- \n",t);
    
            //定时器中断
            //xEventGroupClearBits(app_event_key, EVENT_KEY_BIT);//按键失能
            NVS_WRITE_INT64("time",t_open);
            
            break;
        default:
            break;
    }
    char resp_hex[100];
    char resp_ascii[200];

    /*替换字段*/
    int i;
    for(i=0;i<26;i++){
        resp_hex[i] = frame_hex[i];
    }
    resp_hex[4] = 0xA2;//类型
    resp_hex[23] += 0x01;//校验
    /*格式转换*/
    HexToAscii(resp_hex,resp_ascii,26);
    ESP_LOGI(TAG, "--------resp_ascii:%s ---------- \n",resp_ascii);

    /*发布*/
    app_mqtt_client_publish(topic_t.SUB_RESP_TOPIC,resp_hex,26,1);

    memset(data_hex,0,30);
}
/*9EBF0022A1040AFFFFFFFFFFFFFFFF223139322E3137382E312E313638222232303030302268EEBA*/
/********端口配置********/
static void mqtt_cmd_ip()
{
    int i=0,j=0;
    uint8_t ip[16];
    uint8_t port[10];

    if(data_hex[9] == 0x22)//引号1
    {
        while(1)
        {
            if(data_hex[i+10] == 0x22)//引号2
            {
                ip[i] = '\0';
                break;
            }
            ip[i] = data_hex[i+10];
            i++;
        }
        ESP_LOGI(TAG, "ip:%s",ip);    
    }
    j = i;
    if(data_hex[i+11] == 0x22)//引号3
    {
        while(1)
        {
            if(data_hex[i+12] == 0x22)//引号4
            {
                port[i-j] = '\0';
                break;
            }
            port[i-j] = data_hex[i+12];
            i++;
        }
        ESP_LOGI(TAG, "port:%s",port);   
        //new_port = atoi(port);
    }
    char resp_hex[100];
    char resp_ascii[200];

    /*替换字段*/
    int s;
    for(s=0;s<50;s++){
        resp_hex[s] = frame_hex[s];
    }
    resp_hex[4] = 0xA2;//类型
    //resp_hex[23] += 0x01;//校验
    /*格式转换*/
    HexToAscii(resp_hex,resp_ascii,50);
    ESP_LOGI(TAG, "--------resp_ascii:%s ---------- \n",resp_ascii);

    /*发布*/
    app_mqtt_client_publish(topic_t.SUB_RESP_TOPIC,resp_hex,ip_cmd_len,1);  
    memset(data_hex,0,30);

    NVS_WRITE_INT8(IP_FLAG,1);//启用新IP

    app_mqtt_client_disconnect();//重启mqtt
}
/***********频率控制************/
static void mqtt_cmd_frequence()
{
    /*取值*/
    freq_t.alarm = (data_hex[9]<<8) + data_hex[10];
        ESP_LOGI(TAG, "alarm:%d",freq_t.alarm);
    freq_t.nomral = (data_hex[11]<<8) + data_hex[12];
    {
        NVS_WRITE_INT8(NOMRAL,freq_t.nomral);
        ESP_LOGI(TAG, "nomral:%d",freq_t.nomral);
    }
    freq_t.gether = (data_hex[13]<<8) + data_hex[14];
        ESP_LOGI(TAG, "gether:%d",freq_t.gether);
    freq_t.lbsstation = (data_hex[15]<<8) + data_hex[16];
        ESP_LOGI(TAG, "lbsstation:%d",freq_t.lbsstation);
    freq_t.lbsgps = (data_hex[17]<<8) + data_hex[18];
        ESP_LOGI(TAG, "lbsgps:%d",freq_t.lbsgps);
    freq_t.iccid = (data_hex[19]<<8) + data_hex[20];
        ESP_LOGI(TAG, "iccid:%d",freq_t.iccid);

    char resp_hex[100];
    char resp_ascii[200];
    /*替换字段*/
    int s;
    for(s=0;s<50;s++){
        resp_hex[s] = frame_hex[s];
    }
    resp_hex[4] = 0xA2;//类型
    resp_hex[27] += 0x02;//校验
    /*格式转换*/
    HexToAscii(resp_hex,resp_ascii,50);
    ESP_LOGI(TAG, "--------resp_ascii:%s ---------- \n",resp_ascii);

    /*发布*/
    app_mqtt_client_publish(topic_t.SUB_RESP_TOPIC,resp_hex,30,1);  
    memset(data_hex,0,30);
}


bool app_event_group_key()
{
    if(app_event_key == NULL){
        return false;
    }

    EventBits_t bits = xEventGroupWaitBits(app_event_key,EVENT_KEY_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           0);

    return (bits & EVENT_KEY_BIT) ? true : false;
}