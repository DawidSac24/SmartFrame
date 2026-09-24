#include "wea_priv.h"
#include "freertos/semphr.h"
#include <string.h>

static struct weather_dto s_weather_data;
static SemaphoreHandle_t s_weather_mutex = NULL;

static bool s_override_active = false;
static struct weather_dto s_override_dto;

esp_err_t wea_state_init(void)
{
    s_weather_mutex = xSemaphoreCreateMutex();
    if (!s_weather_mutex)
        return ESP_ERR_NO_MEM;

    memset(&s_weather_data, 0, sizeof(struct weather_dto));
    s_weather_data.is_valid = false;
    return ESP_OK;
}

esp_err_t weather_get_info(struct weather_dto *out_dto)
{
    if (!s_weather_mutex || !out_dto)
        return ESP_ERR_INVALID_STATE;

    if (xSemaphoreTake(s_weather_mutex, pdMS_TO_TICKS(100)))
    {
        if (s_override_active)
        {
            memcpy(out_dto, &s_override_dto, sizeof(struct weather_dto));
        }
        else
        {
            memcpy(out_dto, &s_weather_data, sizeof(struct weather_dto));
        }
        xSemaphoreGive(s_weather_mutex);
        return ESP_OK;
    }
    return ESP_ERR_TIMEOUT;
}

esp_err_t wea_state_set(const struct weather_dto *new_data)
{
    if (!s_weather_mutex || !new_data)
        return ESP_ERR_INVALID_STATE;

    if (xSemaphoreTake(s_weather_mutex, portMAX_DELAY))
    {
        memcpy(&s_weather_data, new_data, sizeof(struct weather_dto));
        s_weather_data.is_valid = true;
        time(&s_weather_data.last_fetched);
        xSemaphoreGive(s_weather_mutex);
        return ESP_OK;
    }
    return ESP_ERR_TIMEOUT;
}

void weather_set_override(const struct weather_dto *forced_dto)
{
    if (forced_dto)
    {
        memcpy(&s_override_dto, forced_dto, sizeof(struct weather_dto));
        s_override_dto.is_valid = true;
        s_override_active = true;
    }
    else
    {
        s_override_active = false;
    }
}

void weather_set_icon_state(enum wea_icon_state state)
{
    if (xSemaphoreTake(s_weather_mutex, portMAX_DELAY))
    {
        if (s_override_active)
        {
            s_override_dto.icon_state = state;
        }
        else
        {
            s_weather_data.icon_state = state;
        }
        xSemaphoreGive(s_weather_mutex);
    }
}