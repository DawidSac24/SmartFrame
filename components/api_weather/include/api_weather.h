#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t api_weather_start_task(float latitude, float longitude);
void api_weather_delete_task(TaskHandle_t task_handle);