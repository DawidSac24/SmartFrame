#include "weather_priv.h"

#include "app_state.h"
#include "esp_console.h"
#include "secrets.h"

#include <string.h>
#include <time.h>

static const char *TAG = "weather_cmd";

esp_err_t weather_cmd_fetch(int argc, char **argv);
esp_err_t weather_cmd_print(int argc, char **argv);

static esp_err_t cmd_weather(int argc, char **argv)
{
    if (argc < 2)
    {
        ESP_LOGE(TAG, "No argument provided. Use 'print' or 'fetch'.");
        return ESP_ERR_INVALID_ARG;
    }

    if (strcmp(argv[1], "print") == 0)
    {
        return weather_cmd_print(0, NULL);
    }
    else if (strcmp(argv[1], "fetch") == 0)
    {
        return weather_cmd_fetch(argc, argv);
    }
    else
    {
        printf("Error: Unknown argument '%s'\n", argv[1]);
    }

    return 0;
}

// Update the registration function to just "weather"
static void register_weather_commands(void)
{
    esp_console_cmd_t cmd = {
        .command = "weather",
        .help = "Manage the weather API (Args: print, fetch)",
        .hint = "<print|fetch>",
        .func = &cmd_weather,
    };
    esp_console_cmd_register(&cmd);
}

void weather_cmd_register(void)
{
    esp_console_cmd_t fetch_cmd = {
        .command = "weather_fetch",
        .help = "Fetches the current weather from the API and updates the global state",
        .hint = NULL,
        .func = &weather_cmd_fetch,
    };
    esp_console_cmd_register(&fetch_cmd);

    esp_console_cmd_t print_cmd = {
        .command = "weather_print",
        .help = "Prints the current weather from the global state",
        .hint = NULL,
        .func = &weather_cmd_print,
    };
    esp_console_cmd_register(&print_cmd);
}

esp_err_t weather_cmd_fetch(int argc, char **argv)
{
    ESP_LOGI(TAG, "Forcing a manual weather fetch...");

    struct localisation loc;
    if (argc >= 4)
    {
        loc.latitude = atof(argv[2]);
        loc.longitude = atof(argv[3]);
    }
    else
    {
        loc.latitude = LATITUDE;
        loc.longitude = LONGITUDE;
    }

    esp_err_t err = api_weather_fetch(&loc);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error fetching weather data: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t weather_cmd_print(int argc, char **argv)
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