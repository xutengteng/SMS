#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "driver/pcnt.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "function.h"
#include "GPIO.h"
static const char *TAG = "example";

/**
 * - GPIO36 -> 电量统计（CF）；
 * - GPIO37 -> 电压电流监测（CF1）；
 * - GPIO25 -> 电压电流切换（SEL）；
 */

xQueueHandle pcnt_evt_queue;   // A queue to handle pulse counter events

/* 初始化 PCNT 函数：
 * - 配置和初始化 PCNT
 * - 设置输入过滤器
 * - 监测电压电流脉冲
 */
static void pcnt_cf1_init(int unit)
{
    /* 准备 PCNT 单元的配置 */
    pcnt_config_t pcnt_config = {
        // 设置 PCNT 输入信号并控制 GPIO
        .pulse_gpio_num = GPIO_CF1,
        .channel = PCNT_CHANNEL_0,
        .unit = unit,
        // 在脉冲输入的正/负边沿上该怎么办？
        .pos_mode = PCNT_COUNT_INC,   // 在正边上计数
        .neg_mode = PCNT_COUNT_DIS,   // 将计数器值保持在负边上,负边数值保持不变
    };
    /* 初始化 PCNT 单元 */
    pcnt_unit_config(&pcnt_config);

    /* 配置和启用输入滤波器 */
    pcnt_set_filter_value(unit, 100);
    pcnt_filter_enable(unit);

    /* 初始化 PCNT 的计数器 */
    pcnt_counter_pause(unit);
    //pcnt_counter_clear(unit);

    /* 一切准备就绪，现在转到计数*/
    pcnt_counter_resume(unit);
}

static void pcnt_cf_init(int unit)
{
    /* 准备 PCNT 单元的配置 */
    pcnt_config_t pcnt_config = {
        // 设置 PCNT 输入信号并控制 GPIO
        .pulse_gpio_num = GPIO_CF,
        .channel = PCNT_CHANNEL_0,
        .unit = unit,
        // 在脉冲输入的正/负边沿上该怎么办？
        .pos_mode = PCNT_COUNT_INC,   // 在正边上计数
        .neg_mode = PCNT_COUNT_DIS,   // 将计数器值保持在负边上,负边数值保持不变
    };
    /* 初始化 PCNT 单元 */
    pcnt_unit_config(&pcnt_config);

    /* 配置和启用输入滤波器 */
    pcnt_set_filter_value(unit, 100);
    pcnt_filter_enable(unit);

    /* 初始化 PCNT 的计数器 */
    pcnt_counter_pause(unit);
    pcnt_counter_clear(unit);

    /* 一切准备就绪，现在转到计数*/
    pcnt_counter_resume(unit);
}

/*开始脉冲计数*/
void pulse_cnt_start(void)
{
    /* 初始化 PCNT 函数*/
    pcnt_cf1_init(PCNT_UNIT_1);
    pcnt_cf_init(PCNT_UNIT_0);
}

/*获取并清空CF脉冲计数*/
int16_t pulse_count_cf_end()
{
    int16_t count = 0;
    pcnt_get_counter_value(PCNT_UNIT_0, &count);
    ESP_LOGI(TAG, "CF Current counter value :%d", count); 
    pcnt_counter_clear(PCNT_UNIT_0);

    return count;
}

/*获取并清空CF1脉冲计数*/
int16_t pulse_count_cf1_end()
{
    int16_t count1 = 0;
    pcnt_get_counter_value(PCNT_UNIT_1, &count1);
    ESP_LOGI(TAG, "CF1 Current counter value :%d", count1); 

    pcnt_counter_clear(PCNT_UNIT_1);

    return count1;
}