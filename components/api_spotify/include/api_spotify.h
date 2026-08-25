#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t api_spotify_start_task(void);
void api_spotify_delete_task(TaskHandle_t task_handle);

esp_err_t api_spotify_send_auth_code(const char *auth_code);