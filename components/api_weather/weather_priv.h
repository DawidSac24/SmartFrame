#pragma once

#include "esp_err.h"
#include "esp_log.h"

struct localisation
{
    float latitude;
    float longitude;
};

esp_err_t api_weather_fetch(struct localisation *localisation);
esp_err_t api_weather_parse(const char *json_str);

void weather_cmd_register(void);