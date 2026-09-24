#include "gfx.h"

#include "storage.h"
#include "hub75_hal.h"

#include "bdf5x8.h"
#include "bdf8x16.h"

#include "jpeg_decoder.h"
#include "esp_log.h"
#include <stdlib.h>

static const char *TAG = "gfx";

esp_err_t gfx_init(void)
{
    return hub75_hal_init();
}

void gfx_update(void)
{
    hub75_hal_update();
}

void gfx_clear(void)
{
    hub75_hal_clear();
}

void gfx_set_brightness(uint8_t brightness)
{
    hub75_hal_set_brightness(brightness);
}
uint8_t gfx_get_brightness(void)
{
    return hub75_hal_get_brightness();
}
void gfx_set_rotation(uint8_t rotation)
{
    hub75_hal_set_rotation(rotation);
}
uint8_t gfx_get_rotation(void)
{
    return hub75_hal_get_rotation();
}

void gfx_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    hub75_hal_draw_pixel(x, y, r, g, b);
}

void gfx_draw_pixels(uint16_t x, uint16_t y, uint16_t with, uint16_t height, const uint8_t *buffer)
{
    hub75_hal_draw_pixels(x, y, with, height, buffer);
}

void gfx_draw_line(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1)
    {
        gfx_draw_pixel(x0, y0, r, g, b);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void gfx_draw_text(int x, int y, uint8_t r, uint8_t g, uint8_t b, const char *text, enum gfx_font font_type)
{
    int cursor_x = x;
    int len = strlen(text);

    for (int i = 0; i < len; i++)
    {
        char c = text[i];

        if (c >= 32 && c <= 126)
        {
            int font_idx = c - 32;

            if (font_type == FONT_5x8)
            {
                // Draw 5x8 Font (Vertical scanning, 1 byte per column)
                for (int col = 0; col < 6; col++)
                {
                    uint8_t line = font5x8[(font_idx * 6) + col];
                    for (int row = 0; row < 8; row++)
                    {
                        if (line & (1 << row))
                        {
                            gfx_draw_pixel(cursor_x + col, y + row, r, g, b);
                        }
                    }
                }
                cursor_x += 6;
            }
            else if (font_type == FONT_8x16)
            {
                // 16 bytes per character (8 bytes top half, 8 bytes bottom half)
                for (int col = 0; col < 8; col++)
                {

                    // 1. Draw the top 8 pixels
                    uint8_t top_byte = font8x16[(font_idx * 16) + col];
                    for (int row = 0; row < 8; row++)
                    {
                        if (top_byte & (1 << row))
                        {
                            gfx_draw_pixel(cursor_x + col, y + row, r, g, b);
                        }
                    }

                    // 2. Draw the bottom 8 pixels (Offset index by 8, offset Y by 8)
                    uint8_t bot_byte = font8x16[(font_idx * 16) + 8 + col];
                    for (int row = 0; row < 8; row++)
                    {
                        if (bot_byte & (1 << row))
                        {
                            gfx_draw_pixel(cursor_x + col, y + 8 + row, r, g, b);
                        }
                    }
                }
                // Advance exactly 8 pixels (the empty columns provide natural spacing)
                cursor_x += 8;
            }
        }
    }
}
void gfx_draw_screen(const uint8_t *buffer)
{
    hub75_hal_draw_pixels(0, 0, 64, 64, buffer);
}

esp_err_t gfx_decode_jpeg(const char *filepath, uint8_t *out_rgb_buffer, size_t buffer_size)
{
    uint8_t *jpg_data = NULL;
    size_t jpg_size = 0;

    // 1. Read binary JPEG into heap
    if (storage_read_buff(filepath, &jpg_data, &jpg_size) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read %s from storage", filepath);
        return ESP_FAIL;
    }

    // 2. Configure decoder and map the buffers INSIDE the struct
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = jpg_data,
        .indata_size = jpg_size,
        .outbuf = out_rgb_buffer,
        .outbuf_size = buffer_size,
        .out_format = JPEG_IMAGE_FORMAT_RGB888,
        .out_scale = JPEG_IMAGE_SCALE_0,
        .flags = {.swap_color_bytes = 0}};

    // 3. Decode!
    esp_jpeg_image_output_t out_img;
    esp_err_t err = esp_jpeg_decode(&jpeg_cfg, &out_img);

    // 4. Instantly free the compressed JPEG memory
    free(jpg_data);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "JPEG Decode failed with error: %d", err);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "JPEG Decoded successfully! Size: %dx%d", out_img.width, out_img.height);
    return ESP_OK;
}