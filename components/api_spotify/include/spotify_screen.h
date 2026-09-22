#pragma once

#include "screen.h"
#include <stdint.h>

enum spotify_theme
{
    SP_THEME_DEFAULT,
    SP_THEME_DISK,
    SP_THEME_SPINNING_DISK
};

struct screen *screen_spotify_create(uint32_t duration_ms);