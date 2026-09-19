#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>

struct spotify_track_dto
{
    char track_name[64];
    char artist_name[64];
    char cover_url[128];
    bool is_playing;
    bool has_new_cover;
};

esp_err_t spotify_init(void);
esp_err_t spotify_get_track_info(struct spotify_track_dto *out_state);

esp_err_t spotify_send_auth_code(const char *auth_code);
