#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/apps/sntp.h"
#include "esp_log.h"
#include "function.h"
#include "GPIO.h"
#include "NVS.h"
#include "MQTT.h"
extern int     mqtt_flag;
time_t preset_time = 0;
time_t local_time;

static void esp_get_now_time(void)
{
    time_t now = 0;
    int tmp = 0;

    while(now == 0){
        //printf("waiting for system time....(%d)\r\n",++tmp);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //time(&now);  //获得本地时间
        now = time(NULL);
    }
    
    local_time = now + 28800; //将时间转换为当地时间

    preset_time = NVS_READ_INT64("time");
    if(preset_time > local_time)
    {
        int t = preset_time - local_time;
        printf("t :【%d】\n",t);
        /*当本地时间在预约时间正负两秒之间且继电器未上电时上电*/
        if((local_time > preset_time-1)&& (local_time < preset_time+1) && (gpio_get_level(GPIO_CTRL) == 0)){  
            relay_on();
            preset_time = 0;
        }
    }
}

static void Upload_again_work()
{
    int cnt = 0;
    char key[4];
    if(mqtt_flag  == 1){
        cnt = NVS_READ_INT8("count");
        while(cnt > 0){
            printf("have data :【%d】\n",cnt);
            itoa(cnt,key,10);
            NVS_READ_BLOB(key);
            cnt--;
            NVS_WRITE_INT8("count",cnt);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

void delay_on_create()
{
    while(1){
        esp_get_now_time();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    //
}

void data_Upload_again()
{
    while(1){
         Upload_again_work();
         vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
    //
}

void create_mqtt_info_manage_tast()
{
    /*创建延时打开任务*/
    xTaskCreatePinnedToCore(&delay_on_create, "delay_open", 1024*4, NULL, 8, NULL, 0);
    vTaskDelay(1);
    /*创建断电上传任务*/
    xTaskCreatePinnedToCore(&data_Upload_again, "Upload_again", 1024*4, NULL, 8, NULL, 1);
}