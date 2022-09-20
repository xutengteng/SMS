#ifndef __FUNCTION_H__
#define __FUNCTION_H__
 
#ifdef _cplusplus
extern "C"
{
#endif
   
#include "driver/gpio.h"

/*********************** 宏定义区 ***********************/
 
/*********************** enum 区 ************************/
 
/*********************** struct 区 **********************/
typedef struct dt
{
    uint8_t  value[20];
    uint8_t  byte;
    uint8_t  multiple[2];
}dt;

typedef struct power
{
	dt data_V;
	dt data_I;
	dt data_P;
	dt data_Q;
}power;

typedef struct freq
{
    uint16_t alarm;
    uint16_t nomral;
    uint16_t gether;
    uint16_t lbsstation;
    uint16_t lbsgps;
    uint16_t iccid;
}freq;

typedef struct {
     float data_V;
     float data_I;
     double data_P;
     float data_Q;
}Data_upload;

typedef struct {
    int16_t cf;
    int16_t cf_i;
    int16_t cf_u;
    int16_t mark;
} Pulse_data;

/*********************** 函数声明区 *********************/


/*继电器控制函数*/
void relay_on();
void relay_off();
/*任务初始化*/
void create_timer_tast();
/*初始化延时打开*/
void delay_on_create();

/*数据采集*/
void pulse_cnt_start(void);
int16_t pulse_count_cf_end();
int16_t pulse_count_cf1_end();
/*数据处理*/
//float Pow_calcu(int16_t CF);
//float Vol_calcu(int16_t CFU);
//float Cur_calcu(int16_t CFI);
//float Qua_calcu(int16_t P_value);
void create_mqtt_info_manage_tast();
#ifdef _cplusplus
}
#endif
 

#endif