/* Non-Volatile Storage (NVS) Read and Write a Value - Example

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "NVS.h"
#include "MQTT.h"
#include "esp_log.h"

extern topic topic_t;

int8_t NVS_READ_INT8(const char* key)
{
    // Open
    int err;
    int8_t value = 0;
    printf("-------------R----------------\n");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Read
        
        err = nvs_get_i8(my_handle, key, &value);
        switch (err) {
            case ESP_OK:
                //printf("%s = %d\n", key,value);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                nvs_close(my_handle);
                return -1;
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
        // Close
        nvs_close(my_handle);
    }
    return  value;
}
void NVS_WRITE_INT8(const char* key,int8_t value)
{
    // Open
    int err;
    printf("-------------W----------------\n");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Write
        err = nvs_set_i8(my_handle, key, value);
        printf((err != ESP_OK) ? "Write Failed!\n" : "Write Done\n");
        err = nvs_commit(my_handle);

        // Close
        nvs_close(my_handle);
    }
}


int8_t NVS_READ_BLOB(const char* key)
{
    // Open
    int err;
    printf("-------------R_BLOB----------------\n");
    nvs_handle_t my_handle;
    ESP_LOGI("R_BLOB","key:%s",key);
    err = nvs_open("power", NVS_READONLY, &my_handle);
    ESP_LOGI("R_BLOB","nvs_open:%d",err);
    if (err != ESP_OK) return err;
    // Read the size of memory space required for blob
    size_t required_size = 0;  // value will default to 0, if not set yet in NVS
    err = nvs_get_blob(my_handle, key, NULL, &required_size);
    ESP_LOGI("R_BLOB","nvs_get_blob:%d",err);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    // Read previously saved blob if available
    uint8_t* value = malloc(required_size + sizeof(uint8_t));
    if (required_size > 0) {
        ESP_LOGI("R_BLOB","nvs_get_blob");
        err = nvs_get_blob(my_handle, key, value, &required_size);
        if (err != ESP_OK) {
            free(value);
            return err;
        }
        app_mqtt_client_publish(topic_t.PUB_TOPIC,(const char*)value,required_size,1);
    }
    // Close
    free(value);
    nvs_close(my_handle);
    return 0;
}

void NVS_WRITE_BLOB(const char* key,const void* value,int len)
{
    // Open
    int err;
    printf("-------------W_BLOB----------------\n");
    nvs_handle_t my_handle;
    err = nvs_open("power", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Write
        err = nvs_set_blob(my_handle, key, value,len);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        err = nvs_commit(my_handle);
        // Close
        nvs_close(my_handle);
    }
}


time_t NVS_READ_INT64(const char* key)
{
    // Open
    int err;
    time_t value = 0;
    printf("-------------R----------------\n");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Read
        
        err = nvs_get_i64(my_handle, key, &value);
        switch (err) {
            case ESP_OK:
                printf("%s = %ld\n", key,value);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                return -1;
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }
        // Close
        nvs_close(my_handle);
    }
    return  value;
}
void NVS_WRITE_INT64(const char* key,int64_t value)
{
    // Open
    int err;
    printf("-------------W----------------\n");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        // Write
        err = nvs_set_i64(my_handle, key, value);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        err = nvs_commit(my_handle);
        // Close
        nvs_close(my_handle);
    }
}