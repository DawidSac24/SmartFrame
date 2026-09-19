idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES hub75_hal wifi web_server app_state api_weather api_spotify cli config storage esp_event )