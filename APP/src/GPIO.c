#include "function.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "GPIO.h"
// #include "MQTT.h"
#include "app_blufi.h"
// #include "esp_wifi.h"






#define Open_State 1
#define Wifi_State 2

#define KEY_PRES 	1		    //KEY按下
#define KEY_LONG 	2	        //KEY长按，长按时间大于3s



/*LED*/
void GPIO_LED_INIT(void)
{
    gpio_config_t led_conf; 

    led_conf.mode = GPIO_MODE_OUTPUT;		     // 配置gpio的模式
    led_conf.intr_type = GPIO_PIN_INTR_DISABLE;  // 失能中断 
    led_conf.pin_bit_mask = (1ULL << GPIO_LED);  // 配置GPIO_IN寄存器
    led_conf.pull_down_en = 0;                   // 下拉失能
    led_conf.pull_up_en = 0;                     // 上拉失能

    gpio_config(&led_conf);                      // 配置gpio参数，并使能

    // 设置gpio输出低电平：
    gpio_set_level(GPIO_LED, 0);
}
/*电压电流检测*/
void GPIO_CF1_INIT(void)
{
    gpio_config_t cf1_conf; 

    cf1_conf.mode = GPIO_MODE_OUTPUT;		 // 配置gpio的模式
    cf1_conf.intr_type = GPIO_PIN_INTR_DISABLE;  // 失能中断 
    cf1_conf.pin_bit_mask = (1ULL << GPIO_CF1);   // 配置GPIO_IN寄存器
    cf1_conf.pull_down_en = 0;                   // 下拉失能
    cf1_conf.pull_up_en = 0;                     // 上拉失能

    gpio_config(&cf1_conf);                      // 配置gpio参数，并使能

}
/*电量统计*/
void GPIO_CF_INIT(void)
{
    gpio_config_t cf_conf; 

    cf_conf.mode = GPIO_MODE_OUTPUT;		     // 配置gpio的模式
    cf_conf.intr_type = GPIO_PIN_INTR_DISABLE;  // 失能中断 
    cf_conf.pin_bit_mask = (1ULL << GPIO_CF);    // 配置GPIO_IN寄存器
    cf_conf.pull_down_en = 0;                   // 下拉失能
    cf_conf.pull_up_en = 0;                     // 上拉失能

    gpio_config(&cf_conf);                      // 配置gpio参数，并使能
}
/*过零检测*/
void GPIO_OVER_INIT(void)
{
    gpio_config_t over_conf; 

    over_conf.mode = GPIO_MODE_OUTPUT;		     // 配置gpio的模式
    over_conf.intr_type = GPIO_PIN_INTR_DISABLE;  // 失能中断 
    over_conf.pin_bit_mask = (1ULL << GPIO_OVER);  // 配置GPIO_IN寄存器
    over_conf.pull_down_en = 0;                   // 下拉失能
    over_conf.pull_up_en = 0;                     // 上拉失能

    gpio_config(&over_conf);                      // 配置gpio参数，并使能

    // 设置gpio为低电平：
    gpio_set_level(GPIO_OVER, 0);
}
/*电压电流切换*/
void GPIO_SEL_INIT(void)
{
    gpio_config_t sel_conf; 

    sel_conf.mode = GPIO_MODE_OUTPUT;		     // 配置gpio的模式
    sel_conf.intr_type = GPIO_PIN_INTR_DISABLE;  // 失能中断 
    sel_conf.pin_bit_mask = (1ULL << GPIO_SEL);   // 配置GPIO_IN寄存器
    sel_conf.pull_down_en = 1;                   // 下拉,优先低电平
    sel_conf.pull_up_en = 0;                     // 上拉失能

    gpio_config(&sel_conf);                      // 配置gpio参数，并使能

}
/*继电器控制*/
void GPIO_CTRL_INIT(void)
{
    gpio_config_t sel_conf; 

    sel_conf.mode = GPIO_MODE_OUTPUT;		     // 配置gpio的模式
    sel_conf.intr_type = GPIO_PIN_INTR_DISABLE;  // 失能中断 
    sel_conf.pin_bit_mask = (1ULL << GPIO_CTRL);   // 配置GPIO_IN寄存器
    sel_conf.pull_down_en = 0;                   // 下拉失能
    sel_conf.pull_up_en = 0;                     // 上拉失能

    gpio_config(&sel_conf);                      // 配置gpio参数，并使能

    // 设置gpio为低电平：
    gpio_set_level(GPIO_CTRL, 0);
}
/*异常断电检测*/
void GPIO_OFF_CHECK_INIT(void)
{
    gpio_config_t sel_conf; 

    sel_conf.mode = GPIO_MODE_INPUT;		     // 配置gpio的模式
    sel_conf.intr_type = GPIO_PIN_INTR_DISABLE;  // 失能中断 
    sel_conf.pin_bit_mask = (1ULL << GPIO_OFF_CHECK);   // 配置GPIO_IN寄存器
    sel_conf.pull_down_en = 0;                   // 下拉失能
    sel_conf.pull_up_en = 0;                     // 上拉失能

    gpio_config(&sel_conf);                      // 配置gpio参数，并使能

    // 设置gpio为低电平：
    gpio_set_level(GPIO_OFF_CHECK, 0);
}
/*按键*/
void GPIO_KEY_INIT(void)
{
    gpio_config_t sel_conf; 

    sel_conf.mode = GPIO_MODE_INPUT;		     // 配置gpio的模式
    sel_conf.intr_type = GPIO_PIN_INTR_DISABLE;  // 失能中断 
    sel_conf.pin_bit_mask = (1ULL << GPIO_KEY);  // 配置GPIO_IN寄存器
    sel_conf.pull_down_en = 0;                   // 
    sel_conf.pull_up_en = 1;                     // 上拉使能

    gpio_config(&sel_conf);                      // 配置gpio参数，并使能
}

