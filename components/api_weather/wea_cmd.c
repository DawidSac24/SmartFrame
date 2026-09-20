#include "api_weather.h"
#include "wea_priv.h"

#include "app_state.h"
#include "esp_console.h"
#include "secrets.h"

#include <string.h>
#include <time.h>

static const char *TAG = "weather_cmd";

static const char *wea_cmd_strings[] = {
    "fetch",
    "print"};

static esp_err_t cmd_weather(int argc, char **argv);
esp_err_t wea_cmd_fetch(void);
esp_err_t wea_cmd_print(void);

void wea_cmd_register(void)
{
    esp_console_cmd_t cmd = {
        .command = "weather",
        .help = "Manage the weather API (Args: print, fetch)",
        .hint = "<print|fetch>",
        .func = &cmd_weather,
    };
    esp_console_cmd_register(&cmd);

    weather_cmd_queue = xQueueCreate(WEA_CMD_UNKNOWN, sizeof(enum wea_cmd));
}

void wea_cmd_send(enum wea_cmd cmd)
{
    if (weather_cmd_queue != NULL)
    {
        xQueueSend(weather_cmd_queue, &cmd, 0);
    }
}

esp_err_t wea_cmd_dispatch(enum wea_cmd cmd)
{
    switch (cmd)
    {
    case WEA_CMD_FETCH:
        return wea_cmd_fetch();
    case WEA_CMD_PRINT:
        return wea_cmd_print();
    default:
        ESP_LOGE(TAG, "Unknown command received: %d", cmd);
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_FAIL;
}

esp_err_t cmd_weather(int argc, char **argv)
{
    if (argc < 2)
    {
        ESP_LOGE(TAG, "No argument provided. Use:");
        for (int i = 0; i < WEA_CMD_UNKNOWN; i++)
        {
            ESP_LOGE(TAG, "weather %s", wea_cmd_strings[i]);
        }
        return ESP_ERR_INVALID_ARG;
    }

    enum wea_cmd cmd = wea_str_to_cmd(argv[1]);

    if (cmd == WEA_CMD_UNKNOWN)
    {
        ESP_LOGE(TAG, "Unknown argument '%s'", argv[1]);
        return ESP_ERR_INVALID_ARG;
    }

    wea_cmd_send(cmd);

    ESP_LOGI(TAG, "Command '%s' dispatched to task.", argv[1]);
    return ESP_OK;
}

enum wea_cmd wea_str_to_cmd(const char *str)
{
    int num_cmds = sizeof(wea_cmd_strings) / sizeof(wea_cmd_strings[0]);
    for (int i = 0; i < num_cmds; i++)
    {
        if (strcmp(str, wea_cmd_strings[i]) == 0)
        {
            return (enum wea_cmd)i;
        }
    }
    return WEA_CMD_UNKNOWN;
}

esp_err_t wea_cmd_fetch()
{
    ESP_LOGI(TAG, "Forcing a manual weather fetch...");

    struct localisation loc;
    loc.latitude = LATITUDE;
    loc.longitude = LONGITUDE;

    esp_err_t err = wea_fetch(&loc);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error fetching weather data: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t wea_cmd_print()
{
    if (xSemaphoreTake(global_state_mutex, pdMS_TO_TICKS(100)))
    {
        printf("\n--- CURRENT WEATHER STATE ---\n");
        printf("ID: %d\n", global_state.weather.id);
        printf("Temperature: %.2f\n", global_state.weather.temperature);
        printf("Humidity: %.2f\n", global_state.weather.humidity);
        printf("Pressure: %.2f\n", global_state.weather.pressure);
        printf("Icon: %s\n", global_state.weather.icon);
        printf("Main: %s\n", global_state.weather.main);
        printf("Description: %s\n", global_state.weather.description);

        if (global_state.weather.last_fetched == 0)
        {
            printf("Last Fetched: Never (Waiting for first download...)\n");
        }
        else
        {
            struct tm timeinfo;
            char time_string[64];

            localtime_r(&global_state.weather.last_fetched, &timeinfo);
            strftime(time_string, sizeof(time_string), "%c", &timeinfo);
            printf("Last Fetched: %s\n", time_string);
        }

        printf("-----------------------------\n\n");
        xSemaphoreGive(global_state_mutex);
    }
    else
    {
        printf("Error: Could not lock Mutex!\n");
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}