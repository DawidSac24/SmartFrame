#include "screen.h"

#include "esp_err.h"

void sched_init();

void sched_tick(int64_t now_us, float dt_ms);

void sched_jump_to(struct screen *target);

void sched_add_screen(struct screen *new_screen);
void sched_remove_screen(struct screen *screen);