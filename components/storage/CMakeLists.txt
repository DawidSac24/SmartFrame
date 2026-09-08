idf_component_register(SRCS "nvs_hal.c" "storage.c"
                       INCLUDE_DIRS "include"
                        PRIV_REQUIRES nvs_flash)