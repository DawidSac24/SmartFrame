#pragma once

#include <stdint.h>

#define DEFAULT_BUFF_SIZE 256
#define DEFAULT_AUTH_CODE_BUFF_SIZE 512

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
