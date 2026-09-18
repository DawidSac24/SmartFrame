#include "sp_storage.h"

#include "storage.h"

#define AUTH_TOKEN "spotify_token"

esp_err_t sp_storage_get_refresh_token(char *out_token, size_t max_len)
{
    return storage_get_str(AUTH_TOKEN, out_token, max_len);
}
esp_err_t sp_storage_set_refresh_token(char *token)
{
    return storage_set_str(AUTH_TOKEN, token);
}