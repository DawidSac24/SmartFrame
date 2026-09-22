#include "ui_task.h"

#include "gfx.h"
#include "screen_scheduler.h"
#include "spotify_screen.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

#include <stdint.h>

static const char *TAG = "ui_task";

void ui_task(void *pvParameters);
void ui_init_screens();

esp_err_t ui_task_init()
{
    esp_err_t gfx_err = gfx_init();
    if (gfx_err != ESP_OK)
    {
        return gfx_err;
    }
    sched_init();

    if (xTaskCreatePinnedToCore(ui_task, "UI_TASK", 4096, NULL, 5, NULL, 1) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create task!");
        return ESP_FAIL;
    }

    ui_init_screens();

    return ESP_OK;
}

void ui_task(void *pvParameters)
{

    int64_t last_time = esp_timer_get_time();

    while (1)
    {
        int64_t now_us = esp_timer_get_time();
        float dt_ms = (float)(now_us - last_time) / 1000.0f;
        last_time = now_us;

        gfx_clear();

        sched_tick(now_us, dt_ms);

        gfx_update();

        vTaskDelay(pdMS_TO_TICKS(16)); // Target ~60 FPS
    }
}

void ui_init_screens()
{
    struct screen *sp_screen = screen_spotify_create(10 * 60);

    sched_add_screen(sp_screen);
}