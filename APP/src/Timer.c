#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "driver/timer.h"
#include "function.h"
#include "MQTT.h"
#include "NVS.h"
#include "GPIO.h"

#define TIMER_DIVIDER         (80)  //硬件定时器时钟分频器
#define TIMER_SCALE            (TIMER_BASE_CLK / TIMER_DIVIDER)  // 将计数器值转换为秒
#define KEY1_SHORT_PRESS_EVENT (0x01<<1)

/*脉冲信息获取*/
Pulse_data data_acquisition;
/*脉冲信息处理*/
Data_upload data_collect;
/*数据上传结构体*/
power upload;

int sod = 0;
int task_mark;
uint8_t PW_ALARM_FLAG = 0x00;
EventGroupHandle_t event_task_mark = NULL;

/*插座状态判断*/
static void overcurrent_handle (int16_t tmp,int16_t tmp_v)
{
/*过流保护*/
    if(tmp > 6780){
        PW_ALARM_FLAG = 0x01;
        relay_off();//断开继电器
        mqtt_publish_warn(PW_ALARM_FLAG);
    }

/*设备拔出报警*/
    if(tmp == 0 || tmp == 1){
        PW_ALARM_FLAG = 0x02;
        mqtt_publish_warn(PW_ALARM_FLAG);
    }

/*插座断电报警*/
    if(tmp_v == 0){
        NVS_WRITE_INT8(KEY_OPEN,1);
        PW_ALARM_FLAG = 0x04;
        mqtt_publish_warn(PW_ALARM_FLAG);
    }   
}
int8_t nomral = 1;
/*数据上传*/
void delay_upload()
{
    printf("sod:%d\n",sod);
    if(sod >= nomral){
       printf("sod == nomral\n");
       mqtt_publish_data(upload);

        overcurrent_handle (data_acquisition.cf,data_acquisition.cf_u);//实现过流保护，
        nomral =  NVS_READ_INT8(NOMRAL);
        if(nomral < 0)//未设置
        {
            nomral = 60;
            printf("nomral:%d\n",nomral);
            NVS_WRITE_INT8(NOMRAL,60);
        }
        sod = 0;
    }
}

uint8_t byte_count(unsigned int dt)
{
    if(dt <= 0xff) return 1;
    if((dt > 0xff)&&(dt <= 0xffff)) return 2;
    if((dt > 0xffff)&&(dt <= 0xffffff)) return 3;
    if((dt > 0xffffff)&&(dt <= 0xffffffff)) return 4;

    return 0;
}

/*脉冲收集函数*/
static void Pulse_count()
{
    data_acquisition.cf = pulse_count_cf_end();//收集电量脉冲
    data_collect.data_P = data_acquisition.cf * 1.4601;//计算功率
    printf("get p: %lf \r\n",data_collect.data_P);

    int p;
    unsigned int data_p = (unsigned int)(data_collect.data_P * 10000);
    upload.data_P.byte = byte_count(data_p);
    upload.data_P.multiple[0] = 0x27;
    upload.data_P.multiple[1] = 0x10;
    for(p=0;p<upload.data_P.byte;p++){
        upload.data_P.value[upload.data_P.byte-p-1] = data_p>>(8*p) & 0XFF;
    }

    data_collect.data_Q = data_collect.data_P / 1000;//计算电能 
    printf("get Q: %f \r\n",data_collect.data_Q); 

    int q;
    unsigned int data_q = (unsigned int)(data_collect.data_Q * 10000);
    upload.data_Q.byte = byte_count(data_q);
    upload.data_Q.multiple[0] = 0x27;
    upload.data_Q.multiple[1] = 0x10;
    for(q=0;q<upload.data_Q.byte;q++){
        upload.data_Q.value[upload.data_Q.byte - q-1] = data_q>>(8*q) & 0XFF;
    }

    if(data_acquisition.mark == 0){
        data_acquisition.cf_i = pulse_count_cf1_end();
        printf("pulse I count:%d \r\n",data_acquisition.cf_i);
        gpio_set_level(GPIO_SEL, 1);//将GPIO_SEL拉高准备收集电压脉冲
        data_collect.data_I = data_acquisition.cf_i * 0.0118;
        printf("get I: %f \r\n",data_collect.data_I); 

        int i;
        unsigned int data_i = (unsigned int)(data_collect.data_I * 10000);
        upload.data_I.byte = byte_count(data_i);
        upload.data_I.multiple[0] = 0x27;
        upload.data_I.multiple[1] = 0x10;
        for(i=0;i<upload.data_I.byte;i++){
            upload.data_I.value[upload.data_I.byte - i-1] = data_i>>(8*i) & 0XFF;
        }
    }

	if(data_acquisition.mark == 1)
	{
        data_acquisition.cf_u = pulse_count_cf1_end();
        printf("pulse U count:%d \r\n",data_acquisition.cf_u);

        gpio_set_level(GPIO_SEL, 0);//将GPIO_SEL拉低准备收集电流脉冲

        data_collect.data_V = data_acquisition.cf_u * 0.1317;
		printf("get U: %f \r\n",data_collect.data_V);

        int u;
        unsigned int data_u = (unsigned int)(data_collect.data_V);
        upload.data_V.byte =  byte_count(data_u);
        upload.data_V.multiple[0] = 0x00;
        upload.data_V.multiple[1] = 0x01;
        for(u=0;u<upload.data_V.byte;u++){
            upload.data_V.value[upload.data_V.byte - u-1] = data_u>>(8*u) & 0XFF;
        }
    }

	data_acquisition.mark = 1 - data_acquisition.mark;//将标志翻转
}

