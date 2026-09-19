#pragma once

#include "sp_types.h"

#include "esp_err.h"

struct http_response
{
    char *data;
    int status_code;
};

esp_err_t sp_client_fetch_track(const char *access_token, char **res);

esp_err_t sp_client_exchange_code(const char *auth_code, const char *auth_header, char **res);
esp_err_t sp_client_refresh_token(const char *refresh_token, const char *auth_header, char **res);