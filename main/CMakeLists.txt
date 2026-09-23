idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES ui wifi web_server api_weather api_spotify cli config storage esp_event screens)