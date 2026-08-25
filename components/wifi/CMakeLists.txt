idf_component_register(SRCS "wifi.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES config esp_wifi esp_netif esp_event nvs_flash mdns)