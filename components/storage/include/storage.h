#pragma once

#include "esp_err.h"

esp_err_t storage_init();

esp_err_t storage_get_str(const char *key, char *out_buff, size_t max_len);
esp_err_t storage_set_str(const char *key, const char *val);