  idf_component_register(SRCS "spotify_auth.c" "spotify_client.c" "spotify_parse.c" "spotify_storage.c" "spotify_task.c" "spotify_cmd.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES app_state wifi config storage http_client mbedtls cjson console esp_timer)
