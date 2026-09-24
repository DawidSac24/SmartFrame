#include "wea_priv.h"
#include "secrets.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "http_client.h"

#define WEATHER_API_URL "https://api.openweathermap.org/data/2.5/weather?lat=%.4f&lon=%.4f&appid=%s&units=metric"
static const char *TAG = "wea_client";

esp_err_t wea_client_fetch_weather(float lat, float lon, char **out_json)
{
    char url_buffer[200];
    snprintf(url_buffer, sizeof(url_buffer), WEATHER_API_URL, lat, lon, WEATHER_API_KEY);

    esp_http_client_config_t config = {
        .url = url_buffer,
        .method = HTTP_METHOD_GET,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 15000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    const int MAX_BUFFER_SIZE = 2048;

    char *json_buffer = malloc(MAX_BUFFER_SIZE + 1);
    if (!json_buffer)
    {
        esp_http_client_cleanup(client);
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err == ESP_OK)
    {
        esp_http_client_fetch_headers(client);
        int total_read_len = esp_http_client_read(client, json_buffer, MAX_BUFFER_SIZE);

        if (total_read_len > 0 && esp_http_client_get_status_code(client) == 200)
        {
            json_buffer[total_read_len] = '\0';
            *out_json = json_buffer; // Pass string pointer back to manager
        }
        else
        {
            ESP_LOGE(TAG, "API Error: Code %d", esp_http_client_get_status_code(client));
            free(json_buffer);
            err = ESP_FAIL;
        }
    }
    else
    {
        free(json_buffer);
    }

    esp_http_client_cleanup(client);
    return err;
}

esp_err_t wea_client_download_img(const char *icon_code, const char *dest_path)
{
    if (!icon_code || !dest_path)
        return ESP_ERR_INVALID_ARG;

    char icon_url[128];
    snprintf(icon_url, sizeof(icon_url), "https://openweathermap.org/img/wn/%s.png", icon_code);

    ESP_LOGI(TAG, "Downloading weather icon from: %s", icon_url);
    return http_client_download_file(icon_url, dest_path);
}