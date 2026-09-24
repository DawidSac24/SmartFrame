#pragma once
#include <stdint.h>
#include "screen.h"

// Initializes the clock singleton and returns its pointer for the scheduler
struct screen *screen_custom_img_init(uint32_t duration_ms);