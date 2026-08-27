idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES display_hal wifi web_server app_state api_weather cli config storage esp_event )