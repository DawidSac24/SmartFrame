#include "weather_screen.h"
#include "api_weather.h"
#include "gfx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static void wth_draw(struct screen *self, float dt_ms)
{
    struct weather_dto weather;

    if (weather_get_info(&weather) != ESP_OK || !weather.is_valid)
    {
        gfx_draw_text(16, 28, 200, 200, 200, "Loading...", FONT_5x8);
        return;
    }

    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "%.1f C", weather.temperature);
    int text_width = strlen(temp_str) * 6;
    int text_x = (64 - text_width) / 2;

    static float anim_timer = 0.0f;
    anim_timer += dt_ms;

    int cx = 32, cy = 20;
    bool is_night = (weather.icon[2] == 'n'); // Check if OpenWeatherMap icon ends in 'n'

    // --- 1. RENDER GRAPHIC BASED ON WEATHER ID & DAY/NIGHT ---
    if (weather.id == 800)
    {
        if (is_night)
        {
            // --- NIGHT: Glowing Crescent Moon & Stars ---
            for (int y = -4; y <= 4; y++)
            {
                for (int x = -4; x <= 4; x++)
                {
                    if (x * x + y * y <= 16 && (x - 2) * (x - 2) + y * y > 9)
                    {
                        gfx_draw_pixel(cx + x, cy + y, 220, 220, 240);
                    }
                }
            }
            gfx_draw_pixel(cx - 6, cy - 4, 255, 255, 255); // Star 1
            gfx_draw_pixel(cx + 6, cy + 5, 200, 200, 200); // Star 2
        }
        else
        {
            // --- SUN: Pulsing Clear Sun ---
            int radius = 5 + (int)(sin(anim_timer / 300.0f) * 1.5f);
            for (int y = -radius; y <= radius; y++)
            {
                for (int x = -radius; x <= radius; x++)
                {
                    if (x * x + y * y <= radius * radius)
                    {
                        gfx_draw_pixel(cx + x, cy + y, 255, 220, 50);
                    }
                }
            }
        }
    }
    else if (weather.id >= 801 && weather.id <= 804)
    {
        // --- CLOUDS / SUNNY WITH CLOUDS / NIGHT WITH CLOUDS ---
        if (!is_night && weather.id == 801)
        {
            // Sun peeking behind a cloud (Sun + Clouds)
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
        // Draw Cloud Body
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
        // --- RAIN (Can be Sun + Rain if light rain ID 500) ---
        if (weather.id == 500 && !is_night)
        {
            // Sun-shower (Rain with sun)
            for (int y = -3; y <= 3; y++)
            {
                for (int x = -3; x <= 3; x++)
                {
                    if (x * x + y * y <= 9)
                        gfx_draw_pixel(cx + x + 6, cy + y - 6, 255, 220, 50);
                }
            }
        }
        // Falling raindrops animation
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
        // --- THUNDERSTORM: Clouds with flashing bolts ---
        int flash = ((int)(anim_timer / 200) % 2); // Blinking effect
        if (flash)
        {
            gfx_draw_pixel(cx, cy + 4, 255, 255, 0);
            gfx_draw_pixel(cx - 1, cy + 5, 255, 255, 0);
            gfx_draw_pixel(cx, cy + 6, 255, 255, 0);
        }
        // Cloud silhouette
        for (int x = -6; x <= 6; x++)
            gfx_draw_pixel(cx + x, cy, 100, 100, 120);
    }
    else if ((weather.id / 100) == 6)
    {
        // --- SNOW: Soft falling flakes ---
        static float snow_offset = 0.0f;
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
    else if (weather.id == 741 || weather.id == 701)
    {
        // --- ATMOSPHERE (Fog/Mist): Horizontal hazy lines ---
        for (int i = -2; i <= 2; i += 2)
        {
            for (int x = -10; x <= 10; x += 2)
            {
                gfx_draw_pixel(cx + x, cy + i * 2, 140, 140, 150);
            }
        }
    }

    // Draw Temperature Label at bottom
    gfx_draw_text(text_x, 48, 255, 255, 255, temp_str, FONT_5x8);
}

// Boilerplate screen vtable setup
static uint32_t wth_trans_in(struct screen *self) { return 0; }
static uint32_t wth_trans_out(struct screen *self) { return 0; }

static const struct screen_vtable s_weather_vtable = {
    .draw = wth_draw,
    .transition_in = wth_trans_in,
    .transition_out = wth_trans_out,
};

static struct screen s_wth_instance;
struct screen *screen_weather_init(uint32_t duration_ms)
{
    s_wth_instance.vtable = &s_weather_vtable;
    s_wth_instance.name = "weather";
    s_wth_instance.display_duration_ms = duration_ms;
    s_wth_instance.active_theme = NULL;
    return &s_wth_instance;
}