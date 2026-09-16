#include "api_weather.h"
#include "weather_priv.h"

#include "app_state.h"
#include "wifi.h"

static const char *TAG = "weather_api";

static struct localisation s_localisation;

QueueHandle_t weather_cmd_queue = NULL;

void api_weather_task(void *pvParameters);

TaskHandle_t api_weather_start_task(float latitude, float longitude)
{
    s_localisation.latitude = latitude;
    s_localisation.longitude = longitude;

    BaseType_t returned;
    TaskHandle_t task_handle = NULL;

    weather_cmd_register();

    returned = xTaskCreatePinnedToCore(api_weather_task, "api_weather_task", 8192, &s_localisation, PRIO_WEATHER_API, &task_handle, 0);
    if (returned != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create task!");
        return NULL;
    }

    return task_handle;
}

void api_weather_delete_task(TaskHandle_t task_handle)
{
    if (task_handle != NULL)
    {
        vTaskDelete(task_handle);
    }
}

void api_weather_task(void *pvParameters)
{
    struct localisation *localisation = (struct localisation *)pvParameters;

    TickType_t current_delay = 0;
    const TickType_t POLL_INTERVAL_MS = pdMS_TO_TICKS(20 * 60 * 1000);

    while (1)
    {
        // Wait for Wi-Fi connection before proceeding
        wait_for_wifi_connection(portMAX_DELAY);

        enum weather_cmd incoming_cmd;

        if (xQueueReceive(weather_cmd_queue, &incoming_cmd, current_delay))
        {
            weather_cmd_dispatch(incoming_cmd);

            // User requested a fetch, reset the delay to standard interval after processing the command
            if (incoming_cmd == WEATHER_CMD_FETCH)
            {
                current_delay = POLL_INTERVAL_MS;
            }
        }
        else
        {
            esp_err_t err = api_weather_fetch(localisation);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to fetch weather data: %s", esp_err_to_name(err));
            }

            ESP_LOGW(TAG, "Weather Task free stack: %d bytes", uxTaskGetStackHighWaterMark(NULL));
            current_delay = POLL_INTERVAL_MS;
        }
    }
}