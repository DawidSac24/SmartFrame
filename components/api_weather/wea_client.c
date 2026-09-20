#include "wea_priv.h"

#include "secrets.h"

#include "esp_http_client.h"
#include "esp_crt_bundle.h"

#define WEATHER_API_URL "https://api.openweathermap.org/data/2.5/weather?lat=%.4f&lon=%.4f&appid=%s"

static const char *TAG = "weather_api";

esp_err_t wea_fetch(struct localisation *localisation)
{
    char url_buffer[200];

    snprintf(url_buffer, sizeof(url_buffer),
             WEATHER_API_URL,
             localisation->latitude, localisation->longitude, WEATHER_API_KEY);

    esp_http_client_config_t config = {
        .url = url_buffer,
        .method = HTTP_METHOD_GET,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 15000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    const int MAX_BUFFER_SIZE = 2048;
    char *json_buffer = malloc(MAX_BUFFER_SIZE + 1);
    if (json_buffer == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for JSON!");
        esp_http_client_cleanup(client);
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));

        free(json_buffer);
        esp_http_client_cleanup(client);
        return err;
    }

    esp_http_client_fetch_headers(client);

    int total_read_len = 0;

    while (1)
    {
        int read_len = esp_http_client_read(client,
                                            json_buffer + total_read_len,
                                            MAX_BUFFER_SIZE - total_read_len);

        if (read_len <= 0)
        {
            break;
        }

        total_read_len += read_len;
    }

    json_buffer[total_read_len] = '\0';

    json_buffer[total_read_len] = '\0';

    // Check if the API actually gave us a 200 OK
    int status_code = esp_http_client_get_status_code(client);
    if (status_code != 200)
    {
        ESP_LOGE(TAG, "API Rejected Request! HTTP Status: %d", status_code);
        ESP_LOGE(TAG, "API Response: %s", json_buffer); // This will tell us what's wrong!
        err = ESP_FAIL;
    }
    else
    {
        ESP_LOGI(TAG, "Successfully downloaded %d bytes.", total_read_len);
        err = wea_parse(json_buffer);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to parse weather data: %s", esp_err_to_name(err));
        }
        else
        {
            ESP_LOGI(TAG, "Weather data fetched and parsed successfully.");
        }
    }

    free(json_buffer);
    esp_http_client_cleanup(client);
    return err;

    ESP_LOGD(TAG, "Successfully downloaded %d bytes.", total_read_len);
    ESP_LOGD(TAG, "JSON Response: %s", json_buffer);

    err = wea_parse(json_buffer);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to parse weather data: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "Weather data fetched and parsed successfully.");
    }

    free(json_buffer);
    esp_http_client_cleanup(client);
    return err;
}
