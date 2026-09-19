#pragma once

#include "sp_types.h"

#include "esp_err.h"

void sp_auth_init(void);

bool sp_auth_is_access_expired();
esp_err_t sp_auth_get_access_token(char *out_buffer, size_t max_len);
esp_err_t sp_auth_get_refresh_token(char *out_buffer, size_t max_len);
esp_err_t sp_auth_get_auth_code(char *out_code);
void sp_auth_get_header(char *out_buff, size_t max_len);

void sp_auth_set_access_token(const char *buff, int expires_in_sec);
void sp_auth_set_refresh_token(const char *buff);
esp_err_t sp_auth_set_auth_code(const char *code);

// ensures the authentication state is valid, refreshing the token or halting for new refresh token if necessary
// esp_err_t sp_auth_ensure_valid(void);
// esp_err_t sp_auth_get_token(char *out_token, size_t max_len);
// esp_err_t sp_auth_get_token_state(struct sp_auth_token_state *token_state);
