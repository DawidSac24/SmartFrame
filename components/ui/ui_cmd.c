#include "ui_cmd.h"

#include "screen.h"
#include "screen_scheduler.h"
#include "ui_task.h"

#include "esp_log.h"
#include <string.h>
#include "esp_console.h"

static const char *TAG = "ui_cmd";

static screen_factory_cb_t s_factory_cb = NULL;

static const char *ui_cmd_strings[] = {
    "add",
    "remove",
    "set",
    "next"};

static esp_err_t cmd_ui(int argc, char **argv);
static esp_err_t ui_cmd_set(const char *target, const char *key, const char *val);

void ui_cmd_register(void)
{
    esp_console_cmd_t cmd = {
        .command = "ui",
        .help = "Manage UI screens",
        .hint = "<add|remove|set|next> [target] [key] [val]",
        .func = &cmd_ui,
    };
    esp_console_cmd_register(&cmd);
}

static esp_err_t cmd_ui(int argc, char **argv)
{
    if (argc < 2)
    {
        ESP_LOGE(TAG, "No argument provided. Available commands:");
        for (int i = 0; i < sizeof(ui_cmd_strings) / sizeof(ui_cmd_strings[0]); i++)
        {
            ESP_LOGE(TAG, "  ui %s", ui_cmd_strings[i]);
        }
        return ESP_ERR_INVALID_ARG;
    }

    const char *action = argv[1];

    // --- EXECUTE: ui next ---
    if (strcmp(action, "next") == 0)
    {
        ESP_LOGI(TAG, "Executing NEXT screen");
        sched_next();
        return ESP_OK;
    }

    // --- EXECUTE: ui add <target> ---
    else if (strcmp(action, "add") == 0)
    {
        if (argc < 3)
        {
            ESP_LOGE(TAG, "Usage: ui add <target>");
            return ESP_ERR_INVALID_ARG;
        }
        const char *target = argv[2];

        if (!s_factory_cb)
        {
            ESP_LOGE(TAG, "No screen factory registered!");
            return ESP_ERR_INVALID_STATE;
        }

        // Call the injected function!
        struct screen *new_scr = s_factory_cb(target, 5000);
        if (new_scr)
        {
            sched_add_screen(new_scr);
            ESP_LOGI(TAG, "Added %s to scheduler", target);
        }
        else
        {
            ESP_LOGE(TAG, "Unknown screen: %s", target);
        }
        return ESP_OK;
    }

    // --- EXECUTE: ui remove <target> ---
    else if (strcmp(action, "remove") == 0)
    {
        if (argc < 3)
        {
            ESP_LOGE(TAG, "Usage: ui remove <target>");
            return ESP_ERR_INVALID_ARG;
        }
        const char *target = argv[2];

        ESP_LOGI(TAG, "Executing REMOVE for %s", target);
        sched_remove_screen(sched_get_screen_by_name(target));
        return ESP_OK;
    }

    // --- EXECUTE: ui set <target> <key> <val> ---
    else if (strcmp(action, "set") == 0)
    {
        if (argc < 5)
        {
            ESP_LOGE(TAG, "Usage: ui set <target> <key> <val>");
            return ESP_ERR_INVALID_ARG;
        }
        const char *target = argv[2];
        const char *key = argv[3];
        const char *val = argv[4];

        // INTERCEPT SPOTIFY EXCLUSIVITY TOGGLE
        if (strcmp(target, "spotify") == 0 && strcmp(key, "exclusive") == 0)
        {
            bool turn_on = (strcmp(val, "on") == 0 || strcmp(val, "1") == 0);
            ui_task_set_spotify_exclusive(turn_on);
            ESP_LOGI(TAG, "Spotify exclusivity set to: %s", turn_on ? "ON" : "OFF");
            return ESP_OK;
        }

        ui_cmd_set(target, key, val);

        return ESP_OK;
    }

    // --- UNKNOWN COMMAND ---
    else
    {
        ESP_LOGE(TAG, "Unknown action '%s'", action);
        return ESP_ERR_INVALID_ARG;
    }
}

static esp_err_t ui_cmd_set(const char *target, const char *key, const char *val)
{
    ESP_LOGI(TAG, "Executing SET for %s: %s = %s", target, key, val);

    struct screen *scr = sched_get_screen_by_name(target);
    if (!scr)
    {
        ESP_LOGE(TAG, "Screen '%s' not found", target);
        return ESP_ERR_NOT_FOUND;
    }

    // 1. AUTOMATIC THEME SWITCHING
    if (strcmp(key, "theme") == 0)
    {
        if (scr->vtable->get_theme)
        {
            const struct theme *t = scr->vtable->get_theme(val);
            if (t)
            {
                scr->active_theme = t;
                ESP_LOGI(TAG, "Theme successfully changed to '%s'", val);
                return ESP_OK;
            }
        }
        ESP_LOGE(TAG, "Screen '%s' does not have theme '%s'", target, val);
        return ESP_ERR_INVALID_ARG;
    }

    // 2. AUTOMATIC THEME CONFIGURATION (e.g. time_color 255,0,0)
    // If the active theme has a param handler, pass the setting to it!
    if (scr->active_theme && scr->active_theme->vtable->set_param)
    {
        // Cast away const safely because the theme struct itself holds the mutable data
        if (scr->active_theme->vtable->set_param((struct theme *)scr->active_theme, key, val))
        {
            return ESP_OK;
        }
    }

    // 3. FALLBACK TO SCREEN CONFIGURATION
    if (scr->vtable->set_param)
    {
        if (scr->vtable->set_param(scr, key, val))
        {
            return ESP_OK;
        }
    }

    ESP_LOGE(TAG, "Parameter '%s' not recognized", key);
    return ESP_ERR_INVALID_ARG;
}

void ui_cmd_set_factory(screen_factory_cb_t cb)
{
    s_factory_cb = cb;
}