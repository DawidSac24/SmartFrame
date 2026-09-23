  idf_component_register(SRCS "sp_state.c" "sp_manager.c" "sp_task.c" "sp_storage.c" "sp_parse.c" "sp_cmd.c" "sp_client.c" "sp_auth.c"
                       INCLUDE_DIRS "include"
                       PRIV_INCLUDE_DIRS "private_include"
                       PRIV_REQUIRES gfx wifi config storage http_client mbedtls cjson console esp_timer)
