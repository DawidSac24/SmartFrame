#include "weather_screen.h"
#include <stddef.h>

extern const struct theme *weather_theme_get(const char *name);

static void wth_prepare(struct screen *self) {}
static uint32_t wth_trans_in(struct screen *self) { return 0; }
static uint32_t wth_trans_out(struct screen *self) { return 0; }
static void wth_destroy(struct screen *self) {}

static void wth_draw(struct screen *self, float dt_ms)
{
    if (self->active_theme && self->active_theme->vtable->draw)
    {
        self->active_theme->vtable->draw((struct theme *)self->active_theme, self, dt_ms);
    }
}

static const struct screen_vtable s_weather_vtable = {
    .prepare = wth_prepare,
    .draw = wth_draw,
    .transition_in = wth_trans_in,
    .transition_out = wth_trans_out,
    .destroy = wth_destroy,
    .get_theme = weather_theme_get};

static struct screen s_wth_instance;

struct screen *screen_weather_init(uint32_t duration_ms)
{
    s_wth_instance.vtable = &s_weather_vtable;
    s_wth_instance.name = "weather";
    s_wth_instance.display_duration_ms = duration_ms;
    s_wth_instance.active_theme = weather_theme_get("default");

    return &s_wth_instance;
}