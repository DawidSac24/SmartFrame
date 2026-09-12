#include "spotify_prv.h"

#include "esp_log.h"
#include "esp_console.h"

static const char *TAG = "spotify_cmd";

esp_err_t spotify_cmd_fetch(int argc, char **argv);

esp_err_t spotify_cmd_print(int argc, char **argv);

static esp_err_t cmd_spotify(int argc, char **argv)
{
    if (argc < 2)
    {
        ESP_LOGE(TAG, "No argument provided. Use 'print' or 'fetch'.");
        return ESP_ERR_INVALID_ARG;
    }

    if (strcmp(argv[1], "print") == 0)
    {
        return spotify_cmd_print(0, NULL);
    }
    else if (strcmp(argv[1], "fetch") == 0)
    {
        return spotify_cmd_fetch(argc, argv);
    }
    else
    {
        printf("Error: Unknown argument '%s'\n", argv[1]);
    }

    return ESP_OK;
}

void spotify_cmd_register(void)
{
    esp_console_cmd_t cmd = {
        .command = "spotify",
        .help = "Manage the spotify API (Args: print, fetch)",
        .hint = "<print|fetch>",
        .func = &cmd_spotify,
    };
    esp_console_cmd_register(&cmd);
}

esp_err_t spotify_cmd_fetch(int argc, char **argv)
{
    if (argc < 3)
    {
        ESP_LOGE(TAG, "fetch needs an argument. Use 'token' or 'track'.");
        return ESP_ERR_INVALID_ARG;
    }

    if (strcmp(argv[2], "token") == 0)
    {
        return spotify_auth_fetch_token();
    }
    else if (strcmp(argv[2], "track") == 0)
    {
        return ESP_ERR_NOT_SUPPORTED;
    }
    else
    {
        printf("Error: Unknown argument '%s'\n", argv[2]);
    }
    return ESP_OK;
}

esp_err_t spotify_cmd_print(int argc, char **argv)
{
    printf("\n--- CURRENT SPOTIFY STATE ---\n");
    struct spotify_auth_token_state token_state;
    esp_err_t spotify_token_res = spotify_auth_get_token_state(&token_state);
    if (spotify_token_res != ESP_OK)
    {
        printf("Spotify Token unauthentificated");
        return spotify_token_res;
    }

    printf("Refresh Token: %s", token_state.refresh_token);
    printf("Access Token: %s", token_state.access_token);
    printf("Expires at: %lld", token_state.expires_at);

    printf("Currently played track: [Not Implemented]");
    return ESP_OK;
}