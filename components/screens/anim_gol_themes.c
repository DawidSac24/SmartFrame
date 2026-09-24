#include "theme.h"
#include "gfx.h"
#include <string.h>
#include <stdlib.h>

#define GOL_TICK_RATE_MS 100

// Shared GOL State
static uint8_t s_grid[64][64];
static uint8_t s_next[64][64];
static float s_time_since_tick = 0;

// Theme 1 State (Solid Custom Color)
static uint8_t s_color_r = 0;
static uint8_t s_color_g = 255;
static uint8_t s_color_b = 0;

// Theme 2 State (Random Per Tick)
static uint8_t s_rnd_r = 255, s_rnd_g = 255, s_rnd_b = 255;

// --- GOL LOGIC STUB ---
static void simulate_gol_step(void)
{
    // TODO: Write your neighbor counting and survival/birth logic here!
    // Read from s_grid, write results to s_next.

    // After calculating, copy s_next back to s_grid
    memcpy(s_grid, s_next, sizeof(s_grid));
}

// --- THEME 1: SOLID CUSTOM COLOR ---
static bool gol_solid_set_param(struct theme *self, const char *key, const char *value)
{
    if (strcmp(key, "color") == 0 && value != NULL)
    {
        // Skip the '#' if the web UI sends it
        if (value[0] == '#')
            value++;

        // Parse the hex string (e.g., "FF5733")
        if (strlen(value) >= 6)
        {
            long hex_val = strtol(value, NULL, 16);
            s_color_r = (hex_val >> 16) & 0xFF;
            s_color_g = (hex_val >> 8) & 0xFF;
            s_color_b = hex_val & 0xFF;
            return true; // Successfully handled
        }
    }
    return false; // Unknown key or bad value
}

static void draw_gol_solid(struct theme *base, struct screen *parent, float dt_ms)
{
    s_time_since_tick += dt_ms;
    if (s_time_since_tick >= GOL_TICK_RATE_MS)
    {
        simulate_gol_step();
        s_time_since_tick = 0;
    }

    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            if (s_grid[y][x])
            {
                gfx_draw_pixel(x, y, s_color_r, s_color_g, s_color_b);
            }
        }
    }
}

static const struct theme_vtable vt_solid = {
    .draw = draw_gol_solid,
    .set_param = gol_solid_set_param};

// --- THEME 2: RANDOM COLOR PER TICK ---
static void draw_gol_rainbow(struct theme *base, struct screen *parent, float dt_ms)
{
    s_time_since_tick += dt_ms;
    if (s_time_since_tick >= GOL_TICK_RATE_MS)
    {
        simulate_gol_step();
        s_time_since_tick = 0;

        // Pick a new bright color every tick
        s_rnd_r = rand() % 256;
        s_rnd_g = rand() % 256;
        s_rnd_b = rand() % 256;
    }

    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            if (s_grid[y][x])
            {
                gfx_draw_pixel(x, y, s_rnd_r, s_rnd_g, s_rnd_b);
            }
        }
    }
}

static const struct theme_vtable vt_rainbow = {.draw = draw_gol_rainbow};

// --- THEME 3: LAYERED/SPATIAL COLORS ---
static void draw_gol_layered(struct theme *base, struct screen *parent, float dt_ms)
{
    s_time_since_tick += dt_ms;
    if (s_time_since_tick >= GOL_TICK_RATE_MS)
    {
        simulate_gol_step();
        s_time_since_tick = 0;
    }

    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            if (s_grid[y][x])
            {
                // Example spatial color: Red at top, blue at bottom, green scales with X
                uint8_t r = 255 - (y * 4);
                uint8_t b = (y * 4);
                uint8_t g = (x * 4);
                gfx_draw_pixel(x, y, r, g, b);
            }
        }
    }
}

static const struct theme_vtable vt_layered = {.draw = draw_gol_layered};

// --- REGISTRY ---
static struct theme th_solid = {.name = "solid", .vtable = &vt_solid};
static struct theme th_rainbow = {.name = "rainbow", .vtable = &vt_rainbow};
static struct theme th_layered = {.name = "layered", .vtable = &vt_layered};

const struct theme *gol_theme_get(const char *name)
{
    if (!name)
        return &th_solid;
    if (strcmp(name, "rainbow") == 0)
        return &th_rainbow;
    if (strcmp(name, "layered") == 0)
        return &th_layered;
    return &th_solid;
}