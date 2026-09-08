#pragma once

#include "esp_err.h"

#define TOKEN_BUFF_SIZE 256

// -- authentification --
void spotify_auth_init(void);
esp_err_t spotify_auth_get_token(char *out_token, size_t max_len);
esp_err_t spotify_auth_handle_callback(const char *code);

// -- client --
struct spotify_token_response
{
    char access_token[TOKEN_BUFF_SIZE];
    char refresh_token[TOKEN_BUFF_SIZE];
    int expires_in_sec;
    bool has_new_refresh_token;
};

esp_err_t spotify_client_exchange_code(const char *auth_code, const char *auth_header,
                                       struct spotify_token_response *out_response);
esp_err_t spotify_client_refresh_token(const char *refresh_token, const char *auth_header,
                                       struct spotify_token_response *out_response);
esp_err_t spotify_auth_send_code(const char *auth_code);

// -- parser --
esp_err_t spotify_parse_token_response(const char *json,
                                       struct spotify_token_response *out);

// -- storage --
esp_err_t spotify_storage_get_refresh_token(char *out_token, size_t max_len);
esp_err_t spotify_storage_set_refresh_token(char *token);