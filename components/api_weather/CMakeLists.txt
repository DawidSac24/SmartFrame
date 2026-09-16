idf_component_register(SRCS "weather_task.c" "weather_client.c" "weather_parse.c" "weather_cmd.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES app_state wifi config esp_http_client mbedtls cjson console)