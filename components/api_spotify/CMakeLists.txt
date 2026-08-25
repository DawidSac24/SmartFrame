idf_component_register(SRCS "spotify_parse.c" "spotify_fetch.c" "spotify_task.c" "spotify_cmd.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES app_state wifi config storage esp_http_client mbedtls cjson console)