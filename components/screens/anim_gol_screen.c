#include "anim_gol_screen.h"
#include <stddef.h>

extern const struct theme *gol_theme_get(const char *name);

static void gol_prepare(struct screen *self)
{
    // Optional: You could randomly seed the GOL grid here every time the screen rotates in!
}
static uint32_t gol_trans_in(struct screen *self) { return 0; }
static uint32_t gol_trans_out(struct screen *self) { return 0; }
static void gol_destroy(struct screen *self) {}

static void gol_draw(struct screen *self, float dt_ms)
{
    if (self->active_theme && self->active_theme->vtable->draw)
    {
        self->active_theme->vtable->draw((struct theme *)self->active_theme, self, dt_ms);
    }
}

static const struct screen_vtable s_gol_vtable = {
    .prepare = gol_prepare,
    .draw = gol_draw,
    .transition_in = gol_trans_in,
    .transition_out = gol_trans_out,
    .destroy = gol_destroy,
    .get_theme = gol_theme_get};

static struct screen s_gol_instance;

struct screen *screen_gol_init(uint32_t duration_ms)
{
    s_gol_instance.vtable = &s_gol_vtable;
    s_gol_instance.name = "game_of_life";
    s_gol_instance.display_duration_ms = duration_ms;
    s_gol_instance.active_theme = gol_theme_get("solid");

    return &s_gol_instance;
}