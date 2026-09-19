idf_component_register(SRCS "nvs_hal.c" "storage.c" "littleFS_hal.c"
                       INCLUDE_DIRS "include"
                        PRIV_REQUIRES nvs_flash littlefs)