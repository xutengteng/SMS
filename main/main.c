#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "app_blufi.h"

#include "MQTT.h"
#include "GPIO.h"
#include "NVS.h"
#include "function.h"
#include "nvs_flash.h"

static const char *TAG = "APP";

void app_main(void)
{
    app_event_key  = xEventGroupCreate();
    app_comm_init();
    app_pub_beat();
    create_timer_tast();
    create_mqtt_info_manage_tast();
    
    while(1)
    {
        ESP_LOGI(TAG, "main");
        vTaskDelay(100);
    }
    
}
