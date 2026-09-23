#pragma once
#include "esp_err.h"

void ui_cmd_register(void);

struct screen;
typedef struct screen *(*screen_factory_cb_t)(const char *name, uint32_t duration_ms);

void ui_cmd_set_factory(screen_factory_cb_t cb);