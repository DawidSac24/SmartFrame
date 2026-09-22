#include "screen_scheduler.h"

#include "esp_timer.h"

enum sched_state
{
    SCHED_IN,
    SCHED_PLAYING,
    SCHED_OUT
};

static struct
{
    struct intr_list screens;
    struct screen *active_scr;
    struct screen *pending_scr;
    enum sched_state state;
    int64_t state_start_time;
} g_sched;

void sched_init()
{
    list_init(&g_sched.screens);
    g_sched.active_scr = NULL;
    g_sched.pending_scr = NULL;
    g_sched.state = SCHED_IN;
}

void sched_tick(int64_t now_us, float dt_ms)
{
    if (g_sched.active_scr == NULL)
        return;

    if (g_sched.state == SCHED_PLAYING)
    {
        uint32_t duration_us = g_sched.active_scr->display_duration_ms * 1000;

        if ((now_us - g_sched.state_start_time) >= duration_us)
        {
            g_sched.state = SCHED_OUT;
            g_sched.active_scr->vtable->transition_out(g_sched.active_scr);
        }
    }
    else if (g_sched.state == SCHED_OUT)
    {
        struct screen *next_scr = NULL;

        if (g_sched.pending_scr != NULL)
        {
            next_scr = g_sched.pending_scr;
            g_sched.pending_scr = NULL;
        }
        else
        {
            struct list_node *next_node = g_sched.active_scr->node.next;
            if (next_node == &g_sched.screens.head)
            {
                next_node = next_node->next;
            }
            next_scr = CONTAINER_OF(next_node, struct screen, node);
        }

        g_sched.active_scr = next_scr;

        if (g_sched.active_scr->vtable->prepare != NULL)
        {
            g_sched.active_scr->vtable->prepare(g_sched.active_scr);
        }

        g_sched.state = SCHED_IN;
        g_sched.state_start_time = now_us;
    }
    else if (g_sched.state == SCHED_IN)
    {
        // Assuming transition_in is instant for now
        g_sched.state = SCHED_PLAYING;
        g_sched.state_start_time = now_us;
    }

    g_sched.active_scr->vtable->draw(g_sched.active_scr, dt_ms);
}

void sched_jump_to(struct screen *target)
{
    if (g_sched.active_scr == target || target == NULL)
        return;

    g_sched.pending_scr = target;

    if (g_sched.state == SCHED_PLAYING || g_sched.state == SCHED_IN)
    {
        g_sched.state = SCHED_OUT;
        if (g_sched.active_scr && g_sched.active_scr->vtable->transition_out)
        {
            g_sched.active_scr->vtable->transition_out(g_sched.active_scr);
        }
    }
}

void sched_add_screen(struct screen *new_screen)
{
    list_push_back(&g_sched.screens, &new_screen->node);

    if (g_sched.active_scr == NULL)
    {
        g_sched.active_scr = new_screen;
    }
}
void sched_remove_screen(struct screen *screen)
{
    list_remove(&screen->node);
}
