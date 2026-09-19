#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>

enum sp_api_state
{
    SP_API_WAITING_AUTH = 0,
    SP_API_AUTHENTICATING,
    SP_API_READY,
    SP_API_ERROR
};

enum sp_cover_state_t
{
    SP_COVER_NONE = 0,
    SP_COVER_DOWNLOADING,
    SP_COVER_NEW_FILE,
    SP_COVER_DECODED,
    SP_COVER_FAILED
};

struct spotify_track_dto
{
    char track_name[64];
    char artist_name[64];
    char cover_url[128];
    bool is_playing;
    enum sp_cover_state_t cover_state;
};

esp_err_t spotify_init(void);
enum sp_api_state spotify_get_state();
esp_err_t spotify_get_track_info(struct spotify_track_dto *out_state);

esp_err_t spotify_send_auth_code(const char *auth_code);
