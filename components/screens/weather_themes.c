#include "theme.h"
#include "gfx.h"
#include "api_weather.h"
#include "lodepng.h" // Include the PNG decoder
#include <stdio.h>
#include <string.h>
#include <math.h>

static uint8_t *s_weather_icon_rgba = NULL;
static bool s_has_decoded_icon = false;

static void trigger_weather_decode(void)
{
    unsigned char *image = NULL;
    unsigned width, height;

    unsigned error = lodepng_decode32_file(&image, &width, &height, "/fs/weather.png");

    if (!error && width == 50 && height == 50)
    {
        if (s_weather_icon_rgba != NULL)
        {
            free(s_weather_icon_rgba);
        }

        s_weather_icon_rgba = image; // Keep the pointer!
        s_has_decoded_icon = true;
        weather_set_icon_state(WEA_ICON_DECODED);
        printf("Weather PNG decoded successfully!\n");
    }
    else
    {
        s_has_decoded_icon = false;
        weather_set_icon_state(WEA_ICON_FAILED);
        if (image)
            free(image);
    }
}

static void draw_wth_default(struct theme *base, struct screen *parent, float dt_ms)
{
    struct weather_dto weather;
    if (weather_get_info(&weather) != ESP_OK || !weather.is_valid)
    {
        gfx_draw_text(16, 28, 200, 200, 200, "Loading...", FONT_5x8);
        return;
    }

    // Check if we need to decode a new downloaded PNG
    if (weather.icon_state == WEA_ICON_NEW_FILE)
    {
        trigger_weather_decode();
    }

    // --- DRAW IMAGE IF AVAILABLE ---
    if (s_has_decoded_icon)
    {
        int start_x = 7;
        int start_y = 0;
        int i = 0;

        for (int y = 0; y < 50; y++)
        {
            for (int x = 0; x < 50; x++)
            {
                uint8_t r = s_weather_icon_rgba[i++];
                uint8_t g = s_weather_icon_rgba[i++];
                uint8_t b = s_weather_icon_rgba[i++];
                uint8_t a = s_weather_icon_rgba[i++]; // Alpha channel!

                // If pixel is transparent (alpha < 128), skip drawing it!
                if (a < 128)
                    continue;

                gfx_draw_pixel(start_x + x, start_y + y, r, g, b);
            }
        }
    }
    // --- FALLBACK TO PROCEDURAL ANIMATIONS ---
    else
    {
        static float anim_timer = 0.0f;
        anim_timer += dt_ms;
        int cx = 32, cy = 20;
        bool is_night = (weather.icon[2] == 'n');

        if (weather.id == 800)
        {
            if (is_night)
            {
                for (int y = -4; y <= 4; y++)
                {
                    for (int x = -4; x <= 4; x++)
                    {
                        if (x * x + y * y <= 16 && (x - 2) * (x - 2) + y * y > 9)
                            gfx_draw_pixel(cx + x, cy + y, 220, 220, 240);
                    }
                }
                gfx_draw_pixel(cx - 6, cy - 4, 255, 255, 255);
                gfx_draw_pixel(cx + 6, cy + 5, 200, 200, 200);
            }
            else
            {
                int radius = 5 + (int)(sin(anim_timer / 300.0f) * 1.5f);
                for (int y = -radius; y <= radius; y++)
                {
                    for (int x = -radius; x <= radius; x++)
                    {
                        if (x * x + y * y <= radius * radius)
                            gfx_draw_pixel(cx + x, cy + y, 255, 220, 50);
                    }
                }
            }
        }
        else if (weather.id >= 801 && weather.id <= 804)
        {
            if (!is_night && weather.id == 801)
            {
                int s_rad = 4;
                for (int y = -s_rad; y <= s_rad; y++)
                {
                    for (int x = -s_rad; x <= s_rad; x++)
                    {
                        if (x * x + y * y <= s_rad * s_rad)
                            gfx_draw_pixel(cx + x - 4, cy + y - 4, 255, 220, 50);
                    }
                }
            }
            for (int cy_off = -2; cy_off <= 2; cy_off++)
            {
                for (int cx_off = -8; cx_off <= 8; cx_off++)
                {
                    gfx_draw_pixel(cx + cx_off, cy + cy_off + 2, 180, 190, 210);
                }
            }
        }
        else if ((weather.id / 100) == 5 || (weather.id / 100) == 3)
        {
            if (weather.id == 500 && !is_night)
            {
                for (int y = -3; y <= 3; y++)
                {
                    for (int x = -3; x <= 3; x++)
                    {
                        if (x * x + y * y <= 9)
                            gfx_draw_pixel(cx + x + 6, cy + y - 6, 255, 220, 50);
                    }
                }
            }
            static float drop_offset = 0.0f;
            drop_offset += dt_ms * 0.04f;
            if (drop_offset > 8.0f)
                drop_offset = 0.0f;
            for (int i = -2; i <= 2; i++)
            {
                int rx = cx + (i * 4);
                int ry = (int)(cy + 2 + fmodf(drop_offset + (i * 2), 14.0f));
                gfx_draw_pixel(rx, ry, 100, 150, 255);
                gfx_draw_pixel(rx - 1, ry + 1, 100, 150, 255);
            }
        }
        else if ((weather.id / 100) == 2)
        {
            static float anim_timer = 0.0f;
            anim_timer += dt_ms;
            int flash = ((int)(anim_timer / 200) % 2);
            if (flash)
            {
                gfx_draw_pixel(cx, cy + 4, 255, 255, 0);
                gfx_draw_pixel(cx - 1, cy + 5, 255, 255, 0);
            }
            for (int x = -6; x <= 6; x++)
                gfx_draw_pixel(cx + x, cy, 100, 100, 120);
        }
        else if ((weather.id / 100) == 6)
        {
            static float snow_offset = 0.0f;
            static float anim_timer = 0.0f;
            anim_timer += dt_ms;
            snow_offset += dt_ms * 0.015f;
            if (snow_offset > 12.0f)
                snow_offset = 0.0f;
            for (int i = -2; i <= 2; i++)
            {
                int sx = cx + (i * 5) + (int)(sin(anim_timer / 500.0f + i) * 2);
                int sy = (int)(cy - 4 + fmodf(snow_offset + (i * 3), 16.0f));
                gfx_draw_pixel(sx, sy, 240, 240, 255);
            }
        }
        else
        {
            gfx_draw_text(cx - 9, cy - 4, 150, 150, 160, "WTH", FONT_5x8);
        }
    }

    // --- DRAW TEMPERATURE (Always displays at the bottom) ---
    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "%.1f C", weather.temperature);
    int text_x = (64 - (strlen(temp_str) * 6)) / 2;
    gfx_draw_text(text_x, 48, 255, 255, 255, temp_str, FONT_5x8);
}

