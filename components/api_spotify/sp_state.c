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

// public getter
esp_err_t spotify_get_track_info(struct spotify_track_dto *out_track)
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

const char *sp_api_state_to_str(enum sp_api_state state)
{
    switch (state)
    {
    case SP_API_WAITING_AUTH:
        return "WAITING_AUTH";
    case SP_API_AUTHENTICATING:
        return "AUTHENTICATING";
    case SP_API_READY:
        return "READY";
    case SP_API_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN_API_STATE";
    }
}

const char *sp_cover_state_to_str(enum sp_cover_state_t state)
{
    switch (state)
    {
    case SP_COVER_NONE:
        return "NONE";
    case SP_COVER_DOWNLOADING:
        return "DOWNLOADING";
    case SP_COVER_NEW_FILE:
        return "NEW_FILE";
    case SP_COVER_DECODED:
        return "DECODED";
    case SP_COVER_FAILED:
        return "FAILED";
    default:
        return "UNKNOWN_COVER_STATE";
    }
}