#include "api_weather.h"
#include "weather_priv.h"

#include "app_state.h"
#include "wifi.h"

#define TASK_DELAY pdMS_TO_TICKS(1200000) // 20 minutes in milliseconds

static const char *TAG = "weather_api";

static struct localisation s_localisation;

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
    while (1)
    {
        // Wait for Wi-Fi connection before proceeding
        wait_for_wifi_connection(portMAX_DELAY);

        esp_err_t err = api_weather_fetch(localisation);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to fetch weather data: %s", esp_err_to_name(err));
        }

        ESP_LOGW("memory", "Weather Task free stack: %d bytes", uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(TASK_DELAY); // Delay for 20 minutes
    }
}