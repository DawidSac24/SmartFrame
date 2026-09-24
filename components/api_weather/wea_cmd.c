#include "api_weather.h"
#include "wea_priv.h"

#include "esp_console.h"
#include "esp_log.h"
#include "secrets.h"

#include <string.h>
#include <time.h>

static const char *TAG = "weather_cmd";

// 1. Added "test" to the command list string array
static const char *wea_cmd_strings[] = {
    "fetch",
    "print",
    "test"};

// Forward declarations
static esp_err_t cmd_weather(int argc, char **argv);
esp_err_t wea_cmd_fetch(void);
esp_err_t wea_cmd_print(void);
esp_err_t wea_cmd_test_mode(const char *condition);
enum wea_cmd wea_str_to_cmd(const char *str);

void wea_cmd_register(void)
{
    esp_console_cmd_t cmd = {
        .command = "weather",
        .help = "Manage the weather API (Args: print, fetch, test <type>)",
        .hint = "<print|fetch|test> [sun|night|clouds|rain|storm|snow]",
        .func = &cmd_weather,
    };
    esp_console_cmd_register(&cmd);
}

void wea_cmd_send(enum wea_cmd cmd)
{
    if (wea_cmd_queue != NULL)
    {
        xQueueSend(wea_cmd_queue, &cmd, 0);
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
    // Note: WEA_CMD_TEST is handled directly in cmd_weather to pass the argument string safely!
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

    // Special handling for test command which requires a secondary sub-argument (e.g., "weather test sun")
    if (cmd == WEA_CMD_TEST)
    {
        if (argc < 3)
        {
            ESP_LOGE(TAG, "Missing test type! Use: weather test <sun|night|clouds|rain|storm|snow>");
            return ESP_ERR_INVALID_ARG;
        }
        return wea_cmd_test_mode(argv[2]);
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

esp_err_t wea_cmd_fetch(void)
{
    ESP_LOGI(TAG, "Forcing a manual weather fetch...");

    esp_err_t err = wea_manager_fetch_and_save_weather(LATITUDE, LONGITUDE);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error fetching weather data: %s", esp_err_to_name(err));
    }
    return err;
}
esp_err_t wea_cmd_test_mode(const char *condition)
{
    struct weather_dto test_dto = {0};
    test_dto.temperature = 21.5f;
    test_dto.humidity = 65.0f;
    test_dto.pressure = 1013.0f;
    test_dto.is_valid = true;

    if (strcmp(condition, "sun") == 0)
    {
        test_dto.id = 800;
        strcpy(test_dto.icon, "01d");
        strcpy(test_dto.main, "Clear");
    }
    else if (strcmp(condition, "night") == 0)
    {
        test_dto.id = 800;
        strcpy(test_dto.icon, "01n");
        strcpy(test_dto.main, "Clear Night");
    }
    else if (strcmp(condition, "clouds") == 0)
    {
        test_dto.id = 804;
        strcpy(test_dto.icon, "04d");
        strcpy(test_dto.main, "Clouds");
    }
    else if (strcmp(condition, "rain") == 0)
    {
        test_dto.id = 500;
        strcpy(test_dto.icon, "10d");
        strcpy(test_dto.main, "Rain");
    }
    else if (strcmp(condition, "storm") == 0)
    {
        test_dto.id = 200;
        strcpy(test_dto.icon, "11d");
        strcpy(test_dto.main, "Thunderstorm");
    }
    else if (strcmp(condition, "snow") == 0)
    {
        test_dto.id = 600;
        strcpy(test_dto.icon, "13d");
        strcpy(test_dto.main, "Snow");
    }
    else
    {
        printf("Unknown test condition: '%s'\n", condition);
        return ESP_ERR_INVALID_ARG;
    }

    printf("Setting override to %s. Downloading icon '%s.png'...\n", condition, test_dto.icon);

    // Set state to DOWNLOADING so UI waits
    test_dto.icon_state = WEA_ICON_DOWNLOADING;
    weather_set_override(&test_dto);

    // Download the test image to LittleFS
    esp_err_t dl_err = wea_client_download_img(test_dto.icon, "/fs/weather.png");

    if (dl_err == ESP_OK)
    {
        test_dto.icon_state = WEA_ICON_NEW_FILE;
        printf("Test icon downloaded successfully! UI will decode it now.\n");
    }
    else
    {
        test_dto.icon_state = WEA_ICON_FAILED;
        printf("Failed to download test icon.\n");
    }

    // Update the override state with the final file status
    weather_set_override(&test_dto);
    return ESP_OK;
}

esp_err_t wea_cmd_print(void)
{
    struct weather_dto dto;
    esp_err_t err = weather_get_info(&dto);

    if (err != ESP_OK)
    {
        printf("Error: Could not retrieve weather state (Code: %s)\n", esp_err_to_name(err));
        return err;
    }

    printf("\n--- CURRENT WEATHER STATE ---\n");

    if (!dto.is_valid)
    {
        printf("Status: No valid weather data available yet.\n");
    }
    else
    {
        printf("ID: %d\n", dto.id);
        printf("Temperature: %.2f\n", dto.temperature);
        printf("Humidity: %.2f\n", dto.humidity);
        printf("Pressure: %.2f\n", dto.pressure);
        printf("Icon: %s\n", dto.icon);
        printf("Main: %s\n", dto.main);
        printf("Description: %s\n", dto.description);

        if (dto.last_fetched == 0)
        {
            printf("Last Fetched: Never (Simulation or Waiting for download...)\n");
        }
        else
        {
            struct tm timeinfo;
            char time_string[64];

            localtime_r(&dto.last_fetched, &timeinfo);
            strftime(time_string, sizeof(time_string), "%c", &timeinfo);
            printf("Last Fetched: %s\n", time_string);
        }
    }

    printf("-----------------------------\n\n");
    return ESP_OK;
}