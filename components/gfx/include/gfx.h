#pragma once

#include "esp_err.h"

esp_err_t gfx_init(void);

void gfx_update(void);
void gfx_clear(void);

void gfx_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void gfx_print_text(int x, int y, const char *text); // Add font params later
void gfx_draw_pixels(uint16_t x, uint16_t y, uint16_t with, uint16_t height, const uint8_t *buffer);

void gfx_draw_screen(const uint8_t *buffer);

// --- 2. Image Decoding ---
esp_err_t gfx_decode_jpeg(const char *filepath, uint8_t *out_rgb_buffer, size_t buffer_size);
