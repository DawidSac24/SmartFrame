#include "spotify_theme_priv.h"
#include "gfx.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

static uint32_t disk_trans_in(struct theme *base, struct screen *parent)
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

    return 1500; // Tell the scheduler to give us 1.5 seconds!
}

static uint32_t disk_trans_out(struct theme *base, struct screen *parent)
{
    struct theme_disk *self = (struct theme_disk *)base;
    self->state = ANIM_OUT_LIFT_ARM;

    return 1500; // Tell the scheduler to give us 1.5 seconds!
}

static void draw_sp_disk(struct theme *base, struct screen *parent, float dt_ms)
{
    struct theme_disk *self = (struct theme_disk *)base;
    struct spotify_track_dto track;

    // FETCH track info, but NEVER abort.
    // We must keep drawing the old cached art while the transition out plays!
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
        sp_theme_trigger_decode(&track); // Use the global helper!
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
        // Automatically lift the arm if the track is paused!
        target_arm = track.is_playing ? 0.5f : 0.0f;
        target_y = 0.0f;
        break;

    case ANIM_OUT_FINISHED:
        target_arm = 0.0f;
        target_y = 64.0f;
        break;
    }

    // 1. Arm Kinematics (Move towards target_arm ALWAYS)
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

    // 2. Disk Slide Kinematics (Move towards target_y ALWAYS)
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

    // 3. Draw the Record
    if (d_y_offset < 64 && sp_theme_has_art())
    {
        // ONLY spin if theme spins, track is playing, AND arm is actually on the record
        if (self->spins && track.is_playing && self->current_arm_angle > 0.48f)
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
                        gfx_draw_pixel(x, screen_y, 20, 20, 20); // Spindle
                    }
                    else
                    {
                        int src_x = 32 + (int)(dx * c - dy * s);
                        int src_y = 32 + (int)(dx * s + dy * c);

                        sp_theme_get_pixel(src_x, src_y, &r, &g, &b);

                        if (dist_sq > 25 * 25 && (int)(dist_sq) % 3 == 0)
                        {
                            r /= 2;
                            g /= 2;
                            b /= 2; // Grooves
                        }
                        gfx_draw_pixel(x, screen_y, r, g, b);
                    }
                }
            }
        }
    }

    // 4. Always draw arm
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

// ----------------------------------------------------------------------------
// EXPOSE VTABLE & INSTANCES TO CORE
// ----------------------------------------------------------------------------

const struct theme_vtable vt_sp_disk = {
    .draw = draw_sp_disk,
    .transition_in = disk_trans_in,
    .transition_out = disk_trans_out};

struct theme_disk th_d_instance = {.base = {.name = "disk", .vtable = &vt_sp_disk}, .spins = false};
struct theme_disk th_s_instance = {.base = {.name = "spinning_disk", .vtable = &vt_sp_disk}, .spins = true};