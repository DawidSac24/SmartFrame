#include "hub75_hal.h"
#include "hub75.h"
#include "esp_log.h"

static const char *TAG = "hub75_hal";

// The brand new ESPHome native driver pointer
Hub75Driver *dma_display = nullptr;

// State tracking
static uint8_t s_brightness = 128;
static uint8_t s_rotation = 0; // 0=0°, 1=90°, 2=180°, 3=270°

esp_err_t hub75_hal_init(void)
{
    Hub75Config config{};
    config.panel_width = 64;
    config.panel_height = 64;
    config.double_buffer = true;

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

    dma_display = new Hub75Driver(config);

    if (dma_display->begin())
    {
        dma_display->set_brightness(s_brightness);
        dma_display->clear();
        dma_display->flip_buffer();
        dma_display->set_rotation(Hub75Rotation::ROTATE_90);

        ESP_LOGI(TAG, "HUB75 DMA Matrix initialized successfully!");
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to initialize HUB75 Driver.");
        return ESP_FAIL;
    }
}

// --- STATE GETTERS & SETTERS ---

void hub75_hal_set_brightness(uint8_t brightness)
{
    s_brightness = brightness;
    if (dma_display != nullptr)
    {
        dma_display->set_brightness(s_brightness);
    }
}

uint8_t hub75_hal_get_brightness(void)
{
    return s_brightness;
}

void hub75_hal_set_rotation(uint8_t rotation)
{
    s_rotation = rotation % 4; // Safely wrap 0-3
}

uint8_t hub75_hal_get_rotation(void)
{
    return s_rotation;
}

// --- DRAWING ---

void hub75_hal_draw_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b)
{
    if (dma_display == nullptr)
        return;

    // Apply rotation mathematically for a 64x64 matrix
    uint16_t tx = x;
    uint16_t ty = y;

    switch (s_rotation)
    {
    case 1: // 90 degrees
        tx = 63 - y;
        ty = x;
        break;
    case 2: // 180 degrees
        tx = 63 - x;
        ty = 63 - y;
        break;
    case 3: // 270 degrees
        tx = y;
        ty = 63 - x;
        break;
    default: // 0 degrees
        break;
    }

    // Bounds check to prevent memory corruption if coordinates go negative/overflow
    if (tx < 64 && ty < 64)
    {
        dma_display->set_pixel(tx, ty, r, g, b);
    }
}

void hub75_hal_draw_pixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *buffer)
{
    if (dma_display == nullptr || buffer == nullptr)
        return;

    // Fast path: Hardware block push if no rotation
    if (s_rotation == 0)
    {
        dma_display->draw_pixels(x, y, w, h, buffer,
                                 Hub75PixelFormat::RGB888,
                                 Hub75ColorOrder::RGB,
                                 false);
    }
    // Safe path: Plot pixel-by-pixel so rotation matrix applies correctly to blocks
    else
    {
        int i = 0;
        for (uint16_t dy = 0; dy < h; dy++)
        {
            for (uint16_t dx = 0; dx < w; dx++)
            {
                uint8_t r = buffer[i++];
                uint8_t g = buffer[i++];
                uint8_t b = buffer[i++];

                // Route through our math-aware pixel function
                hub75_hal_draw_pixel(x + dx, y + dy, r, g, b);
            }
        }
    }
}

void hub75_hal_fill(int16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b)
{
    for (int16_t dy = 0; dy < h; dy++)
    {
        for (int16_t dx = 0; dx < w; dx++)
        {
            hub75_hal_draw_pixel(x + dx, y + dy, r, g, b);
        }
    }
}

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
        dma_display->flip_buffer();
    }
}