#include "spotify_theme_priv.h"
#include "gfx.h"
#include <string.h>

static uint8_t s_rgb_buffer[64 * 64 * 3];
static bool s_has_valid_art = false;

void sp_theme_trigger_decode(struct spotify_track_dto *track)
{
    if (gfx_decode_jpeg("/fs/album.jpg", s_rgb_buffer, sizeof(s_rgb_buffer)) == ESP_OK)
    {
        track->cover_state = SP_COVER_DECODED;
        s_has_valid_art = true;
    }
    else
    {
        track->cover_state = SP_COVER_FAILED;
        s_has_valid_art = false;
    }
    sp_state_set_track_info(track);
}

bool sp_theme_prepare_manual(struct spotify_track_dto *track)
{
    if (spotify_get_track_info(track) != ESP_OK)
        return false;
    return (track->cover_state == SP_COVER_DECODED || track->cover_state == SP_COVER_NEW_FILE);
}

bool sp_theme_prepare_auto(struct spotify_track_dto *track)
{
    if (spotify_get_track_info(track) != ESP_OK)
        return false;
    if (track->cover_state == SP_COVER_NEW_FILE)
        sp_theme_trigger_decode(track);
    return (track->cover_state == SP_COVER_DECODED);
}

void sp_theme_get_pixel(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b)
{
    if (x < 0 || x >= 64 || y < 0 || y >= 64)
    {
        *r = 0;
        *g = 0;
        *b = 0;
        return;
    }
    int idx = (y * 64 + x) * 3;
    *r = s_rgb_buffer[idx];
    *g = s_rgb_buffer[idx + 1];
    *b = s_rgb_buffer[idx + 2];
}

uint8_t sp_theme_calc_luma(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint8_t)((r * 299 + g * 587 + b * 114) / 1000);
}

bool sp_theme_has_art(void) { return s_has_valid_art; }

// ==========================
// REGISTRY
// ==========================
static struct theme th_def = {.name = "default", .vtable = &vt_sp_default};

// (Assume definitions for theme_square and theme_disk are injected by the compiler
// or you can just cast generic structs since this is a simple factory).
extern struct theme th_sq_instance;
extern struct theme th_d_instance;
extern struct theme th_s_instance;

static const struct theme *s_all_sp_themes[] = {
    &th_def, &th_sq_instance, &th_d_instance, &th_s_instance};

const struct theme *spotify_theme_get(const char *name)
{
    if (!name)
        return &th_def;
    for (size_t i = 0; i < 4; i++)
    {
        if (strcmp(s_all_sp_themes[i]->name, name) == 0)
            return s_all_sp_themes[i];
    }
    return &th_def;
}