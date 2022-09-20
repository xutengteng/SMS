idf_component_register(SRCS "main.c"

                            "../APP/src/GPIO.c" 
                            "../APP/src/Character.c" 
                            "../APP/src/MQTT.c" 
                            "../APP/src/MQTT_Subscribed.c" 
                            "../APP/src/MQTT_Publish.c" 
                            "../APP/src/NVS.c" 
                            "../APP/src/D_open.c" 
                            "../APP/src/Pulse_CNT.c" 
                            "../APP/src/Timer.c" 
                            
                       INCLUDE_DIRS 
                                   "../APP/inc" 
                       REQUIRES 
                                   blufi
                       PRIV_REQUIRES
                                   mqtt nvs_flash 
                                   
)

