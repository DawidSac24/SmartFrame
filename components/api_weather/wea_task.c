#include "api_weather.h"
#include "wea_priv.h"
#include "wifi.h"
#include "esp_log.h"

static const char *TAG = "wea_task";
QueueHandle_t wea_cmd_queue = NULL;

static struct
{
    float lat;
    float lon;
} s_loc;

static void weather_task(void *pvParameters)
{
    TickType_t current_delay = 0;
    const TickType_t POLL_INTERVAL_MS = pdMS_TO_TICKS(20 * 60 * 1000);

    while (1)
    {
        wait_for_wifi_connection(portMAX_DELAY);

        enum wea_cmd incoming_cmd;
        if (xQueueReceive(wea_cmd_queue, &incoming_cmd, current_delay))
        {
            esp_err_t cmd_err = wea_cmd_dispatch(incoming_cmd);
            if (cmd_err != ESP_OK)
                ESP_LOGE(TAG, "Failed to execute cmd: %d", incoming_cmd);

            if (incoming_cmd == WEA_CMD_FETCH)
                current_delay = POLL_INTERVAL_MS; // Reset timer on manual fetch
        }
        else
        {
            // CALL THE MANAGER
            esp_err_t err = wea_manager_fetch_and_save_weather(s_loc.lat, s_loc.lon);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Manager failed to fetch weather data: %s", esp_err_to_name(err));
            }
            current_delay = POLL_INTERVAL_MS;
        }
    }
}

esp_err_t weather_init(float latitude, float longitude, uint32_t stack_size, UBaseType_t priority)
{
    if (wea_manager_init() != ESP_OK)
        return ESP_FAIL;

    s_loc.lat = latitude;
    s_loc.lon = longitude;

    wea_cmd_queue = xQueueCreate(10, sizeof(enum wea_cmd));
    if (!wea_cmd_queue)
        return ESP_ERR_NO_MEM;

    wea_cmd_register();

    if (xTaskCreatePinnedToCore(weather_task, "wea_task", stack_size, NULL, priority, NULL, 0) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create task!");
        return ESP_FAIL;
    }
    return ESP_OK;
}