static void draw_wth_compact(struct theme *base, struct screen *parent, float dt_ms)
{
    struct weather_dto weather;
    if (weather_get_info(&weather) != ESP_OK || !weather.is_valid)
        return;

    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "%.1fC", weather.temperature);
    gfx_draw_text(2, 2, 255, 255, 255, temp_str, FONT_5x8);
    gfx_draw_text(2, 54, 150, 150, 150, weather.main, FONT_5x8);
}

static const struct theme_vtable vt_wth_def = {.draw = draw_wth_default};
static const struct theme_vtable vt_wth_cmp = {.draw = draw_wth_compact};

static struct theme th_wth_def = {.name = "default", .vtable = &vt_wth_def};
static struct theme th_wth_cmp = {.name = "compact", .vtable = &vt_wth_cmp};

static const struct theme *s_all_weather_themes[] = {&th_wth_def, &th_wth_cmp};

const struct theme *weather_theme_get(const char *name)
{
    if (!name)
        return &th_wth_def;
    for (size_t i = 0; i < sizeof(s_all_weather_themes) / sizeof(s_all_weather_themes[0]); i++)
    {
        if (strcmp(s_all_weather_themes[i]->name, name) == 0)
            return s_all_weather_themes[i];
    }
    return &th_wth_def;
}