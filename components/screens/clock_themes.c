#include "theme.h"
#include "gfx.h"
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void get_time_strings(char *time_str, char *date_full, char *day_str, char *month_str, char *date_str, struct tm *timeinfo)
{
    time_t now;
    time(&now);
    localtime_r(&now, timeinfo);
    const char *days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    const char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

    if (time_str)
        snprintf(time_str, 8, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    if (date_full)
        snprintf(date_full, 16, "%s %02d", months[timeinfo->tm_mon], timeinfo->tm_mday);
    if (day_str)
        snprintf(day_str, 4, "%s", days[timeinfo->tm_wday]);
    if (month_str)
        snprintf(month_str, 4, "%s", months[timeinfo->tm_mon]);
    if (date_str)
        snprintf(date_str, 4, "%02d", timeinfo->tm_mday);
}

// DIGITAL THEMES
static void draw_digital_default(struct theme *self, struct screen *parent, float dt_ms)
{
    char time_str[8];
    struct tm t;
    get_time_strings(time_str, NULL, NULL, NULL, NULL, &t);
    gfx_draw_text(12, 24, 255, 255, 255, time_str, FONT_8x16);
}

static void draw_digital_small(struct theme *self, struct screen *parent, float dt_ms)
{
    char time_str[8];
    struct tm t;
    get_time_strings(time_str, NULL, NULL, NULL, NULL, &t);
    gfx_draw_text(17, 28, 255, 255, 255, time_str, FONT_5x8);
}

static void draw_date_center(struct theme *self, struct screen *parent, float dt_ms)
{
    char time_str[8], date_full[16];
    struct tm t;
    get_time_strings(time_str, date_full, NULL, NULL, NULL, &t);
    gfx_draw_text(12, 16, 255, 255, 255, time_str, FONT_8x16);
    gfx_draw_text(14, 38, 200, 200, 200, date_full, FONT_5x8);
}

static void draw_date_left(struct theme *self, struct screen *parent, float dt_ms)
{
    char time_str[8], day_str[4], month_str[4], date_str[4];
    struct tm t;
    get_time_strings(time_str, NULL, day_str, month_str, date_str, &t);
    gfx_draw_text(4, 6, 255, 255, 255, time_str, FONT_8x16);
    gfx_draw_text(4, 28, 0, 255, 255, day_str, FONT_5x8);
    gfx_draw_text(4, 38, 200, 200, 200, month_str, FONT_5x8);
    gfx_draw_text(4, 48, 255, 255, 255, date_str, FONT_5x8);
    gfx_draw_line(2, 6, 2, 55, 100, 100, 100);
}

static void draw_clk_split(struct theme *self, struct screen *parent, float dt_ms)
{
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char hr_str[4];
    snprintf(hr_str, sizeof(hr_str), "%02d", timeinfo.tm_hour);
    char mn_str[4];
    snprintf(mn_str, sizeof(mn_str), "%02d", timeinfo.tm_min);

    gfx_draw_text(24, 14, 200, 200, 255, hr_str, FONT_8x16);
    gfx_draw_text(24, 34, 255, 200, 200, mn_str, FONT_8x16);
}

static const struct theme_vtable vt_def = {.draw = draw_digital_default};
static const struct theme_vtable vt_def_small = {.draw = draw_digital_small};
static const struct theme_vtable vt_date_cen = {.draw = draw_date_center};
static const struct theme_vtable vt_date_left = {.draw = draw_date_left};
static const struct theme_vtable vt_split = {.draw = draw_clk_split};

static struct theme th_def = {.name = "default", .vtable = &vt_def};
static struct theme th_def_small = {.name = "default_small", .vtable = &vt_def_small};
static struct theme th_date_cen = {.name = "date_center", .vtable = &vt_date_cen};
static struct theme th_date_left = {.name = "date_left", .vtable = &vt_date_left};
static struct theme th_split = {.name = "split", .vtable = &vt_split};

// ANALOG THEMES
struct theme_analog
{
    struct theme base;
    bool is_square;
    bool has_numbers;
};

static void draw_hand(int cx, int cy, float angle, int length, uint8_t r, uint8_t g, uint8_t b)
{
    int x1 = cx + (int)(length * sin(angle));
    int y1 = cy - (int)(length * cos(angle));
    gfx_draw_line(cx, cy, x1, y1, r, g, b);
}

static void draw_analog(struct theme *base, struct screen *parent, float dt_ms)
{
    struct theme_analog *self = (struct theme_analog *)base;
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    int cx = 32, cy = 32;

    for (int i = 0; i < 12; i++)
    {
        float angle = i * (M_PI / 6.0f);
        if (self->has_numbers)
        {
            char num_str[3];
            snprintf(num_str, sizeof(num_str), "%d", i == 0 ? 12 : i);
            float dx = sin(angle), dy = cos(angle);
            if (self->is_square)
            {
                float max_val = fmax(fabs(dx), fabs(dy));
                float scale = 25.0f / max_val;
                dx *= scale;
                dy *= scale;
            }
            else
            {
                dx *= 25.0f;
                dy *= 25.0f;
            }
            int nx = cx + (int)dx - (i >= 10 ? 5 : 2);
            int ny = cy - (int)dy - 4;
            uint8_t c = (i % 3 == 0) ? 255 : 100;
            gfx_draw_text(nx, ny, c, c, c, num_str, FONT_5x8);
        }
        else if (self->is_square)
        {
            float max_val = fmax(fabs(sin(angle)), fabs(cos(angle)));
            float scale_outer = 31.0f / max_val;
            float scale_inner = ((i % 3 == 0) ? 24.0f : 28.0f) / max_val;
            gfx_draw_line(cx + (int)(scale_inner * sin(angle)), cy - (int)(scale_inner * cos(angle)),
                          cx + (int)(scale_outer * sin(angle)), cy - (int)(scale_outer * cos(angle)), 200, 200, 200);
        }
        else
        {
            int r_inner = (i % 3 == 0) ? 24 : 27;
            int r_outer = 31;
            gfx_draw_line(cx + (int)(r_inner * sin(angle)), cy - (int)(r_inner * cos(angle)),
                          cx + (int)(r_outer * sin(angle)), cy - (int)(r_outer * cos(angle)), 255, 255, 255);
        }
    }

    float hr_angle = (timeinfo.tm_hour % 12) * (M_PI / 6.0f) + (timeinfo.tm_min * (M_PI / 360.0f));
    float min_angle = timeinfo.tm_min * (M_PI / 30.0f) + (timeinfo.tm_sec * (M_PI / 1800.0f));
    float sec_angle = timeinfo.tm_sec * (M_PI / 30.0f);

    draw_hand(cx, cy, hr_angle, 14, 255, 255, 255);
    draw_hand(cx, cy, min_angle, 22, 255, 255, 255);
    draw_hand(cx, cy, sec_angle, 24, 255, 0, 0);
    gfx_draw_pixel(cx, cy, 255, 255, 255);
}

static const struct theme_vtable vt_analog = {.draw = draw_analog};

static struct theme_analog th_a_rnd = {.base = {.name = "round", .vtable = &vt_analog}, .is_square = false, .has_numbers = false};
static struct theme_analog th_a_rnd_num = {.base = {.name = "round_num", .vtable = &vt_analog}, .is_square = false, .has_numbers = true};
static struct theme_analog th_a_sq = {.base = {.name = "square", .vtable = &vt_analog}, .is_square = true, .has_numbers = false};
static struct theme_analog th_a_sq_num = {.base = {.name = "square_num", .vtable = &vt_analog}, .is_square = true, .has_numbers = true};

// REGISTRY
static const struct theme *s_all_clock_themes[] = {
    &th_def,
    &th_def_small,
    &th_date_cen,
    &th_date_left,
    &th_split,
    &th_a_rnd.base,
    &th_a_rnd_num.base,
    &th_a_sq.base,
    &th_a_sq_num.base,
};

const struct theme *clock_theme_get(const char *name)
{
    if (!name)
        return &th_def;
    for (size_t i = 0; i < sizeof(s_all_clock_themes) / sizeof(s_all_clock_themes[0]); i++)
    {
        if (strcmp(s_all_clock_themes[i]->name, name) == 0)
            return s_all_clock_themes[i];
    }
    return &th_def;
}