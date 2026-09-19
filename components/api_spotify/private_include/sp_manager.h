#pragma once

#include "esp_err.h"

void sp_manager_init(void);

esp_err_t sp_manager_fetch_and_save_track(void);

esp_err_t sp_manager_ensure_authentificated(void);

esp_err_t sp_manager_print_track(void);