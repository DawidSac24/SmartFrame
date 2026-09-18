#pragma once

#include "sp_types.h"

#include "esp_err.h"

esp_err_t sp_client_fetch_track(const char *token, struct sp_track_info *out);

esp_err_t sp_client_exchange_code(const char *auth_code, const char *auth_header,
                                  struct sp_token_response *out_response);
esp_err_t sp_client_refresh_token(const char *refresh_token, const char *auth_header,
                                  struct sp_token_response *out_response);
