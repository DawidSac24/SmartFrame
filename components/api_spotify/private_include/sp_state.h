#pragma once

#include "api_spotify.h"

#include "esp_err.h"

void sp_state_init(void);

const char *sp_api_state_to_str(enum sp_api_state state);
const char *sp_cover_state_to_str(enum sp_cover_state_t state);