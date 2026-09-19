#pragma once

#include "api_spotify.h"
#include "sp_types.h"

#include "esp_err.h"

esp_err_t sp_parse_track(const char *json, struct spotify_track_dto *out_info);

esp_err_t sp_parse_token_response(const char *json,
                                  struct sp_token_response *out);
