#include "sp_parse.h"

#include "cJSON.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "spotify_parser";

esp_err_t sp_parse_track(const char *json_string, struct sp_track_info *out_info)
{
    if (json_string == NULL || out_info == NULL)
        return ESP_ERR_INVALID_ARG;

    // 1. Parse the raw string into a cJSON object
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL)
    {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_FAIL;
    }

    // 2. Check if music is actually playing
    cJSON *is_playing = cJSON_GetObjectItem(root, "is_playing");
    if (cJSON_IsTrue(is_playing))
    {
        out_info->is_playing = true;
    }
    else
    {
        out_info->is_playing = false;
        // If paused, you might still want the track info, so we continue
    }

    // 3. Get the main "item" object (the track itself)
    cJSON *item = cJSON_GetObjectItem(root, "item");
    if (item == NULL || cJSON_IsNull(item))
    {
        ESP_LOGW(TAG, "No track currently playing (or podcast playing)");
        cJSON_Delete(root); // ALWAYS delete root before returning!
        return ESP_ERR_NOT_FOUND;
    }

    // 4. Extract Track Name
    cJSON *track_name = cJSON_GetObjectItem(item, "name");
    if (cJSON_IsString(track_name) && track_name->valuestring != NULL)
    {
        strncpy(out_info->track_name, track_name->valuestring, sizeof(out_info->track_name) - 1);
        out_info->track_name[sizeof(out_info->track_name) - 1] = '\0';
    }

    // 5. Extract First Artist Name
    cJSON *artists = cJSON_GetObjectItem(item, "artists");
    if (cJSON_IsArray(artists))
    {
        cJSON *first_artist = cJSON_GetArrayItem(artists, 0); // Get index 0
        if (first_artist != NULL)
        {
            cJSON *artist_name = cJSON_GetObjectItem(first_artist, "name");
            if (cJSON_IsString(artist_name) && artist_name->valuestring != NULL)
            {
                strncpy(out_info->artist_name, artist_name->valuestring, sizeof(out_info->artist_name) - 1);
                out_info->artist_name[sizeof(out_info->artist_name) - 1] = '\0';
            }
        }
    }

    // 6. Extract the 64x64 Image URL
    cJSON *album = cJSON_GetObjectItem(item, "album");
    if (album != NULL)
    {
        cJSON *images = cJSON_GetObjectItem(album, "images");
        if (cJSON_IsArray(images))
        {
            // Spotify orders images: [0] Large, [1] Medium, [2] Small (64x64)
            // Getting the last item in the array ensures we get the smallest available image
            int num_images = cJSON_GetArraySize(images);
            if (num_images > 0)
            {
                cJSON *smallest_image = cJSON_GetArrayItem(images, num_images - 1);
                cJSON *url = cJSON_GetObjectItem(smallest_image, "url");
                if (cJSON_IsString(url) && url->valuestring != NULL)
                {
                    strncpy(out_info->image_url, url->valuestring, sizeof(out_info->image_url) - 1);
                    out_info->image_url[sizeof(out_info->image_url) - 1] = '\0';
                }
            }
        }
    }

    cJSON_Delete(root);

    ESP_LOGI(TAG, "Parsed: '%s' by '%s'", out_info->track_name, out_info->artist_name);
    return ESP_OK;
}

esp_err_t sp_parse_token_response(const char *json,
                                  struct sp_token_response *out)
{
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    cJSON *access_token_json = cJSON_GetObjectItem(root, "access_token");
    cJSON *expires_in_json = cJSON_GetObjectItem(root, "expires_in");
    cJSON *refresh_token_json = cJSON_GetObjectItem(root, "refresh_token");

    if (!cJSON_IsString(access_token_json) || !cJSON_IsNumber(expires_in_json))
    {
        cJSON_Delete(root);
        return ESP_FAIL;
    }
    strncpy(out->access_token, access_token_json->valuestring, sizeof(out->access_token) - 1);
    out->access_token[sizeof(out->access_token) - 1] = '\0';

    out->expires_in_sec = expires_in_json->valueint;

    // copy refresh token, may be NULL
    if (cJSON_IsString(refresh_token_json))
    {
        strncpy(out->refresh_token, refresh_token_json->valuestring, sizeof(out->refresh_token) - 1);
        out->refresh_token[sizeof(out->refresh_token) - 1] = '\0';
        out->has_new_refresh_token = true;
    }
    else
    {
        out->has_new_refresh_token = false;
    }

    cJSON_Delete(root);
    return ESP_OK;
}