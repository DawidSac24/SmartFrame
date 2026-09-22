#include "sp_manager.h"

#include "sp_state.h"
#include "sp_types.h"
#include "sp_auth.h"
#include "sp_client.h"
#include "sp_parse.h"
#include "sp_storage.h"

#include "http_client.h"
#include "gfx.h"

#include "esp_log.h"

static const char *TAG = "spotify_manager";

static uint8_t g_album_rgb_buffer[64 * 64 * 3];

void sp_manager_init(void)
{
    sp_auth_init();
    sp_state_init();
}

esp_err_t sp_manager_fetch_and_save_track(void)
{
    esp_err_t auth_err = sp_manager_ensure_authentificated();
    if (auth_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed the spotify authentification: %s", esp_err_to_name(auth_err));
        return auth_err;
    }

    char access_token[256];
    esp_err_t token_err = sp_auth_get_access_token(access_token, sizeof(access_token));
    if (token_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get access token: %s", esp_err_to_name(token_err));
        return token_err;
    }

    char *json_response = NULL;
    esp_err_t fetch_err = sp_client_fetch_track(access_token, &json_response);
    if (fetch_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to fetch track info: %s", esp_err_to_name(fetch_err));
        return fetch_err;
    }

    struct spotify_track_dto new_track;
    esp_err_t parse_err = sp_parse_track(json_response, &new_track);
    if (parse_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to parse track info: %s", esp_err_to_name(parse_err));
        return parse_err;
    }
    ESP_LOGI(TAG, "Track Fetched and parsed correctly!");
    ESP_LOGI(TAG, "Currently playing: '%s' by '%s'", new_track.track_name, new_track.artist_name);

    struct spotify_track_dto last_track;
    spotify_get_track_info(&last_track);

    if (strcmp(last_track.cover_url, new_track.cover_url) != 0 || last_track.cover_state == SP_COVER_FAILED)
    {
        ESP_LOGI(TAG, "Downloading album art...");
        new_track.cover_state = SP_COVER_DOWNLOADING;
        sp_state_set_track_info(&new_track);

        esp_err_t dl_err = http_client_download_file(new_track.cover_url, "/fs/album.jpg");

        if (dl_err == ESP_OK)
        {
            ESP_LOGI(TAG, "Album art saved to flash!");
            new_track.cover_state = SP_COVER_NEW_FILE;
        }
        else
        {
            new_track.cover_state = SP_COVER_FAILED;
        }
    }
    else
    {
        new_track.cover_state = last_track.cover_state;
    }

    sp_state_set_track_info(&new_track);
    return ESP_OK;
}

esp_err_t sp_manager_ensure_authentificated(void)
{
    char access_token[DEFAULT_BUFF_SIZE];

    if (sp_auth_get_access_token(access_token, sizeof(access_token)) == ESP_OK)
    {
        return ESP_OK;
    }

    esp_err_t req_err;
    char *json_response = NULL;

    char auth_header[DEFAULT_BUFF_SIZE];
    sp_auth_get_header(auth_header, sizeof(auth_header));

    char refresh_token[DEFAULT_BUFF_SIZE];
    if (sp_auth_get_refresh_token(refresh_token, sizeof(refresh_token)) == ESP_OK)
    {
        ESP_LOGI(TAG, "Access token expired. Using refresh token...");
        req_err = sp_client_refresh_token(refresh_token, auth_header, &json_response);
    }
    else
    {
        ESP_LOGW(TAG, "No refresh token found. Waiting for Auth Code...");
        char auth_code[DEFAULT_AUTH_CODE_BUFF_SIZE];

        req_err = sp_auth_get_auth_code(auth_code);
        if (req_err != ESP_OK)
            return req_err;

        ESP_LOGI(TAG, "Got Auth Code! Waiting 2s for Web Server to free RAM...");
        vTaskDelay(pdMS_TO_TICKS(2000));

        req_err = sp_client_exchange_code(auth_code, auth_header, &json_response);
    }

    if (req_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed HTTP token request: %s", esp_err_to_name(req_err));
        return req_err;
    }

    struct sp_token_response token_res;
    req_err = sp_parse_token_response(json_response, &token_res);
    free(json_response);

    if (req_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to parse token JSON: %s", esp_err_to_name(req_err));
        return req_err;
    }

    sp_auth_set_access_token(token_res.access_token, token_res.expires_in_sec);

    if (token_res.has_new_refresh_token)
    {
        sp_auth_set_refresh_token(token_res.refresh_token);
        sp_storage_set_refresh_token(token_res.refresh_token);
    }

    return ESP_OK;
}

esp_err_t sp_manager_print_track(void)
{
    struct spotify_track_dto track;
    esp_err_t track_err = spotify_get_track_info(&track);
    if (track_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get track info: %s", esp_err_to_name(track_err));
        return track_err;
    }
    ESP_LOGI(TAG, "TRACK INFO:");
    ESP_LOGI(TAG, "track name: %s", track.track_name);
    ESP_LOGI(TAG, "artist name: %s", track.artist_name);
    ESP_LOGI(TAG, "is playing: %d", track.is_playing);
    ESP_LOGI(TAG, "cover state: %d", sp_cover_state_to_str(track.cover_state));
    return ESP_OK;
}