/*创建事件组*/
void task_ev(void)
{
    event_task_mark = xEventGroupCreate();
    if(event_task_mark != NULL){
        printf("event ok!\r\n");
    }
    if(event_task_mark == NULL){
        printf("event error!\r\n");
    }
}

static bool IRAM_ATTR timer_group_isr_callback(void *args)
{
    sod++;
    xEventGroupSetBits(event_task_mark,KEY1_SHORT_PRESS_EVENT);
    return pdTRUE;
}

/**
 * @brief 初始化计时器组的选定计时器
 * @param 定时器组编号，索引从 0 开始
 * @param 定时器ID，索引从 0 开始
 * @param auto_reload 是否自动重新加载报警事件
 * @param timer_interval_sec 报警间隔
 */
static void example_tg_timer_init(int group, int timer, bool auto_reload, int timer_interval_sec)
{
    /* 选择并初始化计时器的基本参数 */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = auto_reload,
    };
    timer_init(group, timer, &config); //初始化定时器
    timer_set_counter_value(group, timer, 0); //设置计数起点
    timer_set_alarm_value(group, timer, timer_interval_sec * TIMER_SCALE); //设置报警值
    timer_enable_intr(group, timer); //使能中断

    timer_isr_callback_add(group, timer, timer_group_isr_callback, NULL, 0); //中断回调函数

    timer_start(group, timer); //开启定时器
}

/*任务处理函数*/
void pulse_capture(void)
{
    /*开启定时器并设置为1s报警*/
    example_tg_timer_init(TIMER_GROUP_0, TIMER_0, true, 1);

    /*开始脉冲计数*/
    pulse_cnt_start();

    while (1) {
        EventBits_t r_event = xEventGroupWaitBits(event_task_mark,KEY1_SHORT_PRESS_EVENT,pdTRUE,pdFALSE,3000/ portTICK_PERIOD_MS);

        if(r_event&KEY1_SHORT_PRESS_EVENT){

            /*获得脉冲计数*/
            Pulse_count();

            /*上传数据*/
            delay_upload();

        }
    }
}

 void pulse_capture_create()
 {
	while(1){
		pulse_capture();
        vTaskDelay(500 / portTICK_PERIOD_MS);
	}
	
 }

void create_timer_tast()
{
    /*将标志位置0*/
    data_acquisition.mark = 0;

    task_ev();

    if(NVS_READ_INT8(KEY_OPEN) == 1){
       relay_on(); 
       NVS_WRITE_INT8(KEY_OPEN,0);
    }
    /*创建任务*/
    xTaskCreatePinnedToCore(&pulse_capture_create, "timer_task", 1024*5, NULL, 8, NULL, 1);
}