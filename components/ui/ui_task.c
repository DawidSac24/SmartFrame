#include "ui_task.h"
#include "gfx.h"
#include "screen_scheduler.h"
#include "api_spotify.h" // Or "sp_state.h", depending on your track getter
#include "ui_cmd.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "ui_task";

static struct screen *sp_screen = NULL;
static bool s_spotify_exclusive = true;
static bool s_spotify_enabled = true; // Enabled by default

void ui_task(void *pvParameters);

esp_err_t ui_task_init(uint32_t stack_size, UBaseType_t priority)

{

    esp_err_t gfx_err = gfx_init();

    if (gfx_err != ESP_OK)

    {

        return gfx_err;
    }

    sched_init();

    ui_cmd_register();

    if (xTaskCreatePinnedToCore(ui_task, "UI_TASK", stack_size, NULL, priority, NULL, 1) != pdPASS)

    {

        ESP_LOGE(TAG, "Failed to create task!");

        return ESP_FAIL;
    }

    return ESP_OK;
}

void ui_task(void *pvParameters)
{
    int64_t last_time = esp_timer_get_time();
    static bool is_in_priority = false;
    static bool is_in_sched = false;

    // NEW: Track how long the song has been inactive
    int64_t track_inactive_start_time = 0;
    bool is_track_debouncing = false;

    while (1)
    {
        int64_t now_us = esp_timer_get_time();
        float dt_ms = (float)(now_us - last_time) / 1000.0f;
        last_time = now_us;

        if (sp_screen && s_spotify_enabled)
        {
            struct spotify_track_dto track;
            spotify_get_track_info(&track);

            // 1. Raw API state
            bool raw_track_active = (track.cover_state == SP_COVER_NEW_FILE || track.cover_state == SP_COVER_DECODED);

            // 2. Debounced state (Patient logic)
            bool debounced_track_active = raw_track_active;

            if (raw_track_active)
            {
                is_track_debouncing = false; // Reset timer when song plays
            }
            else
            {
                if (!is_track_debouncing)
                {
                    is_track_debouncing = true;
                    track_inactive_start_time = now_us;
                    debounced_track_active = true; // Pretend it's still playing!
                }
                else
                {
                    // WAIT 10 SECONDS for Wi-Fi to fetch the next cover!
                    if ((now_us - track_inactive_start_time) < 10000000)
                    {
                        debounced_track_active = true; // Keep holding priority!
                    }
                }
            }

            // 3. Route using the DEBOUNCED state
            bool should_be_priority = debounced_track_active && s_spotify_exclusive;
            bool should_be_sched = debounced_track_active && !s_spotify_exclusive;

            if (should_be_sched && !is_in_sched)
            {
                sched_add_screen(sp_screen);
                is_in_sched = true;
            }
            else if (sp_screen && !s_spotify_enabled)
            {
                if (is_in_priority)
                {
                    sched_clear_priority();
                    is_in_priority = false;
                }
                if (is_in_sched)
                {
                    sched_remove_screen(sp_screen);
                    is_in_sched = false;
                }
            }
            else if (!should_be_sched && is_in_sched)
            {
                sched_remove_screen(sp_screen);
                is_in_sched = false;
            }

            if (should_be_priority && !is_in_priority)
            {
                sched_set_priority(sp_screen);
                is_in_priority = true;
            }
            else if (!should_be_priority && is_in_priority)
            {
                sched_clear_priority();
                is_in_priority = false;
            }
        }

        gfx_clear();
        sched_tick(now_us, dt_ms);
        gfx_update();

        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

void ui_task_set_spotify_screen(struct screen *scr)
{
    sp_screen = scr;
    if (sp_screen)
    {
        sched_register_screen(sp_screen); // Auto-register in master list!
    }
}

void ui_task_set_spotify_enabled(bool enabled)
{
    s_spotify_enabled = enabled;
    if (!enabled)
    {
        sched_clear_priority();
        sched_remove_screen(sp_screen);
    }
}

bool ui_task_get_spotify_enabled(void)
{
    return s_spotify_enabled;
}

struct screen *ui_task_get_spotify_screen(void)
{
    return sp_screen;
}

void ui_task_set_spotify_exclusive(bool exclusive)
{
    s_spotify_exclusive = exclusive;
}

bool ui_task_get_spotify_exclusive(void)
{
    return s_spotify_exclusive;
}