#pragma once

#include "esp_err.h"

esp_err_t api_spotify_get_refresh_token();
esp_err_t api_spotify_fetch_access_token();

esp_err_t api_spotify_parse_tokens();