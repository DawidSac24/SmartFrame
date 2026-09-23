idf_component_register(SRCS "wea_state.c" "wea_task.c" "wea_parse.c" "wea_cmd.c" "wea_client.c"
                       INCLUDE_DIRS "include"
                       PRIV_INCLUDE_DIRS "."
                       PRIV_REQUIRES wifi config esp_http_client mbedtls cjson console)