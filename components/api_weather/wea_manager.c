#include "wea_priv.h"
#include "wifi.h"
#include "esp_log.h"

static const char *TAG = "wea_manager";
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
            wea_cmd_dispatch(incoming_cmd);
            if (incoming_cmd == WEA_CMD_FETCH)
            {
                current_delay = POLL_INTERVAL_MS; // Reset timer on manual fetch
            }
        }
        else
        {
            wea_fetch(s_loc.lat, s_loc.lon);
            current_delay = POLL_INTERVAL_MS;
        }
    }
}

esp_err_t weather_init(float latitude, float longitude)
{
    if (wea_state_init() != ESP_OK)
        return ESP_FAIL;

    s_loc.lat = latitude;
    s_loc.lon = longitude;

    wea_cmd_queue = xQueueCreate(10, sizeof(enum wea_cmd));
    wea_cmd_register();

    if (xTaskCreatePinnedToCore(weather_task, "wea_task", 6144, NULL, 3, NULL, 0) != pdPASS)
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}