#pragma once

#include "sp_types.h"

#include "esp_err.h"

void sp_auth_init(void);
void sp_auth_clear(void);

// returns ESP_OK if the authentication state is valid, ESP_ERR_INVALID_STATE if not
esp_err_t sp_auth_is_valid(void);
// ensures the authentication state is valid, refreshing the token or halting for new refresh token if necessary
esp_err_t sp_auth_ensure_valid(void);
esp_err_t sp_auth_get_token(char *out_token, size_t max_len);
esp_err_t sp_auth_get_token_state(struct sp_auth_token_state *token_state);

esp_err_t sp_auth_send_code(const char *code);
