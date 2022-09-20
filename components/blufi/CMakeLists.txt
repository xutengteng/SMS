idf_component_register(SRCS "app_blufi.c"
                            "blufi_security.c"
                            "blufi_init.c"

                    INCLUDE_DIRS "."

                    PRIV_REQUIRES nvs_flash bt blufi esp_wifi freertos esp_system
                    )
