#pragma once

#include "screen.h"

#include "esp_err.h"

void sched_init();
void sched_tick(int64_t now_us, float dt_ms);

struct screen *sched_get_screen_by_name(const char *name);

void sched_register_screen(struct screen *scr);
struct screen *sched_get_screen_by_name(const char *target);
size_t sched_get_all_screens(struct screen **out_screens, size_t max_count);
bool sched_is_screen_active(struct screen *scr);

void sched_add_screen(struct screen *new_screen);
void sched_remove_screen(struct screen *screen);

void sched_set_priority(struct screen *target);
void sched_clear_priority(void);
void sched_next(void);