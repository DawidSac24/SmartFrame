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

    esp_err_t hub75_hal_init(void);

    void hub75_hal_set_brightness(uint8_t brightness);
    uint8_t hub75_hal_get_brightness(void);

    void hub75_hal_set_rotation(uint8_t rotation);
    uint8_t hub75_hal_get_rotation(void);

    void hub75_hal_draw_pixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *buffer);
    void hub75_hal_draw_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
    void hub75_hal_fill(int16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b);

    void hub75_hal_clear(void);
    void hub75_hal_update(void);

#ifdef __cplusplus
}
#endif
