#include "spotify_prv.h"

#include "cJSON.h"
#include <string.h>

esp_err_t spotify_parse_token_response(const char *json,
                                       struct spotify_token_response *out)
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