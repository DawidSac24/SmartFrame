#include "spotify_screen.h"
#include "api_spotify.h"

#include "gfx.h" // Your drawing/JPEG wrappers

struct spotify_screen
{
    struct screen base;
    enum spotify_theme theme;
    // Animation states
    float rotation_angle;
    float tonearm_angle; // 0 = resting, 30 = on record
    uint8_t *rgb_buffer;
};

static void sp_prepare(struct screen *self);
static void sp_draw(struct screen *self, float dt_ms);
static void sp_trans_in(struct screen *self);
static void sp_trans_out(struct screen *self);
static void sp_destroy(struct screen *self);

static const struct screen_vtable s_spotify_vtable = {
    .prepare = sp_prepare,
    .draw = sp_draw,
    .transition_in = sp_trans_in,
    .transition_out = sp_trans_out,
    .destroy = sp_destroy};

struct screen *screen_spotify_create(uint32_t duration_ms)
{
    // TODO: Use an allocator later
    struct spotify_screen *sp_scr = calloc(1, sizeof(struct spotify_screen));
    if (!sp_scr)
        return NULL;

    sp_scr->base.vtable = &s_spotify_vtable;
    sp_scr->base.display_duration_ms = duration_ms;
    sp_scr->theme = SP_THEME_DEFAULT;
    sp_scr->rotation_angle = 0.0f;
    sp_scr->tonearm_angle = 0.0f;
    sp_scr->rgb_buffer = NULL;

    return (struct screen *)sp_scr;
}

static void sp_prepare(struct screen *self)
{
    // struct spotify_screen *sp_scr = (struct spotify_screen *)self;
}

static void sp_trans_in(struct screen *self)
{
    // struct spotify_screen *sp_scr = (struct spotify_screen *)self;
    // We will animate tonearm_angle from 0 to 30 here
}

static void sp_trans_out(struct screen *self)
{
    // struct spotify_screen *sp_scr = (struct spotify_screen *)self;
    // We will animate tonearm_angle from 30 back to 0 here
}

static void sp_draw(struct screen *self, float dt_ms)
{
    struct spotify_screen *sp_scr = (struct spotify_screen *)self;

    if (!sp_scr->rgb_buffer)
        return;

    // 1. Fetch current track info
    struct spotify_track_dto track;
    if (spotify_get_track_info(&track) != ESP_OK)
        return;

    if (track.cover_state == SP_COVER_NONE || track.cover_state == SP_COVER_FAILED)
    {
        sched_next();
        return;
    }

    // // 3. The One-Time Decode
    // if (track.cover_state == SP_COVER_NEW_FILE && sp_scr->rgb_buffer != NULL)
    // {
    //     if (gfx_decode_jpeg("/fs/album.jpg", sp_scr->rgb_buffer, RGB_BUFFER_SIZE) == ESP_OK)
    //     {
    //         track.cover_state = SP_COVER_DECODED;
    //     }
    //     else
    //     {
    //         track.cover_state = SP_COVER_FAILED;
    //     }

    //     // Push the decoded/failed state back to prevent infinite decoding loops
    //     sp_state_set_track_info(&track);
    // }

    // if (sp_scr->theme == SP_THEME_DEFAULT)
    // {
    //     if (gfx_decode_jpeg("/fs/album.jpg", sp_scr->rgb_buffer, sizeof(sp->scr)) == ESP_OK)
    //     {
    //         new_track.cover_state = SP_COVER_DECODED;
    //         gfx_draw_screen(g_album_rgb_buffer);
    //     }
    //     else
    //     {
    //         new_track.cover_state = SP_COVER_FAILED;
    //     }

    //     gfx_draw_pixels(0, 0, 64, 64, sp_scr->rgb_buffer);
    // }
    // else if (sp_scr->theme == SP_THEME_DISK)
    // {
    //     // Tomorrow: We will draw this using a circular mask
    // }
    // else if (sp_scr->theme == SP_THEME_SPINNING_DISK)
    // {
    //     // Day 3: We will implement nearest-neighbor rotation math here
    //     // sp_scr->rotation_angle += speed * dt_ms;
    // }
}

static void sp_destroy(struct screen *self)
{
    struct spotify_screen *sp_scr = (struct spotify_screen *)self;

    // Free the decoded image buffer
    if (sp_scr->rgb_buffer)
    {
        free(sp_scr->rgb_buffer);
    }

    // Free the struct itself (Swap to pool_free later)
    free(sp_scr);
}