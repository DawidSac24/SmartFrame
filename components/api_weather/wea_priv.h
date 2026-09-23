#pragma once

#include "api_weather.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

esp_err_t wea_state_init(void);
esp_err_t wea_state_set(const struct weather_dto *new_data);

// Client & Parsing
esp_err_t wea_fetch(float lat, float lon);
esp_err_t wea_parse(const char *json_str, struct weather_dto *out_dto);

enum wea_cmd
{
    WEA_CMD_FETCH,
    WEA_CMD_PRINT,
    WEA_CMD_TEST,
    WEA_CMD_UNKNOWN
};

extern QueueHandle_t wea_cmd_queue;
void wea_cmd_register(void);
esp_err_t wea_cmd_dispatch(enum wea_cmd cmd);
void weather_set_override(const struct weather_dto *forced_dto);