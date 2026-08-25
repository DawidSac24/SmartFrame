#include "weather_priv.h"

#include "app_state.h"

#include "cJSON.h"
#include <time.h>

static const char *TAG = "weather_api";

esp_err_t api_weather_parse(const char *json_str)
{
    if (json_str == NULL)
        return ESP_ERR_INVALID_ARG;

    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL)
        return ESP_ERR_NOT_FOUND;

    cJSON *weather_array = cJSON_GetObjectItem(root, "weather");
    if (!cJSON_IsArray(weather_array))
        return ESP_ERR_NOT_FOUND;

    cJSON *weather_item = cJSON_GetArrayItem(weather_array, 0);
    cJSON *data_item = cJSON_GetObjectItem(root, "main");
    if (!cJSON_IsObject(data_item))
        return ESP_ERR_NOT_FOUND;

    cJSON *id = cJSON_GetObjectItem(weather_item, "id");
    cJSON *icon = cJSON_GetObjectItem(weather_item, "icon");

    if (!cJSON_IsNumber(id) || !cJSON_IsString(icon))
        return ESP_ERR_NOT_FOUND;
    ESP_LOGD(TAG, "Weather ID: %d", id->valueint);
    ESP_LOGD(TAG, "Icon Code: %s", icon->valuestring);

    cJSON *coord = cJSON_GetObjectItem(root, "coord");
    if (cJSON_IsObject(coord))
    {
        ESP_LOGD(TAG, "Coordinates: lat=%.4f, lon=%.4f",
                 cJSON_GetObjectItem(coord, "lat")->valuedouble,
                 cJSON_GetObjectItem(coord, "lon")->valuedouble);
    }

    if (xSemaphoreTake(global_state_mutex, portMAX_DELAY))
    {
        global_state.weather.id = id->valueint;
        global_state.weather.temperature = cJSON_GetObjectItem(data_item, "temp")->valuedouble;
        global_state.weather.humidity = cJSON_GetObjectItem(data_item, "humidity")->valuedouble;
        global_state.weather.pressure = cJSON_GetObjectItem(data_item, "pressure")->valuedouble;

        strncpy(global_state.weather.icon, icon->valuestring, sizeof(global_state.weather.icon) - 1);
        global_state.weather.icon[sizeof(global_state.weather.icon) - 1] = '\0';

        strncpy(global_state.weather.main, cJSON_GetObjectItem(weather_item, "main")->valuestring, sizeof(global_state.weather.main) - 1);
        global_state.weather.main[sizeof(global_state.weather.main) - 1] = '\0';

        strncpy(global_state.weather.description, cJSON_GetObjectItem(weather_item, "description")->valuestring, sizeof(global_state.weather.description) - 1);
        global_state.weather.description[sizeof(global_state.weather.description) - 1] = '\0';

        time(&global_state.weather.last_fetched);

        global_state.weather.is_dirty = true;

        xSemaphoreGive(global_state_mutex);
    }

    cJSON_Delete(root);
    return ESP_OK;
}