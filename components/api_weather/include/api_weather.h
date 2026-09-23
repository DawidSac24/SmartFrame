#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct weather_dto
{
    int id;
    float temperature;
    float humidity;
    float pressure;
    char icon[8];
    char main[16];
    char description[64];
    time_t last_fetched;
    bool is_valid;
};

esp_err_t weather_init(float latitude, float longitude, uint32_t stack_size, UBaseType_t priority);

esp_err_t weather_get_info(struct weather_dto *out_dto);