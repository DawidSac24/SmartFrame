#pragma once

#include <stdint.h>

#define DEFAULT_BUFF_SIZE 256

struct sp_auth_token_state
{
    char access_token[DEFAULT_BUFF_SIZE];
    char refresh_token[DEFAULT_BUFF_SIZE];
    int64_t expires_at;
};

struct sp_token_response
{
    char access_token[DEFAULT_BUFF_SIZE];
    char refresh_token[DEFAULT_BUFF_SIZE];
    int expires_in_sec;
    bool has_new_refresh_token;
};

struct sp_track_info
{
    char track_name[64];
    char artist_name[64];
    char image_url[128];
    bool is_playing;
};
