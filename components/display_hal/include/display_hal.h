#pragma once
#include <stdint.h>
#include "esp_err.h"

// Standard test colors
#define COLOR_BLACK 0, 0, 0
#define COLOR_RED 255, 0, 0
#define COLOR_GREEN 0, 255, 0
#define COLOR_BLUE 0, 0, 255
#define COLOR_WHITE 255, 255, 255

#ifdef __cplusplus
extern "C"
{
#endif

    enum class Hub75Rotation : uint16_t
    {
        ROTATE_0 = 0,     // No rotation (default)
        ROTATE_90 = 90,   // 90° clockwise
        ROTATE_180 = 180, // 180°
        ROTATE_270 = 270  // 270° clockwise (90° counter-clockwise)
    };

    esp_err_t display_hal_init(void);

    void display_hal_draw_pixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *buffer);
    void display_hal_draw_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
    void display_hal_fill(int16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b);

    void display_hal_clear(void);
    void display_hal_update(void);

#ifdef __cplusplus
}
#endif