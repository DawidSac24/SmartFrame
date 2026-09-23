#include "clock_screen.h"

#include <stddef.h>

// Forward-declare the theme getter from clock_themes.c
extern const struct theme *clock_theme_get(const char *name);

static void clk_prepare(struct screen *self) {}
static uint32_t clk_trans_in(struct screen *self) { return 0; }
static uint32_t clk_trans_out(struct screen *self) { return 0; }
static void clk_destroy(struct screen *self) {}

static void clk_draw(struct screen *self, float dt_ms)
{
    // Delegate to the active theme!
    if (self->active_theme && self->active_theme->vtable->draw)
    {
        self->active_theme->vtable->draw((struct theme *)self->active_theme, self, dt_ms);
    }
}

static const struct screen_vtable s_clock_vtable = {
    .prepare = clk_prepare,
    .draw = clk_draw,
    .transition_in = clk_trans_in,
    .transition_out = clk_trans_out,
    .destroy = clk_destroy,
    .get_theme = clock_theme_get // UI engine calls this when you type "ui set clock theme xyz"
};

static struct screen s_clk_instance;

struct screen *screen_clock_init(uint32_t duration_ms)
{
    s_clk_instance.vtable = &s_clock_vtable;
    s_clk_instance.name = "clock";
    s_clk_instance.display_duration_ms = duration_ms;

    // Set your starting default theme here:
    s_clk_instance.active_theme = clock_theme_get("round");

    return &s_clk_instance;
}