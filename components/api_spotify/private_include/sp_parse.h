#pragma once

#include "sp_types.h"

#include "esp_err.h"

esp_err_t sp_parse_token_response(const char *json_string,
                                  struct sp_token_response *out_info);
esp_err_t sp_parse_track(const char *json_string, struct sp_track_info *out_info);
