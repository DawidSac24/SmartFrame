#include "spotify_theme_priv.h"
#include "gfx.h"
#include <string.h>

static void draw_sp_default(struct theme *self, struct screen *parent, float dt_ms)
{
    struct spotify_track_dto track;
    spotify_get_track_info(&track);
    if (track.cover_state == SP_COVER_NEW_FILE)
        sp_theme_trigger_decode(&track);
    if (!sp_theme_has_art())
        return;

    uint8_t r, g, b;
    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            sp_theme_get_pixel(x, y, &r, &g, &b);
            gfx_draw_pixel(x, y, r, g, b);
        }
    }

    if (!track.is_playing)
    {
        for (int i = 0; i < 4; i++)
        {
            gfx_draw_line(26 + i, 26, 26 + i, 38, 255, 255, 255); // Left bar
            gfx_draw_line(34 + i, 26, 34 + i, 38, 255, 255, 255); // Right bar
        }
    }
}

struct theme_square
{
    struct theme base;
    float text_scroll_x;
    char last_track_name[64];
};

static void draw_sp_square(struct theme *base, struct screen *parent, float dt_ms)
{
    struct theme_square *self = (struct theme_square *)base;
    struct spotify_track_dto track;
    spotify_get_track_info(&track);

    if (track.cover_state == SP_COVER_NEW_FILE)
        sp_theme_trigger_decode(&track);
    if (!sp_theme_has_art())
        return;

    if (track.cover_state == SP_COVER_DECODED || track.cover_state == SP_COVER_NEW_FILE)
    {
        if (strcmp(self->last_track_name, track.track_name) != 0)
        {
            strncpy(self->last_track_name, track.track_name, sizeof(self->last_track_name) - 1);
            self->text_scroll_x = 4.0f;
        }
    }

    uint8_t r, g, b;
    uint32_t tot_r = 0, tot_g = 0, tot_b = 0;
    int samples = 0;
    for (int y = 0; y < 64; y += 4)
    {
        for (int x = 0; x < 64; x += 4)
        {
            sp_theme_get_pixel(x, y, &r, &g, &b);
            tot_r += r;
            tot_g += g;
            tot_b += b;
            samples++;
        }
    }
    uint8_t bg_r = tot_r / samples;
    uint8_t bg_g = tot_g / samples;
    uint8_t bg_b = tot_b / samples;

    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 64; x++)
            gfx_draw_pixel(x, y, bg_r, bg_g, bg_b);
    }

    int art_size = 52, art_offset_x = 6, art_offset_y = 2;
    for (int y = 0; y < art_size; y++)
    {
        for (int x = 0; x < art_size; x++)
        {
            sp_theme_get_pixel(x + 6, y + 6, &r, &g, &b);
            gfx_draw_pixel(x + art_offset_x, y + art_offset_y, r, g, b);
        }
    }

    uint8_t text_c = (sp_theme_calc_luma(bg_r, bg_g, bg_b) > 128) ? 0 : 255;
    int text_len = strlen(self->last_track_name), text_width = text_len * 6, text_x;
    if (text_width <= 64)
        text_x = (64 - text_width) / 2;
    else
    {
        self->text_scroll_x -= (dt_ms / 1000.0f) * 18.0f;
        if (self->text_scroll_x < -text_width)
            self->text_scroll_x = 64.0f;
        text_x = (int)self->text_scroll_x;
    }

    uint8_t shadow_c = (text_c == 255) ? 0 : 255;
    gfx_draw_text(text_x + 1, art_size + 5, shadow_c, shadow_c, shadow_c, self->last_track_name, FONT_5x8);
    gfx_draw_text(text_x, art_size + 4, text_c, text_c, text_c, self->last_track_name, FONT_5x8);
    if (!track.is_playing)
    {
        for (int i = 0; i < 4; i++)
        {
            gfx_draw_line(26 + i, 22, 26 + i, 34, 255, 255, 255); // Left bar
            gfx_draw_line(34 + i, 22, 34 + i, 34, 255, 255, 255); // Right bar
        }
    }
}

const struct theme_vtable vt_sp_default = {.draw = draw_sp_default};
const struct theme_vtable vt_sp_square = {.draw = draw_sp_square};

struct theme_square th_sq_instance = {.base = {.name = "square", .vtable = &vt_sp_square}, .text_scroll_x = 0.0f, .last_track_name = {0}};