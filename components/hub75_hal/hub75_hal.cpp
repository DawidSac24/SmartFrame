#include "hub75_hal.h"
#include "hub75.h"
#include "esp_log.h"

static const char *TAG = "hub75_hal";

// The brand new ESPHome native driver pointer
Hub75Driver *dma_display = nullptr;

esp_err_t hub75_hal_init(void)
{
    // 1. Create the configuration object
    Hub75Config config{};
    config.panel_width = 64;
    config.panel_height = 64;

    // CRITICAL: Prevent screen tearing
    config.double_buffer = true;

    // 2. Matrix Portal S3 Hardwired Pins
    config.pins.r1 = 42;
    config.pins.g1 = 41;
    config.pins.b1 = 40;
    config.pins.r2 = 38;
    config.pins.g2 = 39;
    config.pins.b2 = 37;
    config.pins.a = 45;
    config.pins.b = 36;
    config.pins.c = 48;
    config.pins.d = 35;
    config.pins.e = 21;
    config.pins.lat = 47;
    config.pins.oe = 14;
    config.pins.clk = 2;

    // 3. Allocate the object in RAM
    dma_display = new Hub75Driver(config);

    // 4. Boot the DMA Engine
    if (dma_display->begin())
    {
        dma_display->set_brightness(70); // 0-255
        dma_display->clear();
        dma_display->flip_buffer(); // Push the clear to the screen

        ESP_LOGI(TAG, "HUB75 DMA Matrix initialized successfully!");

        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to initialize HUB75 Driver.");
        return ESP_FAIL;
    }
}

void hub75_hal_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if (dma_display != nullptr)
    {
        dma_display->set_pixel(x, y, r, g, b);
    }
}
void hub75_hal_draw_pixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *buffer)
{
    if (dma_display != nullptr && buffer != NULL)
    {
        dma_display->draw_pixels(x, y, w, h, buffer,
                                 Hub75PixelFormat::RGB888,
                                 Hub75ColorOrder::RGB,
                                 false); // Not big-endian
    }
}
void hub75_hal_fill(int16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b);

void hub75_hal_clear(void)
{
    if (dma_display != nullptr)
    {
        dma_display->clear();
    }
}

void hub75_hal_update(void)
{
    if (dma_display != nullptr)
    {
        // Swaps the invisible RAM buffer to the physical LEDs
        dma_display->flip_buffer();
    }
}