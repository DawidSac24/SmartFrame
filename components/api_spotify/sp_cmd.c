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

static const char *sp_cmd_strings[] = {
    "fetch",
    "print",
    "clear"};

static esp_err_t
cmd_spotify(int argc, char **argv);
esp_err_t sp_cmd_fetch_track(void);
esp_err_t sp_cmd_print(void);
esp_err_t sp_cmd_clear(void);

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

void sp_cmd_send(enum sp_cmd cmd)
{
    if (sp_cmd_queue != NULL)
    {
        xQueueSend(sp_cmd_queue, &cmd, 0);
    }
}

esp_err_t sp_cmd_dispatch(enum sp_cmd cmd)
{
    switch (cmd)
    {
    case SP_CMD_FETCH_TRACK:
        return sp_cmd_fetch_track();
    case SP_CMD_PRINT:
        return sp_cmd_print();
    case SP_CMD_CLEAR:
        return sp_cmd_clear();
    default:
        ESP_LOGE(TAG, "Unkown command received: %d", cmd);
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_FAIL;
}

esp_err_t cmd_spotify(int argc, char **argv)
{
    if (argc < 2)
    {
        ESP_LOGE(TAG, "No argument provided. Use:");
        for (int i = 0; i < SP_CMD_UNKNOWN; i++)
        {
            ESP_LOGE(TAG, "spotify %s", sp_cmd_strings[i]);
        }
        return ESP_ERR_INVALID_ARG;
    }

    enum sp_cmd cmd = sp_str_to_cmd(argv[1]);

    if (cmd == SP_CMD_UNKNOWN)
    {
        ESP_LOGE(TAG, "Unknown argument '%s'", argv[1]);
        return ESP_ERR_INVALID_ARG;
    }

    sp_cmd_send(cmd);

    ESP_LOGI(TAG, "Command '%s' dispatched to task.", argv[1]);
    return ESP_OK;
}

enum sp_cmd sp_str_to_cmd(const char *str)
{
    int num_cmds = sizeof(sp_cmd_strings) / sizeof(sp_cmd_strings[0]);
    for (int i = 0; i < num_cmds; i++)
    {
        if (strcmp(str, sp_cmd_strings[i]) == 0)
        {
            return (enum sp_cmd)i;
        }
    }
    return SP_CMD_UNKNOWN;
}

esp_err_t sp_cmd_fetch_track(void)
{
    return sp_manager_fetch_and_save_track();
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