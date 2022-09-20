# Smart socket Project


## Illustrate
MCU:ESP32

SDK:ESP-IDF4.4.1

IDE:VSCode

## Configure the project
Open the project configuration menu (`idf.py menuconfig`).

In the `Serial flasher config` menu:

* Set the `Flash size` to 4MB

In the `Component config` menu:

* Set the `Bluetooth` menu:

    * Set the `Bluedroid Options`

        * Set the `Include blufi function`

* Set the `ESP32-Specific` menu:

    * Unset the `Hardeare brownout detect&reset`

* Set the `APP Configuration` menu:
    * Set the `Device Configuration` menu:
        * Click to `Slect the type of the device.`,choice `16A` device,and config the acouunt and passward for devide.

## Build and Flash
* [ESP-IDF Getting Started Guide on ESP32](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)

## DEBUG FINISH
* BLUFI
* MQTT
* BL0937交流电能芯片
* NVS存储
* LED
* KEY

版本v1.0
1.10A设备版本，16A设备版本与此版本兼容，通过menuconfig配置设备类型
2.【交付】

版本v1.1
1.根据硬件添加按键、指示灯的代码
2.重新【交付】

版本v1.2
1.修改设备MQTT id重复导致MQTT连接失败问题，将BE码作为MQTT id
2.改小断点续传存储:0x78(130)-> 0x3c(60),防止溢出
3.在产测软件未研发成功前，供临时测试