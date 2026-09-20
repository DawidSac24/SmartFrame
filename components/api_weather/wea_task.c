#include "api_weather.h"
#include "wea_priv.h"

#include "app_state.h"
#include "wifi.h"

static const char *TAG = "weather_api";

static struct localisation s_localisation;

QueueHandle_t wea_cmd_queue = NULL;

void weather_task(void *pvParameters);

TaskHandle_t weather_init(float latitude, float longitude)
{
    s_localisation.latitude = latitude;
    s_localisation.longitude = longitude;

    wea_cmd_register();

    TaskHandle_t task_handle = NULL;
    BaseType_t returned = xTaskCreatePinnedToCore(weather_task, "weather_task", 8192, &s_localisation, PRIO_WEATHER_API, &task_handle, 0);
    if (returned != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create task!");
        return NULL;
    }

    return task_handle;
}

void weather_task(void *pvParameters)
{
    struct localisation *localisation = (struct localisation *)pvParameters;

    TickType_t current_delay = 0;
    const TickType_t POLL_INTERVAL_MS = pdMS_TO_TICKS(20 * 60 * 1000);

    while (1)
    {
        // Wait for Wi-Fi connection before proceeding
        wait_for_wifi_connection(portMAX_DELAY);

        enum wea_cmd incoming_cmd;

        if (xQueueReceive(weather_cmd_queue, &incoming_cmd, current_delay))
        {
            esp_err_t cmd_err = wea_cmd_dispatch(incoming_cmd);
            if (cmd_err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to execute cmd: %d, err: %s", incoming_cmd, cmd_err);
            }
            else
            {
                // User requested a fetch, reset the delay to standard interval after processing the command
                if (incoming_cmd == WEA_CMD_FETCH)
                {
                    current_delay = POLL_INTERVAL_MS;
                }
            }
        }
        else
        {
            esp_err_t err = wea_fetch(localisation);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to fetch weather data: %s", esp_err_to_name(err));
            }

            ESP_LOGW(TAG, "Weather Task free stack: %d bytes", uxTaskGetStackHighWaterMark(NULL));
            current_delay = POLL_INTERVAL_MS;
        }
    }
}