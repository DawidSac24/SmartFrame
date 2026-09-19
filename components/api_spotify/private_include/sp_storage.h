#pragma once

#include "esp_err.h"

esp_err_t sp_storage_get_refresh_token(char *out_token, size_t max_len);
esp_err_t sp_storage_set_refresh_token(char *token);
