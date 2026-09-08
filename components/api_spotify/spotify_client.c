#include "spotify_prv.h"

#include "http_client.h"

#include "esp_log.h"
#include <string.h>

#define SPOTIFY_REDIRECT_URI "https%3A%2F%2Fsmartframe.local%2Fcallback"

static const char *TAG = "spotify_client";

esp_err_t spotify_client_request_tokens(const char *body, const char *auth_header,
                                        struct spotify_token_response *out_response);

esp_err_t spotify_client_exchange_code(const char *auth_code, const char *auth_header,
                                       struct spotify_token_response *out_response)
{
    char body[HTTP_REQUEST_BODY_BUFF_SIZE];
    snprintf(body, sizeof(body),
             "grant_type=authorization_code"
             "&code=%s"
             "&redirect_uri=%s",
             auth_code, SPOTIFY_REDIRECT_URI);

    esp_err_t res = spotify_client_request_tokens(body, auth_header, out_response);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to exchange authentification code: %s", esp_err_to_name(res));
        return res;
    }

    ESP_LOGI(TAG, "Successfully exchanged auth code for tokens!");
    return ESP_OK;
}

esp_err_t spotify_client_refresh_token(const char *refresh_token, const char *auth_header,
                                       struct spotify_token_response *out_response)
{
    char body[HTTP_REQUEST_BODY_BUFF_SIZE];
    snprintf(body, sizeof(body),
             "grant_type=refresh_token"
             "&code=%s",
             refresh_token);

    esp_err_t res = spotify_client_request_tokens(body, auth_header, out_response);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to refresh access token: %s", esp_err_to_name(res));
        return res;
    }

    ESP_LOGI(TAG, "Successfully refreshed access token!");
    return ESP_OK;
}

esp_err_t spotify_client_request_tokens(const char *body, const char *auth_header,
                                        struct spotify_token_response *out_response)
{
    const char *url = "https://accounts.spotify.com/api/token";

    char **http_response = (char **)malloc(sizeof(char *));
    int *status_code = (int *)malloc(sizeof(int));
    esp_err_t http_result = http_client_request(url, HTTP_CLIENT_GET, auth_header,
                                                "application/x-www-form-urlencoded",
                                                body, http_response, status_code);

    if (http_result != ESP_OK || *status_code != 200)
    {
        free(http_response);
        ESP_LOGE(TAG, "Token request failed! HTTP Status: %d", status_code);
        ESP_LOGE(TAG, "Spotify Response: %s", &http_response);
        if (http_result == ESP_OK)
            return ESP_ERR_INVALID_RESPONSE;
        return http_result;
    }

    esp_err_t parse_result = spotify_parse_token_response(*http_response, out_response);
    if (parse_result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to parse JSON response: %s", esp_err_to_name(parse_result));
        free(http_response);
        return parse_result;
    }
    return ESP_OK;
}