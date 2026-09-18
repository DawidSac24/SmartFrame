#pragma once

#include "api_spotify.h"

#include "esp_err.h"

void sp_state_init(void);

esp_err_t sp_state_set_track_info(const struct spotify_track_dto *track_info);