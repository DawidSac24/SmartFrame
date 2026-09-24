#include "wea_priv.h"
#include "api_weather.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "wea_manager";

esp_err_t wea_manager_init(void)
{
    return wea_state_init();
}

esp_err_t wea_manager_fetch_and_save_weather(float lat, float lon)
{
    char *json_response = NULL;
    esp_err_t fetch_err = wea_client_fetch_weather(lat, lon, &json_response);

    if (fetch_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to fetch weather JSON: %s", esp_err_to_name(fetch_err));
        return fetch_err;
    }

    struct weather_dto new_weather;
    memset(&new_weather, 0, sizeof(new_weather));

    esp_err_t parse_err = wea_parse(json_response, &new_weather);
    if (json_response != NULL)
        free(json_response);

    if (parse_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to parse weather info: %s", esp_err_to_name(parse_err));
        return parse_err;
    }

    // --- ICON DOWNLOAD LOGIC ---
    struct weather_dto last_weather;
    weather_get_info(&last_weather);

    if (strcmp(last_weather.icon, new_weather.icon) != 0 || last_weather.icon_state == WEA_ICON_FAILED)
    {
        ESP_LOGD(TAG, "Downloading new weather icon...");
        new_weather.icon_state = WEA_ICON_DOWNLOADING;
        wea_state_set(&new_weather); // Update state to downloading

        // Delegate to client
        esp_err_t dl_err = wea_client_download_img(new_weather.icon, "/fs/weather.png");

        if (dl_err == ESP_OK)
        {
            ESP_LOGD(TAG, "Weather icon saved to LittleFS!");
            new_weather.icon_state = WEA_ICON_NEW_FILE;
        }
        else
        {
            ESP_LOGE(TAG, "Failed to save weather icon!");
            new_weather.icon_state = WEA_ICON_FAILED;
        }
    }
    else
    {
        new_weather.icon_state = last_weather.icon_state;
    }

    wea_state_set(&new_weather);
    return ESP_OK;
}