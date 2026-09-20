#pragma once

#pragma once
// #include "weather_types.h"
#include "esp_err.h"

// --- 1. Primitive Drawing ---
void gfx_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void gfx_print_text(int x, int y, const char *text); // Add font params later

void gfx_draw_screen(const uint8_t *buffer);

// --- 2. Image Decoding ---
// Reads from LittleFS and writes to a pre-allocated RGB buffer
esp_err_t gfx_decode_jpeg(const char *filepath, uint8_t *out_rgb_buffer, size_t buffer_size);

// --- 3. Screen Layouts ---
// Takes the data and arranges the primitives on the screen
// void gfx_draw_spotify_screen(const struct spotify_track_dto *track, const uint8_t *decoded_image_buffer);
// void gfx_draw_weather_screen(const struct weather_dto* weather);