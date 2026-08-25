#include "nvs_hal.h"

#include "nvs_flash.h"
#include "nvs.h"

struct reg_entry
{
    const char *namespace;
    const char *key;
};

static const struct reg_entry registery[REG_MAX] = {
    [REG_SPOTIFY_REFRESH_TOKEN] = {"spotify", "refresh_token"}};

esp_err_t nvs_hal_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

esp_err_t nvs_hal_read_str(enum nvs_reg_key key, char *out_buff, size_t max_len)
{
    if (key >= REG_MAX || out_buff == NULL)
        return ESP_ERR_INVALID_ARG;

    const struct reg_entry *entry = &registery[key];
    nvs_handle_t handle;

    esp_err_t err = nvs_open(entry->namespace, NVS_READONLY, &handle);
    if (err == ESP_OK)
    {
        err = nvs_get_str(handle, entry->key, out_buff, &max_len);
        nvs_close(handle);
    }

    return err;
}

esp_err_t nvs_hal_write_str(enum nvs_reg_key key, const char *buff)
{
    if (key >= REG_MAX || buff == NULL)
        return ESP_ERR_INVALID_ARG;

    const struct reg_entry *entry = &registery[key];
    nvs_handle_t handle;

    esp_err_t err = nvs_open(entry->namespace, NVS_READWRITE, &handle);

    if (err == ESP_OK)
    {
        err = nvs_set_str(handle, entry->key, buff);
        if (err == ESP_OK)
            nvs_commit(handle);

        nvs_close(handle);
    }
    return err;
}