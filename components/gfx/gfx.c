#include "gfx.h"
#include "storage.h"
#include "jpeg_decoder.h"
#include "esp_log.h"
#include <stdlib.h>

#include "hub75_hal.h"

static const char *TAG = "gfx";

void gfx_draw_screen(const uint8_t *buffer)
{
    hub75_hal_draw_pixels(0, 0, 64, 64, buffer);
    hub75_hal_update();
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