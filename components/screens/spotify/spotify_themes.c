#include "theme.h"
#include "gfx.h"
#include "api_spotify.h" // Or "sp_state.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static uint8_t s_rgb_buffer[64 * 64 * 3];
static bool s_has_valid_art = false; // NEW: Persists the art state during transitions!

static void trigger_decode(struct spotify_track_dto *track)
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
    sp_state_set_track_info(track); // Push state back to prevent infinite loops
}

static void get_cover_pixel(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b)
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

static uint8_t calculate_luma(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint8_t)((r * 299 + g * 587 + b * 114) / 1000);
}

// ============================================================================
// DEFAULT THEME
// ============================================================================
static void draw_sp_default(struct theme *self, struct screen *parent, float dt_ms)
{
    struct spotify_track_dto track;
    spotify_get_track_info(&track);

    if (track.cover_state == SP_COVER_NEW_FILE)
    {
        trigger_decode(&track);
    }

    // Rely on the cached boolean so it stays visible during the 1.5s transition out!
    if (!s_has_valid_art)
        return;

    uint8_t r, g, b;
    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            get_cover_pixel(x, y, &r, &g, &b);
            gfx_draw_pixel(x, y, r, g, b);
        }
    }
}

// ============================================================================
// SQUARE THEME
// ============================================================================
struct theme_square
{
    struct theme base;
    float text_scroll_x;
    char last_track_name[64]; // Caches name so it doesn't vanish during transitions
};

