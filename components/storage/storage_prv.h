#pragma once

#include "esp_err.h"

// NVS HAL
esp_err_t nvs_hal_init();

esp_err_t nvs_hal_get_str(const char *key, char *out_buff, size_t max_len);
esp_err_t nvs_hal_set_str(const char *key, const char *val);

// LittleFS HAL
esp_err_t fs_hal_init();

esp_err_t fs_hal_read_buff(const char *filepath, uint8_t **out_data, size_t *out_size);
esp_err_t fs_hal_write_buff(const char *filepath, const uint8_t *data, size_t size);