/*继电器开*/
void relay_on()
{
    gpio_pad_select_gpio(GPIO_CTRL);                // 选择一个GPIO
    gpio_set_direction(GPIO_CTRL, GPIO_MODE_OUTPUT);// 把这个GPIO作为输出
    gpio_set_level(GPIO_CTRL, 1); 
    vTaskDelay(10);
}
/*继电器关*/
void relay_off()
{
    gpio_pad_select_gpio(GPIO_CTRL);                // 选择一个GPIO
    gpio_set_direction(GPIO_CTRL, GPIO_MODE_OUTPUT);// 把这个GPIO作为输出
    gpio_set_level(GPIO_CTRL, 0);
    vTaskDelay(10);
}

void LED_ON()
{
    gpio_pad_select_gpio(GPIO_LED);                // 选择一个GPIO
    gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);// 把这个GPIO作为输出
    gpio_set_level(GPIO_LED, 0); //低电平有效 led亮
    vTaskDelay(10);
}

void LED_OFF()
{
    gpio_pad_select_gpio(GPIO_LED);                // 选择一个GPIO
    gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);// 把这个GPIO作为输出
    gpio_set_level(GPIO_LED, 1);
    vTaskDelay(10);
}

static u_int8_t led_level = 1;

void LED_TOGGLE()
{
    gpio_pad_select_gpio(GPIO_LED);                // 选择一个GPIO
    gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);// 把这个GPIO作为输出
    gpio_set_level(GPIO_LED, led_level); //低电平有效 led亮
    led_level = (~led_level) & 0x01;
    vTaskDelay(10);
}

static void user_defined_delay(int time)
{
    vTaskDelay(time / portTICK_PERIOD_MS);
}



//此函数为按键扫描函数
//按键按下的结果，按键短按时返回KEY0_PRES,按下长按时间大于3S时返回KEY0_LONG_PRES
uint8_t KEY_Scan(void)
{	
    int key_value = gpio_get_level(GPIO_KEY);  
	static uint8_t key_up = 1;//按键按松开标志，按下时为1，松开时为0
	static u_int16_t keytimecount = 0;	  
	if(key_up && (key_value == 0))
	{     
		vTaskDelay(10 / portTICK_PERIOD_MS);//去抖动 
        key_value = gpio_get_level(GPIO_KEY); 
		if(key_value == 0) key_up=0;
	}
	else if(key_value == 1)
	{
		if(key_up == 0)
		{
			key_up = 1; 			
			if(keytimecount<10) // 短按
			{
				keytimecount=0;
				return KEY_PRES;
			}
			keytimecount = 0;
		}
	}
	if(key_up == 0)//如果按键按下则按10ms时间计时
	{
		vTaskDelay(1 / portTICK_PERIOD_MS);
		keytimecount++;
		if(keytimecount >= 30)//如果长按时间大于3s则返回按键长按,在此处设置按键长按时长
			return KEY_LONG;
	}
	return 0;// 无按键按下
}

static void led_task(void *arg)//联网闪烁  联网成功常亮
{
    int state = 1;
    while(1) //app_blufi_connected() 判断联网状态 bool 真 连上 
    {
        LED_ON();
        user_defined_delay(50);
        switch (state)
        {
            case Open_State:
                LED_OFF();
                user_defined_delay(150);
                if(app_blufi_connected()) //判断wifi是否连接成功 
                    state = Wifi_State;                     
                break; 
            case Wifi_State:
                LED_ON();
                if(app_blufi_connected() == false)  //判断是否断开 ，断开回到初始状态
                    state = Open_State;                                 
                break;      
            default:
                break;
        }
        //vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static void key_task(void *arg)//长按 3s 重启
{
    uint8_t key = 0;
    while(1)
    {		
        key=KEY_Scan();//获取按下的按键值
        if(key == KEY_LONG)
            esp_restart();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_phy_init()
{
    GPIO_CF1_INIT();/* 电压电流检测 */
    GPIO_CF_INIT(); /* 电量统计     */ 
    GPIO_SEL_INIT();/* 电压电流切换 */
    GPIO_KEY_INIT();/* 按键初始化   */
    GPIO_LED_INIT();/* LED初始化   */
}

void app_interact_task()
{
    xTaskCreatePinnedToCore(&led_task, "led_task", 4096, NULL, 4, NULL, 0);//LED 任务  任务正常 
        vTaskDelay(1);
    xTaskCreatePinnedToCore(&key_task, "key_task", 4096, NULL, 3, NULL, 0);//按键 任务  长按 3s 重启
        vTaskDelay(1);
}
