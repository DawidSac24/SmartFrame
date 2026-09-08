#pragma once

#include "esp_err.h"

// NVS HAL
esp_err_t nvs_hal_init();

esp_err_t nvs_hal_get_str(const char *key, char *out_buff, size_t max_len);
esp_err_t nvs_hal_set_str(const char *key, const char *val);

// LittleFS HAL