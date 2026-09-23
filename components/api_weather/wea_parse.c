#include "wea_priv.h"
#include "cJSON.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "wea_parse";

esp_err_t wea_parse(const char *json_str, struct weather_dto *out_dto)
{
    if (!json_str || !out_dto)
        return ESP_ERR_INVALID_ARG;

    cJSON *root = cJSON_Parse(json_str);
    if (!root)
        return ESP_ERR_NOT_FOUND;

    cJSON *weather_array = cJSON_GetObjectItem(root, "weather");
    cJSON *weather_item = cJSON_GetArrayItem(weather_array, 0);
    cJSON *data_item = cJSON_GetObjectItem(root, "main");

    if (!cJSON_IsObject(weather_item) || !cJSON_IsObject(data_item))
    {
        cJSON_Delete(root);
        return ESP_ERR_NOT_FOUND;
    }

    // Populate the DTO
    out_dto->id = cJSON_GetObjectItem(weather_item, "id")->valueint;
    out_dto->temperature = cJSON_GetObjectItem(data_item, "temp")->valuedouble;
    out_dto->humidity = cJSON_GetObjectItem(data_item, "humidity")->valuedouble;
    out_dto->pressure = cJSON_GetObjectItem(data_item, "pressure")->valuedouble;

    strncpy(out_dto->icon, cJSON_GetObjectItem(weather_item, "icon")->valuestring, sizeof(out_dto->icon) - 1);
    strncpy(out_dto->main, cJSON_GetObjectItem(weather_item, "main")->valuestring, sizeof(out_dto->main) - 1);
    strncpy(out_dto->description, cJSON_GetObjectItem(weather_item, "description")->valuestring, sizeof(out_dto->description) - 1);

    cJSON_Delete(root);
    return ESP_OK;
}