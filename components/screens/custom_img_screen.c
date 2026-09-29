#include "custom_img_screen.h"
#include "theme.h"
#include "gfx.h"
#include "api_custom_img.h"
#include "lodepng.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"

// --- DECODING STATE & LOGIC ---
static uint8_t *s_custom_rgba = NULL; // Just a pointer now! No 16KB static array.
static bool s_has_decoded = false;

static void trigger_custom_decode(void)
{
    unsigned char *image = NULL;
    unsigned width, height;

    unsigned error = lodepng_decode32_file(&image, &width, &height, "/fs/custom.png");

    // Check if it's successful AND at least 64x64
    if (!error && width >= 64 && height >= 64)
    {
        if (s_custom_rgba != NULL)
        {
            free(s_custom_rgba);
        }

        if (width == 64 && height == 64)
        {
            // Fast path: Webpage already sent a perfect 64x64
            s_custom_rgba = image;
        }
        else
        {
            // Direct API path: Image is large, let's crop the exact center 64x64
            s_custom_rgba = malloc(64 * 64 * 4);
            if (!s_custom_rgba)
            {
                free(image);
                return; // Fail gracefully if out of memory
            }

            int start_x = (width - 64) / 2;
            int start_y = (height - 64) / 2;

            for (int y = 0; y < 64; y++)
            {
                for (int x = 0; x < 64; x++)
                {
                    int src_idx = ((start_y + y) * width + (start_x + x)) * 4;
                    int dst_idx = (y * 64 + x) * 4;

                    s_custom_rgba[dst_idx] = image[src_idx];         // R
                    s_custom_rgba[dst_idx + 1] = image[src_idx + 1]; // G
                    s_custom_rgba[dst_idx + 2] = image[src_idx + 2]; // B
                    s_custom_rgba[dst_idx + 3] = image[src_idx + 3]; // A
                }
            }
            free(image); // We kept the center, delete the massive original!
        }

        s_has_decoded = true;
        custom_img_set_state(CUSTOM_IMG_DECODED);
        ESP_LOGI("cimg", "Custom PNG decoded & cropped successfully!");
    }
    else
    {
        s_has_decoded = false;
        custom_img_set_state(CUSTOM_IMG_FAILED);
        if (error)
        {
            ESP_LOGE("cimg", "Custom PNG decode failed (Error %u)", error);
        }
        else if (width < 64 || height < 64)
        {
            ESP_LOGE("cimg", "Custom PNG too small! Must be at least 64x64.");
        }

        if (image)
            free(image);
    }
}

// --- THEME LAYER ---
static void draw_custom_default(struct theme *base, struct screen *parent, float dt_ms)
{
    // Check if the API just downloaded a new image
    if (custom_img_get_state() == CUSTOM_IMG_NEW_FILE)
    {
        trigger_custom_decode();
    }

    if (s_has_decoded)
    {
        int i = 0;
        for (int y = 0; y < 64; y++)
        {
            for (int x = 0; x < 64; x++)
            {
                uint8_t r = s_custom_rgba[i++];
                uint8_t g = s_custom_rgba[i++];
                uint8_t b = s_custom_rgba[i++];
                uint8_t a = s_custom_rgba[i++];

                if (a > 128)
                {
                    gfx_draw_pixel(x, y, r, g, b);
                }
            }
        }
    }
    else
    {
        gfx_draw_text(10, 28, 150, 150, 150, "No Image", FONT_5x8);
    }
}

static const struct theme_vtable vt_custom_theme = {.draw = draw_custom_default};
static struct theme th_custom_def = {.name = "default", .vtable = &vt_custom_theme};

static const struct theme *get_custom_theme(const char *name)
{
    return &th_custom_def; // Always return default since it's the only one
}

// --- SCREEN LAYER ---
static void cimg_prepare(struct screen *self) {}
static uint32_t cimg_trans_in(struct screen *self) { return 0; }
static uint32_t cimg_trans_out(struct screen *self) { return 0; }
static void cimg_destroy(struct screen *self) {}

static void cimg_draw(struct screen *self, float dt_ms)
{
    if (self->active_theme && self->active_theme->vtable->draw)
    {
        self->active_theme->vtable->draw((struct theme *)self->active_theme, self, dt_ms);
    }
}

static const struct screen_vtable s_custom_vtable = {
    .prepare = cimg_prepare,
    .draw = cimg_draw,
    .transition_in = cimg_trans_in,
    .transition_out = cimg_trans_out,
    .destroy = cimg_destroy,
    .get_theme = get_custom_theme};

static struct screen s_cimg_instance;

// --- INITIALIZATION ---
struct screen *screen_custom_img_init(uint32_t duration_ms)
{
    s_cimg_instance.vtable = &s_custom_vtable;
    s_cimg_instance.name = "custom_img";
    s_cimg_instance.display_duration_ms = duration_ms;
    s_cimg_instance.active_theme = &th_custom_def;

    // Try to load the image from LittleFS on boot
    if (custom_img_get_state() == CUSTOM_IMG_NONE)
    {
        trigger_custom_decode();
    }

    return &s_cimg_instance;
}