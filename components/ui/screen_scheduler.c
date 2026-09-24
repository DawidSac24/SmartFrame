#include "screen_scheduler.h"

#include "ui_task.h"

#include "esp_timer.h"
#include <string.h>

#define MAX_REGISTERED_SCREENS 8

static struct screen *s_registry[MAX_REGISTERED_SCREENS];
static size_t s_registry_count = 0;

enum sched_state
{
    SCHED_IN,
    SCHED_PLAYING,
    SCHED_OUT
};

static struct
{
    struct intr_list screens;
    struct screen *current_scr;
    struct screen *priority_scr;
    enum sched_state state;
    int64_t state_start_time;
    uint32_t current_trans_ms;
} g_sched;

void sched_rotate_list(void);

void sched_init()
{
    list_init(&g_sched.screens);
    g_sched.current_scr = NULL;
    g_sched.priority_scr = NULL;
    g_sched.state = SCHED_IN;
    g_sched.current_trans_ms = 0;
}

void sched_tick(int64_t now_us, float dt_ms)
{
    struct screen *carousel_target = NULL;
    if (g_sched.screens.head.next != &g_sched.screens.head)
    {
        carousel_target = CONTAINER_OF(g_sched.screens.head.next, struct screen, node);
    }

    struct screen *target_screen = g_sched.priority_scr ? g_sched.priority_scr : carousel_target;
    if (!target_screen && !g_sched.current_scr)
        return;

    // 2. TRIGGER TRANSITION OUT
    if (g_sched.current_scr != target_screen && g_sched.state == SCHED_PLAYING)
    {
        g_sched.state = SCHED_OUT;
        g_sched.state_start_time = now_us;
        g_sched.current_trans_ms = 0; // Default to 0
        if (g_sched.current_scr && g_sched.current_scr->vtable->transition_out)
        {
            // Ask the screen how long it needs to animate out!
            g_sched.current_trans_ms = g_sched.current_scr->vtable->transition_out(g_sched.current_scr);
        }
    }

    // 3. STATE MACHINE TIMING
    if (g_sched.state == SCHED_PLAYING)
    {
        if (g_sched.priority_scr == NULL && g_sched.current_scr)
        {
            uint32_t dur_us = g_sched.current_scr->display_duration_ms * 1000;
            if ((now_us - g_sched.state_start_time) >= dur_us)
                sched_rotate_list();
        }
    }
    else if (g_sched.state == SCHED_OUT)
    {
        if (g_sched.current_scr == target_screen)
        { // Abort sequence if target reverted
            g_sched.state = SCHED_IN;
            g_sched.state_start_time = now_us;
            if (g_sched.current_scr && g_sched.current_scr->vtable->transition_in)
            {
                g_sched.current_trans_ms = g_sched.current_scr->vtable->transition_in(g_sched.current_scr);
            }
        }
        else if ((now_us - g_sched.state_start_time) >= ((int64_t)g_sched.current_trans_ms * 1000))
        {
            // Transition OUT complete. Safe to swap.
            g_sched.current_scr = target_screen;
            if (g_sched.current_scr && g_sched.current_scr->vtable->prepare)
            {
                g_sched.current_scr->vtable->prepare(g_sched.current_scr);
            }
            g_sched.state = SCHED_IN;
            g_sched.state_start_time = now_us;
            g_sched.current_trans_ms = 0;

            if (g_sched.current_scr && g_sched.current_scr->vtable->transition_in)
            {
                // Ask the new screen how long it needs to animate in!
                g_sched.current_trans_ms = g_sched.current_scr->vtable->transition_in(g_sched.current_scr);
            }
        }
    }
    else if (g_sched.state == SCHED_IN)
    {
        if ((now_us - g_sched.state_start_time) >= ((int64_t)g_sched.current_trans_ms * 1000))
        {
            g_sched.state = SCHED_PLAYING;
            g_sched.state_start_time = now_us;
        }
    }

    // 4. DRAW
    if (g_sched.current_scr && g_sched.current_scr->vtable->draw)
    {
        g_sched.current_scr->vtable->draw(g_sched.current_scr, dt_ms);
    }
}

void sched_register_screen(struct screen *scr)
{
    if (!scr || s_registry_count >= MAX_REGISTERED_SCREENS)
        return;
    for (size_t i = 0; i < s_registry_count; i++)
    {
        if (s_registry[i] == scr)
            return;
    }
    s_registry[s_registry_count++] = scr;
}

size_t sched_get_all_screens(struct screen **out_screens, size_t max_count)
{
    size_t count = (s_registry_count < max_count) ? s_registry_count : max_count;
    for (size_t i = 0; i < count; i++)
    {
        out_screens[i] = s_registry[i];
    }
    return count;
}

bool sched_is_screen_active(struct screen *scr)
{
    if (!scr)
        return false;

    // If checking the Spotify screen, its active status is determined by user preference
    if (scr == ui_task_get_spotify_screen())
    {
        return ui_task_get_spotify_enabled();
    }

    return list_contains(&g_sched.screens, &scr->node);
}

struct screen *sched_get_screen_by_name(const char *target)
{
    if (!target)
        return NULL;
    for (size_t i = 0; i < s_registry_count; i++)
    {
        if (s_registry[i]->name && strcmp(s_registry[i]->name, target) == 0)
        {
            return s_registry[i];
        }
    }
    return NULL;
}

void sched_add_screen(struct screen *new_screen)
{
    if (!new_screen)
        return;

    // Auto-register in the master list so the web server knows about it!
    sched_register_screen(new_screen);

    if (!list_contains(&g_sched.screens, &new_screen->node))
    {
        list_push_back(&g_sched.screens, &new_screen->node);
    }

    if (g_sched.current_scr == NULL)
    {
        g_sched.current_scr = new_screen;
    }
}
void sched_remove_screen(struct screen *screen)
{
    if (!screen)
        return;
    if (!list_contains(&g_sched.screens, &screen->node))
        return;

    list_remove(&screen->node);
}

void sched_set_priority(struct screen *target)
{
    g_sched.priority_scr = target;
}

void sched_clear_priority(void)
{
    g_sched.priority_scr = NULL;
}

void sched_next(void)
{
    if (g_sched.priority_scr == NULL)
    {
        sched_rotate_list();
    }
    // We removed the forced transition code here.
    // rotating the list changes the target, which sched_tick handles gracefully!
}

void sched_rotate_list(void)
{
    if (!list_is_empty(&g_sched.screens))
    {
        struct list_node *first = g_sched.screens.head.next;
        list_remove(first);
        list_push_back(&g_sched.screens, first);
    }
}