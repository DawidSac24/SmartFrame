#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum wea_icon_state
{
    WEA_ICON_NONE = 0,
    WEA_ICON_DOWNLOADING,
    WEA_ICON_NEW_FILE,
    WEA_ICON_DECODED,
    WEA_ICON_FAILED
};

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

    enum wea_icon_state icon_state;
};

esp_err_t weather_init(float latitude, float longitude, uint32_t stack_size, UBaseType_t priority);

esp_err_t weather_get_info(struct weather_dto *out_dto);
void weather_set_icon_state(enum wea_icon_state state);