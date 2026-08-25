#pragma once

#include "esp_err.h"

enum nvs_reg_key
{
    REG_SPOTIFY_REFRESH_TOKEN,
    REG_UI_IMPORT_IMG,
    REG_MAX
};

esp_err_t nvs_hal_init();

esp_err_t nvs_hal_read_str(enum nvs_reg_key key, char *out_buff, size_t max_len);
esp_err_t nvs_hal_write_str(enum nvs_reg_key key, const char *buff);