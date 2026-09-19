#include "api_spotify.h"
#include "sp_state.h"
#include "sp_cmd.h"
#include "sp_types.h"
#include "sp_auth.h"
#include "sp_storage.h"
#include "sp_manager.h"

#include "esp_log.h"
#include "esp_console.h"

static const char *TAG = "spotify_cmd";

esp_err_t sp_cmd_fetch_track(void);

esp_err_t sp_cmd_print(void);

esp_err_t sp_cmd_clear(void);

static esp_err_t cmd_spotify(int argc, char **argv)
{
    if (argc < 2)
    {
        ESP_LOGE(TAG, "No argument provided. Use 'print' or 'fetch'.");
        return ESP_ERR_INVALID_ARG;
    }

    if (strcmp(argv[1], "print") == 0)
        return sp_cmd_print();

    else if (strcmp(argv[1], "fetch") == 0)
        return sp_cmd_fetch_track();

    else if (strcmp(argv[1], "clear") == 0)
        return sp_cmd_clear();

    else
        printf("Error: Unknown argument '%s'\n", argv[1]);

    return ESP_OK;
}

void sp_cmd_register(void)
{
    esp_console_cmd_t cmd = {
        .command = "spotify",
        .help = "Manage the spotify API (Args: print, fetch)",
        .hint = "<print|fetch>",
        .func = &cmd_spotify,
    };
    esp_console_cmd_register(&cmd);
}

esp_err_t sp_cmd_fetch_track(void)
{
    // if (sp_auth_is_valid() != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Spotify authentication is not valid.");
    //     return ESP_ERR_INVALID_STATE;
    // }

    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t sp_cmd_clear(void)
{
    sp_storage_set_refresh_token("");
    sp_auth_init();

    struct spotify_track_dto empty_track;
    empty_track.track_name[0] = '\0';
    empty_track.artist_name[0] = '\0';
    empty_track.cover_url[0] = '\0';
    empty_track.is_playing = false;
    empty_track.cover_state = SP_COVER_NONE;

    esp_err_t set_track_err = sp_state_set_track_info(&empty_track);
    if (set_track_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to clear the track from the spotify state: %s", esp_err_to_name(set_track_err));
        return set_track_err;
    }

    return ESP_OK;
}

esp_err_t sp_cmd_print(void)
{
    ESP_LOGI(TAG, "\n--- CURRENT SPOTIFY STATE ---\n");

    if (sp_auth_is_access_expired())
    {
        ESP_LOGI(TAG, "Acces token is expired!");
    }

    // Printing tokens
    char token[DEFAULT_BUFF_SIZE];
    esp_err_t token_err = sp_auth_get_access_token(token, sizeof(token));
    if (token_err == ESP_OK)
        ESP_LOGI(TAG, "Access token: %s", token);
    else
        ESP_LOGI(TAG, "Didn't get the Refresh token: %s", esp_err_to_name(token_err));

    token_err = sp_auth_get_refresh_token(token, sizeof(token));
    if (token_err == ESP_OK)
        ESP_LOGI(TAG, "Refresh token: %s", token);
    else
        ESP_LOGI(TAG, "Didn't get the Refresh token: %s", esp_err_to_name(token_err));

    // Printing track info
    return sp_manager_print_track();
}