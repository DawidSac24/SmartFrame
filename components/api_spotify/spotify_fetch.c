#include "spotify_prv.h"

#include "nvs_hal.h"
#include "app_state.h"

const char *TAG = "spotify";

esp_err_t api_spotify_get_refresh_token()
{
    char token_buff[TOKEN_BUFFER_SIZE];
    esp_err_t err = nvs_hal_read_str(REG_SPOTIFY_REFRESH_TOKEN, &token_buff, TOKEN_BUFFER_SIZE);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Spotify refresh token fetched successfully");
    }
    ESP_LOGE(TAG, "%s: Failed reading the spotify refresh token, waiting for the spotify authentification", esp_err_to_name(err));
}

esp_err_t api_spotify_fetch_access_token()
{
}