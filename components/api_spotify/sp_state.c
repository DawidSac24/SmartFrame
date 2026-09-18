#include "sp_state.h"
#include "api_spotify.h"

#include <string.h>

static struct spotify_track_dto current_track;
static SemaphoreHandle_t state_mutex = NULL;

void sp_state_init(void)
{
    state_mutex = xSemaphoreCreateMutex();
    memset(&current_track, 0, sizeof(current_track));
}

// private setter
esp_err_t sp_state_set_track_info(const struct spotify_track_dto *track_info)
{
    if (state_mutex == NULL || track_info == NULL)
        return ESP_ERR_INVALID_STATE;

    if (xSemaphoreTake(state_mutex, portMAX_DELAY))
    {
        current_track = *track_info;
        xSemaphoreGive(state_mutex);
        return ESP_OK;
    }
    return ESP_ERR_TIMEOUT;
}

// public getter
esp_err_t sp_get_track_info(struct spotify_track_dto *out_track)
{
    if (state_mutex == NULL || out_track == NULL)
        return ESP_ERR_INVALID_STATE;

    if (xSemaphoreTake(state_mutex, portMAX_DELAY))
    {
        *out_track = current_track;
        xSemaphoreGive(state_mutex);
        return ESP_OK;
    }
    return ESP_ERR_TIMEOUT;
}