static void draw_sp_square(struct theme *base, struct screen *parent, float dt_ms)
{
    struct theme_square *self = (struct theme_square *)base;
    struct spotify_track_dto track;
    spotify_get_track_info(&track);

    if (track.cover_state == SP_COVER_NEW_FILE)
    {
        trigger_decode(&track);
    }

    if (!s_has_valid_art)
        return;

    // Only update cache if we have a valid song (prevents wiping name during transition out)
    if (track.cover_state == SP_COVER_DECODED || track.cover_state == SP_COVER_NEW_FILE)
    {
        if (strcmp(self->last_track_name, track.track_name) != 0)
        {
            strncpy(self->last_track_name, track.track_name, sizeof(self->last_track_name) - 1);
            self->last_track_name[sizeof(self->last_track_name) - 1] = '\0';
            self->text_scroll_x = 4.0f; // Reset scroll delay
        }
    }

    uint8_t r, g, b;
    uint32_t tot_r = 0, tot_g = 0, tot_b = 0;
    int samples = 0;

    for (int y = 0; y < 64; y += 4)
    {
        for (int x = 0; x < 64; x += 4)
        {
            get_cover_pixel(x, y, &r, &g, &b);
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
        {
            gfx_draw_pixel(x, y, bg_r, bg_g, bg_b);
        }
    }

    int art_size = 52;
    int art_offset_x = (64 - art_size) / 2;
    int art_offset_y = 2;

    for (int y = 0; y < art_size; y++)
    {
        for (int x = 0; x < art_size; x++)
        {
            int src_x = x + (64 - art_size) / 2;
            int src_y = y + (64 - art_size) / 2;
            get_cover_pixel(src_x, src_y, &r, &g, &b);
            gfx_draw_pixel(x + art_offset_x, y + art_offset_y, r, g, b);
        }
    }

    uint8_t luma = calculate_luma(bg_r, bg_g, bg_b);
    uint8_t text_color = (luma > 128) ? 0 : 255;

    // SCROLL MATH (uses cached name!)
    int text_len = strlen(self->last_track_name);
    int text_width = text_len * 6;
    int text_x;

    if (text_width <= 64)
    {
        text_x = (64 - text_width) / 2;
    }
    else
    {
        self->text_scroll_x -= (dt_ms / 1000.0f) * 18.0f;
        if (self->text_scroll_x < -text_width)
        {
            self->text_scroll_x = 64.0f;
        }
        text_x = (int)self->text_scroll_x;
    }

    int text_y = art_offset_y + art_size + 2;
    uint8_t shadow_c = (text_color == 255) ? 0 : 255;

    gfx_draw_text(text_x + 1, text_y + 1, shadow_c, shadow_c, shadow_c, self->last_track_name, FONT_5x8);
    gfx_draw_text(text_x, text_y, text_color, text_color, text_color, self->last_track_name, FONT_5x8);
}

// ============================================================================
// DISK THEMES
// ============================================================================
enum disk_anim_state
{
    ANIM_IDLE,
    ANIM_SWAP_LIFT_ARM,
    ANIM_SWAP_SLIDE_DOWN,
    ANIM_SWAP_DECODE,
    ANIM_IN_SLIDE_UP,
    ANIM_IN_DROP_ARM,
    ANIM_OUT_LIFT_ARM,
    ANIM_OUT_SLIDE_DOWN,
    ANIM_OUT_FINISHED
};

struct theme_disk
{
    struct theme base;
    bool spins;
    float current_angle;
    float current_arm_angle;
    float disk_y;
    enum disk_anim_state state;
};

static void disk_trans_in(struct theme *base, struct screen *parent)
{
    struct theme_disk *self = (struct theme_disk *)base;
    self->current_arm_angle = 0.0f;
    self->disk_y = 64.0f;

    struct spotify_track_dto track;
    spotify_get_track_info(&track);

    if (track.cover_state == SP_COVER_NEW_FILE)
    {
        self->state = ANIM_SWAP_DECODE;
    }
    else
    {
        self->state = ANIM_IN_SLIDE_UP;
    }
}

static void disk_trans_out(struct theme *base, struct screen *parent)
{
    struct theme_disk *self = (struct theme_disk *)base;
    self->state = ANIM_OUT_LIFT_ARM;
}

static void draw_sp_disk(struct theme *base, struct screen *parent, float dt_ms)
{
    struct theme_disk *self = (struct theme_disk *)base;
    struct spotify_track_dto track;
    spotify_get_track_info(&track);

    if (track.cover_state == SP_COVER_NEW_FILE && self->state == ANIM_IDLE)
    {
        self->state = ANIM_SWAP_LIFT_ARM;
    }

    float arm_speed = 0.4f;
    float slide_speed = 45.0f;
    float target_arm = self->current_arm_angle;
    float target_y = self->disk_y;

    switch (self->state)
    {
    case ANIM_SWAP_LIFT_ARM:
    case ANIM_OUT_LIFT_ARM:
        target_arm = 0.0f;
        if (self->current_arm_angle <= 0.01f)
            self->state = (self->state == ANIM_SWAP_LIFT_ARM) ? ANIM_SWAP_SLIDE_DOWN : ANIM_OUT_SLIDE_DOWN;
        break;

    case ANIM_SWAP_SLIDE_DOWN:
    case ANIM_OUT_SLIDE_DOWN:
        target_y = 64.0f;
        if (self->disk_y >= 63.9f)
            self->state = (self->state == ANIM_SWAP_SLIDE_DOWN) ? ANIM_SWAP_DECODE : ANIM_OUT_FINISHED;
        break;

    case ANIM_SWAP_DECODE:
        trigger_decode(&track);
        self->state = ANIM_IN_SLIDE_UP;
        break;

    case ANIM_IN_SLIDE_UP:
        target_y = 0.0f;
        if (self->disk_y <= 0.1f)
            self->state = ANIM_IN_DROP_ARM;
        break;

    case ANIM_IN_DROP_ARM:
        target_arm = 0.5f;
        if (self->current_arm_angle >= 0.49f)
            self->state = ANIM_IDLE;
        break;

    case ANIM_IDLE:
        target_arm = 0.5f;
        target_y = 0.0f;
        break;

    case ANIM_OUT_FINISHED:
        target_arm = 0.0f;
        target_y = 64.0f;
        break;
    }

    // Kinematics
    if (self->current_arm_angle < target_arm)
    {
        self->current_arm_angle += (dt_ms / 1000.0f) * arm_speed;
        if (self->current_arm_angle > target_arm)
            self->current_arm_angle = target_arm;
    }
    else if (self->current_arm_angle > target_arm)
    {
        self->current_arm_angle -= (dt_ms / 1000.0f) * arm_speed;
        if (self->current_arm_angle < target_arm)
            self->current_arm_angle = target_arm;
    }

    if (self->disk_y < target_y)
    {
        self->disk_y += (dt_ms / 1000.0f) * slide_speed;
        if (self->disk_y > target_y)
            self->disk_y = target_y;
    }
    else if (self->disk_y > target_y)
    {
        self->disk_y -= (dt_ms / 1000.0f) * slide_speed;
        if (self->disk_y < target_y)
            self->disk_y = target_y;
    }

    int d_y_offset = (int)self->disk_y;

    // Render using s_has_valid_art so the disk stays visible while sliding out!
    if (d_y_offset < 64 && s_has_valid_art)
    {
        if (self->spins)
        {
            self->current_angle += (dt_ms / 1000.0f) * 0.4f;
            if (self->current_angle > 2 * M_PI)
                self->current_angle -= 2 * M_PI;
        }

        float c = cos(self->current_angle);
        float s = sin(self->current_angle);
        uint8_t r, g, b;

        for (int y = 0; y < 64; y++)
        {
            int screen_y = y + d_y_offset;
            if (screen_y >= 64)
                break;

            for (int x = 0; x < 64; x++)
            {
                float dx = x - 32;
                float dy = y - 32;
                float dist_sq = dx * dx + dy * dy;

                if (dist_sq <= 30 * 30)
                {
                    if (dist_sq <= 4 * 4)
                    {
                        gfx_draw_pixel(x, screen_y, 20, 20, 20);
                    }
                    else
                    {
                        int src_x = 32 + (int)(dx * c - dy * s);
                        int src_y = 32 + (int)(dx * s + dy * c);
                        get_cover_pixel(src_x, src_y, &r, &g, &b);

                        if (dist_sq > 25 * 25 && (int)(dist_sq) % 3 == 0)
                        {
                            r /= 2;
                            g /= 2;
                            b /= 2;
                        }
                        gfx_draw_pixel(x, screen_y, r, g, b);
                    }
                }
            }
        }
    }

    // Always draw arm
    int pivot_x = 61;
    int pivot_y = -2;
    int arm_length = 28;
    int needle_x = pivot_x - (int)(arm_length * sin(self->current_arm_angle));
    int needle_y = pivot_y + (int)(arm_length * cos(self->current_arm_angle));

    gfx_draw_line(pivot_x, pivot_y, needle_x, needle_y, 180, 180, 180);
    gfx_draw_line(pivot_x + 1, pivot_y, needle_x + 1, needle_y, 100, 100, 100);
    gfx_draw_line(needle_x - 1, needle_y - 1, needle_x + 1, needle_y + 1, 80, 80, 80);
    gfx_draw_line(needle_x - 1, needle_y, needle_x + 1, needle_y + 2, 50, 50, 50);
}

// ... Registry remains the same ...

// ============================================================================
// REGISTRY
// ============================================================================

static const struct theme_vtable vt_default = {.draw = draw_sp_default};
static const struct theme_vtable vt_square = {.draw = draw_sp_square};
static const struct theme_vtable vt_disk = {.draw = draw_sp_disk, .transition_in = disk_trans_in, .transition_out = disk_trans_out};

// Instances
static struct theme th_def = {.name = "default", .vtable = &vt_default};
static struct theme_square th_sq = {.base = {.name = "square", .vtable = &vt_square}, .text_scroll_x = 0.0f, .last_track_name = {0}};
static struct theme_disk th_d = {.base = {.name = "disk", .vtable = &vt_disk}, .spins = false};
static struct theme_disk th_s = {.base = {.name = "spinning_disk", .vtable = &vt_disk}, .spins = true};

static const struct theme *s_all_sp_themes[] = {
    &th_def,
    &th_sq.base,
    &th_d.base,
    &th_s.base,
};

const struct theme *spotify_theme_get(const char *name)
{
    if (!name)
        return &th_def;
    for (size_t i = 0; i < sizeof(s_all_sp_themes) / sizeof(s_all_sp_themes[0]); i++)
    {
        if (strcmp(s_all_sp_themes[i]->name, name) == 0)
            return s_all_sp_themes[i];
    }
    return &th_def;
}