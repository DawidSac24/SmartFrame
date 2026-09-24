#include "api_screens.h"
#include "screen_scheduler.h"
#include "ui_task.h"
#include "cJSON.h"
#include "esp_log.h"
#include <string.h>
#include "gfx.h"

static const char *TAG = "api_screens";

// Helper to list available themes per screen
static void add_themes_for_screen(cJSON *themes_arr, const char *screen_name)
{
    if (strcmp(screen_name, "spotify") == 0)
    {
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("default"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("square"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("disk"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("spinning_disk"));
    }
    else if (strcmp(screen_name, "weather") == 0)
    {
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("default"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("compact"));
    }
    else if (strcmp(screen_name, "clock") == 0)
    {
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("default"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("default_small"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("date_center"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("date_left"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("split"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("round"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("round_num"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("square"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("square_num"));
    }
    // --- NEW: Add Game of Life themes so the UI dropdown populates ---
    else if (strcmp(screen_name, "game_of_life") == 0)
    {
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("solid"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("rainbow"));
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("layered"));
    }
    else
    {
        cJSON_AddItemToArray(themes_arr, cJSON_CreateString("default"));
    }
}

// GET /api/screens - Returns all screens and configuration
static esp_err_t screens_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store, no-cache, must-revalidate");

    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "spotify_exclusive", ui_task_get_spotify_exclusive());

    cJSON_AddNumberToObject(root, "brightness", gfx_get_brightness());
    cJSON_AddNumberToObject(root, "orientation", gfx_get_rotation());

    cJSON *screens_arr = cJSON_CreateArray();
    struct screen *all_screens[8];
    size_t count = sched_get_all_screens(all_screens, 8);

    for (size_t i = 0; i < count; i++)
    {
        struct screen *s = all_screens[i];
        if (!s || !s->name)
            continue;

        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "id", s->name);
        cJSON_AddBoolToObject(item, "active", sched_is_screen_active(s));
        cJSON_AddNumberToObject(item, "duration", s->display_duration_ms / 1000); // in seconds
        cJSON_AddStringToObject(item, "theme", s->active_theme ? s->active_theme->name : "default");

        cJSON *themes = cJSON_CreateArray();
        add_themes_for_screen(themes, s->name);
        cJSON_AddItemToObject(item, "themes", themes);

        // Optional: If you wanted to send the CURRENT color back to the UI, you could do it here.
        // For now, the UI will just dictate the color to the ESP32.

        cJSON_AddItemToArray(screens_arr, item);
    }
    cJSON_AddItemToObject(root, "screens", screens_arr);

    char *json_str = cJSON_PrintUnformatted(root);
    httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);

    free(json_str);
    cJSON_Delete(root);
    return ESP_OK;
}

// POST /api/screens - Applies incoming configuration
esp_err_t screens_post_handler(httpd_req_t *req)
{
    char buf[1024];
    int total_len = req->content_len;

    if (total_len >= sizeof(buf))
    {
        ESP_LOGE(TAG, "Payload too large!");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Payload too large");
        return ESP_FAIL;
    }

    int ret = httpd_req_recv(req, buf, total_len);
    if (ret <= 0)
    {
        return ESP_FAIL;
    }
    buf[ret] = '\0'; // Ensure null-termination

    cJSON *root = cJSON_Parse(buf);
    if (!root)
    {
        ESP_LOGE(TAG, "JSON Parse Error before: [%s]", cJSON_GetErrorPtr());
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    // 1. Update Global Settings
    cJSON *sp_excl = cJSON_GetObjectItem(root, "spotify_exclusive");
    if (cJSON_IsBool(sp_excl))
    {
        ui_task_set_spotify_exclusive(cJSON_IsTrue(sp_excl));
    }

    cJSON *bright_obj = cJSON_GetObjectItem(root, "brightness");
    if (cJSON_IsNumber(bright_obj))
    {
        gfx_set_brightness((uint8_t)bright_obj->valueint);
        ESP_LOGI(TAG, "Brightness set to: %d", bright_obj->valueint);
    }

    cJSON *orient_obj = cJSON_GetObjectItem(root, "orientation");
    if (cJSON_IsNumber(orient_obj))
    {
        gfx_set_rotation((uint8_t)orient_obj->valueint);
        ESP_LOGI(TAG, "Orientation set to: %d", orient_obj->valueint);
    }

    // 2. Update screen parameters
    cJSON *screens_arr = cJSON_GetObjectItem(root, "screens");
    if (cJSON_IsArray(screens_arr))
    {
        int num_items = cJSON_GetArraySize(screens_arr);
        for (int i = 0; i < num_items; i++)
        {
            cJSON *item = cJSON_GetArrayItem(screens_arr, i);
            cJSON *id_obj = cJSON_GetObjectItem(item, "id");
            if (!cJSON_IsString(id_obj))
                continue;

            struct screen *s = sched_get_screen_by_name(id_obj->valuestring);
            if (!s)
                continue;

            // Duration
            cJSON *dur_obj = cJSON_GetObjectItem(item, "duration");
            if (cJSON_IsNumber(dur_obj) && dur_obj->valueint > 0)
            {
                s->display_duration_ms = dur_obj->valueint * 1000;
            }

            // Theme
            cJSON *th_obj = cJSON_GetObjectItem(item, "theme");
            if (cJSON_IsString(th_obj) && s->vtable && s->vtable->get_theme)
            {
                const struct theme *new_th = s->vtable->get_theme(th_obj->valuestring);
                if (new_th)
                {
                    s->active_theme = new_th;
                }
            }

            // Active / Inactive
            cJSON *act_obj = cJSON_GetObjectItem(item, "active");
            if (cJSON_IsBool(act_obj))
            {
                bool should_be_active = cJSON_IsTrue(act_obj);

                if (strcmp(s->name, "spotify") == 0)
                {
                    ui_task_set_spotify_enabled(should_be_active);
                }
                else
                {
                    bool is_active = sched_is_screen_active(s);
                    if (should_be_active && !is_active)
                    {
                        sched_add_screen(s);
                    }
                    else if (!should_be_active && is_active)
                    {
                        sched_remove_screen(s);
                    }
                }
            }

            // --- NEW: Parse Custom Parameters (like "color") ---
            cJSON *color_obj = cJSON_GetObjectItem(item, "color");
            if (cJSON_IsString(color_obj))
            {
                // Safely check if the active theme supports parameters
                if (s->active_theme && s->active_theme->vtable && s->active_theme->vtable->set_param)
                {
                    s->active_theme->vtable->set_param((struct theme *)s->active_theme, "color", color_obj->valuestring);
                }
            }
        }
    }

    cJSON_Delete(root);
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

void register_screens_api_uri_handler(httpd_handle_t server)
{
    static const httpd_uri_t uri_get = {
        .uri = "/api/screens",
        .method = HTTP_GET,
        .handler = screens_get_handler,
        .user_ctx = NULL};

    static const httpd_uri_t uri_post = {
        .uri = "/api/screens",
        .method = HTTP_POST,
        .handler = screens_post_handler,
        .user_ctx = NULL};

    httpd_register_uri_handler(server, &uri_get);
    httpd_register_uri_handler(server, &uri_post);
}