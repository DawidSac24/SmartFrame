#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

struct localisation
{
    float latitude;
    float longitude;
};
// client
esp_err_t api_weather_fetch(struct localisation *localisation);
// parser
esp_err_t api_weather_parse(const char *json_str);

// cmd
enum weather_cmd
{
    WEATHER_CMD_FETCH,
    WEATHER_CMD_PRINT,
    WEATHER_CMD_MAX
};

extern QueueHandle_t weather_cmd_queue;

void weather_cmd_register(void);

void weather_cmd_send(enum weather_cmd cmd);
void weather_cmd_dispatch(enum weather_cmd cmd);