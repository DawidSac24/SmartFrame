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
esp_err_t wea_fetch(struct localisation *localisation);
// parser
esp_err_t wea_parse(const char *json_str);

// cmd
enum wea_cmd
{
    WEA_CMD_FETCH,
    WEA_CMD_PRINT,
    WEA_CMD_UNKNOWN
};

extern QueueHandle_t weather_cmd_queue;

void wea_cmd_register(void);

void wea_cmd_send(enum wea_cmd cmd);
esp_err_t wea_cmd_dispatch(enum wea_cmd cmd);

enum wea_cmd wea_str_to_cmd(const char *cmd);