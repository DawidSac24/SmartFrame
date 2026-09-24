#pragma once

#include "esp_err.h"
#include <stdint.h>

enum gfx_font
{
    FONT_5x8,
    FONT_8x16
};

esp_err_t gfx_init(void);

void gfx_update(void);
void gfx_clear(void);

void gfx_set_brightness(uint8_t brightness);
uint8_t gfx_get_brightness(void);

void gfx_set_rotation(uint8_t rotation);
uint8_t gfx_get_rotation(void);

void gfx_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void gfx_draw_pixels(uint16_t x, uint16_t y, uint16_t with, uint16_t height, const uint8_t *buffer);
void gfx_draw_line(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b);

void gfx_draw_text(int x, int y, uint8_t r, uint8_t g, uint8_t b, const char *text, enum gfx_font font_type);

void gfx_draw_screen(const uint8_t *buffer);

// --- 2. Image Decoding ---
esp_err_t gfx_decode_jpeg(const char *filepath, uint8_t *out_rgb_buffer, size_t buffer_size);
