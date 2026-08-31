#include "api_spotify.h"
#include "spotify_prv.h"

#include "esp_log.h"

#include "app_state.h"
#include "wifi.h"

static QueueHandle_t auth_code_queue;

void api_spotify_task(void *pvParameters);

TaskHandle_t api_spotify_start_task(void)
{
    auth_code_queue = xQueueCreate(1, 350 * sizeof(char));

    return ESP_OK;
}

void api_spotify_delete_task(TaskHandle_t task_handle)
{
    if (task_handle != NULL)
    {
        vTaskDelete(task_handle);
    }
}

void api_spotify_task(void *pvParameters)
{
}

esp_err_t api_spotify_send_auth_code(const char *auth_code)
{
    if (auth_code_queue == NULL)
    {
        return ESP_FAIL;
    }

    if (xQueueSend(auth_code_queue, auth_code, 0) == pdPASS)
    {
        ESP_LOGI("SPOTIFY", "Auth code successfully injected into Queue!");
        return ESP_OK;
    }
    else
    {
        ESP_LOGE("SPOTIFY", "Queue full! Auth code dropped.");
        return ESP_ERR_TIMEOUT;
    }
}
