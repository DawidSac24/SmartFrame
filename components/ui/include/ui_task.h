#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct screen;

esp_err_t ui_task_init(uint32_t stack_size, UBaseType_t priority);

void ui_task_set_spotify_screen(struct screen *scr);

void ui_task_set_spotify_exclusive(bool exclusive);