#include "spotify_screen.h"
#include <stddef.h>

extern const struct theme *spotify_theme_get(const char *name);

static uint32_t sp_trans_in(struct screen *self)
{
    if (self->active_theme && self->active_theme->vtable->transition_in)
        return self->active_theme->vtable->transition_in((struct theme *)self->active_theme, self);
    return 0;
}

static uint32_t sp_trans_out(struct screen *self)
{
    if (self->active_theme && self->active_theme->vtable->transition_out)
        return self->active_theme->vtable->transition_out((struct theme *)self->active_theme, self);
    return 0;
}

static void sp_draw(struct screen *self, float dt_ms)
{
    if (self->active_theme && self->active_theme->vtable->draw)
        self->active_theme->vtable->draw((struct theme *)self->active_theme, self, dt_ms);
}

static const struct screen_vtable s_spotify_vtable = {
    .draw = sp_draw,
    .transition_in = sp_trans_in,
    .transition_out = sp_trans_out,
    .get_theme = spotify_theme_get};

static struct screen s_sp_instance;

struct screen *screen_spotify_init(uint32_t duration_ms)
{
    s_sp_instance.vtable = &s_spotify_vtable;
    s_sp_instance.name = "spotify";
    s_sp_instance.display_duration_ms = duration_ms;
    s_sp_instance.active_theme = spotify_theme_get("spinning_disk");
    return &s_sp_instance;
}