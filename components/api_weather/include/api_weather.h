#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t weather_init(float latitude, float longitude);