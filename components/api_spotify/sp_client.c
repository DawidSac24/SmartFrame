#include "sp_client.h"
#include "sp_types.h"
#include "http_client.h"
#include "esp_log.h"
#include <string.h>

#define SPOTIFY_REDIRECT_URI "https%3A%2F%2Fsmartframe.local%2Fcallback"

static const char *TAG = "spotify_client";

esp_err_t sp_client_request_tokens(const char *body, const char *auth_header, char **res);

esp_err_t sp_client_fetch_track(const char *access_token, char **res)
{
    if (access_token == NULL || res == NULL)
        return ESP_ERR_INVALID_ARG;

    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", access_token);

    const char *url = "https://api.spotify.com/v1/me/player/currently-playing";
    int status_code = 0;

    esp_err_t req_err = http_client_request(url, HTTP_CLIENT_GET, auth_header,
                                            "application/json", NULL,
                                            res, &status_code);

    if (req_err != ESP_OK || status_code != 200)
    {
        ESP_LOGE(TAG, "Track request failed! HTTP Status: %d", status_code);
        if (*res != NULL)
        {
            ESP_LOGE(TAG, "Spotify Response: %s", *res);
            free(*res);
            *res = NULL;
        }
        return (req_err == ESP_OK) ? ESP_ERR_INVALID_RESPONSE : req_err;
    }

    return ESP_OK;
}

esp_err_t sp_client_exchange_code(const char *auth_code, const char *auth_header, char **res)
{
    char body[1024];
    snprintf(body, sizeof(body),
             "grant_type=authorization_code"
             "&code=%s"
             "&redirect_uri=%s",
             auth_code, SPOTIFY_REDIRECT_URI);

    ESP_LOGI(TAG, "Full Auth Body length: %d", strlen(body));
    ESP_LOGI(TAG, "Full Auth Body: %s", body);

    esp_err_t req_err = sp_client_request_tokens(body, auth_header, res);
    if (req_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to exchange auth code: %s", esp_err_to_name(req_err));
        return req_err;
    }

    ESP_LOGI(TAG, "Successfully exchanged auth code for tokens!");
    return ESP_OK;
}

esp_err_t sp_client_refresh_token(const char *refresh_token, const char *auth_header, char **res)
{
    char body[HTTP_REQUEST_BODY_BUFF_SIZE];

    snprintf(body, sizeof(body),
             "grant_type=refresh_token"
             "&refresh_token=%s",
             refresh_token);

    esp_err_t req_err = sp_client_request_tokens(body, auth_header, res);
    if (req_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to refresh access token: %s", esp_err_to_name(req_err));
        return req_err;
    }

    ESP_LOGI(TAG, "Successfully refreshed access token!");
    return ESP_OK;
}

esp_err_t sp_client_request_tokens(const char *body, const char *auth_header, char **res)
{
    const char *url = "https://accounts.spotify.com/api/token";
    int status_code = 0;

    esp_err_t req_err = http_client_request(url, HTTP_CLIENT_POST, auth_header,
                                            "application/x-www-form-urlencoded",
                                            body, res, &status_code);

    if (req_err != ESP_OK || status_code != 200)
    {
        ESP_LOGE(TAG, "Token request failed! HTTP Status: %d", status_code);

        if (*res != NULL)
        {
            ESP_LOGE(TAG, "Spotify Response: %s", *res);
            free(*res);
            *res = NULL;
        }

        return (req_err == ESP_OK) ? ESP_ERR_INVALID_RESPONSE : req_err;
    }

    return ESP_OK;
}