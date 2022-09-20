#include "MQTT.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "function.h"
#include "NVS.h"
#include "esp_log.h"
#define HEAD 2
#define LENGTH 2
extern int     mqtt_flag;
static const char *TAG = "APP_PUB";

extern char deviceNo[8];
uint8_t Time[6];
char    data[100] = {0};

// 1字节，[Bit8]00000000[Bit1] 
// Bit1：为1时表示发生过温保护 
// Bit2：为1时表示发生漏电 
// Bit3：为1时表示发生过载 
// Bit4：为1时表示发生短路 
// Bit5：为1时表示发生缺少地线 
// Bit6：为1时表示发生RTC工作异常 
// Bit7：为1时表示发生零线与火线接反 
// Bit8：为1时表示发生缺少零线
uint8_t HARD_ALARM_FLAG = 0x00;
// Bit1：为1时表示限定保护（功率过大）
// Bit2：为1时表示设备拔出
// Bit3：为1时表示断电
// Bit4：保留
// Bit5-8：保留
uint8_t POWER_ALARM_FLAG = 0x00;
//保留
uint8_t OTHER_ALARM_FLAG = 0x00;

topic topic_t;
power power_t;

/*获取当前时间（开机已在初始化中校准时间）*/
void current_time()
{
    time_t tm_now;
    struct tm *local;

    tm_now = time(NULL);
    setenv("TZ", "CST-8", 1);
    tzset();
    local = localtime(&tm_now);
    Time[0] = local->tm_year % 100;
    Time[1] = local->tm_mon + 1;
    Time[2] = local->tm_mday;
    Time[3] = local->tm_hour;
    Time[4] = local->tm_min;
    Time[5] = local->tm_sec;
}
/***************************心跳上报****************************/
void mqtt_publish_beat()
{
    uint16_t offset = 0;
    uint8_t  i,checksum = 0;
    char  data_hex[100] = {0};
    //uint8_t  data_ascii[200] = {0};

    //【包头】
    data_hex[offset++] = 0x9E;
    data_hex[offset++] = 0xBF;

    //【Length】
    data_hex[offset++] = 0x00;
    data_hex[offset++] = 0x00;
    //【Type】
    data_hex[offset++] = 0x21;
    //【Cmd】
    data_hex[offset++] = 0x01;
    //【data_hex】
        //【模块】冷链模块 0x0A；其它：------ 
        data_hex[offset++] = 0x0A;
        //【设备id】
        data_hex[offset++] = deviceNo[0];
        data_hex[offset++] = deviceNo[1];
        data_hex[offset++] = deviceNo[2];
        data_hex[offset++] = deviceNo[3];
        data_hex[offset++] = deviceNo[4];
        data_hex[offset++] = deviceNo[5];
        data_hex[offset++] = deviceNo[6];
        data_hex[offset++] = deviceNo[7];
        //【"V1.01"】
        data_hex[offset++] = 0x22;//"
        data_hex[offset++] = 0x56;
        data_hex[offset++] = 0x31;
        data_hex[offset++] = 0x2E;
        data_hex[offset++] = 0x30;
        data_hex[offset++] = 0x31;
        data_hex[offset++] = 0x22;//"

        //【信号】dBm
        data_hex[offset++] = 0x20;
        //【设备状态信息】
        data_hex[offset++] = 0x01;
        //【电量】空格
        data_hex[offset++] = 0x20;

    /*【________________________Message数据帧__________________________________】
    **【9E】 【BF】 【Length】 【Type】 【Cmd】 【Data】 【CheckSum】 【EE】 【BA】
    **			   【_______校验和计算范围___________】
    */
    for(i=HEAD;i<(offset-HEAD);i++){
        checksum += data_hex[i];
    }
    data_hex[offset++] = checksum;
    /*【________________________Message数据帧__________________________________】
    **【9E】 【BF】 【Length】 【Type】 【Cmd】 【Data】 【CheckSum】 【EE】 【BA】
    *						  【____________帧长计算范围__________】
    */
    data_hex[HEAD+1] = offset - HEAD - LENGTH;

    //【包尾】
    data_hex[offset++] = 0xEE;
    data_hex[offset++] = 0xBA;

    //HexToAscii(data_hex,data_ascii,offset);
    //【发布】
    app_mqtt_client_publish(topic_t.BEAT_TOPIC,data_hex,offset,1);

}
/***************************预警上报****************************/
void mqtt_publish_warn(uint8_t alarm)
{
    uint16_t offset = 0;
    uint8_t  i,checksum = 0;
    char  data_hex[100] = {0};
    //uint8_t  data_ascii[200] = {0};

    //【包头】
    data_hex[offset++] = 0x9E;
    data_hex[offset++] = 0xBF;

    //【Length】
    data_hex[offset++] = 0x00;
    data_hex[offset++] = 0x00;
    //【Type】
    data_hex[offset++] = 0x21;
    //【Cmd】
    data_hex[offset++] = 0x17;
    //【data_hex】
        //【模块】冷链模块 0x0A；其它：------ 
        data_hex[offset++] = 0x0A;
        //【设备id】
        data_hex[offset++] = deviceNo[0];
        data_hex[offset++] = deviceNo[1];
        data_hex[offset++] = deviceNo[2];
        data_hex[offset++] = deviceNo[3];
        data_hex[offset++] = deviceNo[4];
        data_hex[offset++] = deviceNo[5];
        data_hex[offset++] = deviceNo[6];
        data_hex[offset++] = deviceNo[7];
        //【Time】
        current_time();
        data_hex[offset++] = Time[0];
        data_hex[offset++] = Time[1];
        data_hex[offset++] = Time[2];
        data_hex[offset++] = Time[3];
        data_hex[offset++] = Time[4];
        data_hex[offset++] = Time[5];
        //【报警】
        data_hex[offset++] =  0;
        data_hex[offset++] =  alarm;
        data_hex[offset++] =  0;

    //【Checksum】
    for(i=HEAD;i<(offset-HEAD);i++){
        checksum += data_hex[i];
    }
    data_hex[offset++] = checksum;

    data_hex[HEAD+1] = offset - HEAD - LENGTH;

    //【包尾】
    data_hex[offset++] = 0xEE;
    data_hex[offset++] = 0xBA;

    //HexToAscii(data_hex,data_ascii,offset);
    //【发布】

    int ret = app_mqtt_client_publish(topic_t.PUB_TOPIC,data_hex,offset,1);

}
/***************************数据上报****************************/
int mqtt_publish_data(power power_s)
{
    uint16_t offset = 0;
    uint8_t  i,checksum = 0;
    //uint8_t  data_ascii[200] = {0};

    //【包头】
    data[offset++] = 0x9E;
    data[offset++] = 0xBF;

    //【Length】
    data[offset++] = 0x00;
    data[offset++] = 0x00;
    //【Type】
    data[offset++] = 0x21;
    //【Cmd】
    data[offset++] = 0x13;
    //【Data】
        //【模块】冷链模块 0x0A；其它：------ 
        data[offset++] = 0x0A;
        //【设备id】
        data[offset++] = deviceNo[0];
        data[offset++] = deviceNo[1];
        data[offset++] = deviceNo[2];
        data[offset++] = deviceNo[3];
        data[offset++] = deviceNo[4];
        data[offset++] = deviceNo[5];
        data[offset++] = deviceNo[6];
        data[offset++] = deviceNo[7];
        //【Time】
        current_time();
        data[offset++] = Time[0];
        data[offset++] = Time[1];
        data[offset++] = Time[2];
        data[offset++] = Time[3];
        data[offset++] = Time[4];
        data[offset++] = Time[5];
        //【通道编码】
        data[offset++] = 1;//1字节；1：第一路
		
			//【采集数值1:电压】
			
			//【符号】
            data[offset++] = 0;//1字节；0：正，1：负，2：无意义，数值结束符
			//【扩大倍数】
            data[offset++] = power_s.data_V.multiple[0];//2字节；默认1，最大10000倍
			data[offset++] = power_s.data_V.multiple[1];
			//【字节数】
			data[offset++] = power_s.data_V.byte;
			//【数值】
			for(i=0;i<power_s.data_V.byte;i++)
			{
				data[offset++] = power_s.data_V.value[i];
			}
			
			//【采集数值2：电流】
			
			//【符号】
            data[offset++] = 0;
			//【扩大倍数】
            data[offset++] = power_s.data_I.multiple[0];
			data[offset++] = power_s.data_I.multiple[1];
			//【字节数】
			data[offset++] = power_s.data_I.byte;
			//【数值】
			for(i=0;i<power_s.data_I.byte;i++)
			{
				data[offset++] = power_s.data_I.value[i];
			}
			
			//【采集数值3：功率】
			//【符号】
            data[offset++] = 0;
			//【扩大倍数】
            data[offset++] = power_s.data_P.multiple[0];
			data[offset++] = power_s.data_P.multiple[1];
			//【字节数】
			data[offset++] = power_s.data_P.byte;
			//【数值】
			for(i=0;i<power_s.data_P.byte;i++)
			{
				data[offset++] = power_s.data_P.value[i];
			}
			
			//【采集数值4：累计用电量】
			//【符号】
            data[offset++] = 0;
			//【扩大倍数】
            data[offset++] = power_s.data_Q.multiple[0];
			data[offset++] = power_s.data_Q.multiple[1];
			//【字节数】
			data[offset++] = power_s.data_Q.byte;
			//【数值】
			for(i=0;i<power_s.data_Q.byte;i++)
			{
				data[offset++] = power_s.data_Q.value[i];
			}
			
    //【Checksum】
    for(i=HEAD;i<(offset-HEAD);i++){
        checksum += data[i];
    }
    data[offset++] = checksum;

    data[HEAD+1] = offset - HEAD - LENGTH;

    //【包尾】
    data[offset++] = 0xEE;
    data[offset++] = 0xBA;

    //HexToAscii(data,data_ascii,offset);
    //【发布】
    int ret = app_mqtt_client_publish(topic_t.PUB_TOPIC,data,offset,1);
    if(mqtt_flag == 0)//无网络时发布不出去，保存
    {
        int8_t cnt;
        ESP_LOGI(TAG, "pub failed!");
        char key[4];
        cnt = NVS_READ_INT8("count");//读数据数量
        if(cnt < 0)
        {
            cnt = 0;
        }
        ESP_LOGI(TAG, "all:%d",cnt);

        if(cnt > 0x3c) 
        {
            return 1;
        }
        cnt++;//下一个空间
        itoa(cnt,key,10);
        NVS_WRITE_BLOB(key,data,offset);//写
        NVS_WRITE_INT8("count",cnt);
    }
    return 1;
}

static void pub_task(void *arg)
{
    while(1){
        mqtt_publish_beat();//心跳
        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
    /*发布数据*/
}

/****发送心跳****/
void app_pub_beat()
{
    xTaskCreatePinnedToCore(&pub_task, "pub", 4096, NULL, 8, NULL, 0);
    vTaskDelay(1);
}