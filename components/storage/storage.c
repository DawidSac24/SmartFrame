#include "storage.h"
#include "storage_prv.h"

#include "esp_log.h"

static const char *TAG = "storage";

esp_err_t storage_init()
{
    esp_err_t nvs_res = nvs_hal_init();

    if (nvs_res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialise nvs: %s", esp_err_to_name(nvs_res));
        return nvs_res;
    }

    esp_err_t fs_res = fs_hal_init();

    if (fs_res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialise littleFS: %s", esp_err_to_name(fs_res));
        return fs_res;
    }

    return ESP_OK;
}

esp_err_t storage_get_str(const char *key, char *out_buff, size_t max_len)
{
    if (key == NULL || out_buff == NULL)
        return ESP_ERR_INVALID_ARG;

    if (key[0] == '/')
    {
        return ESP_ERR_NOT_SUPPORTED; // add a little_fs read str function?
    }
    else
    {
        if (strlen(key) > 15)
            return ESP_ERR_INVALID_ARG;
        return nvs_hal_get_str(key, out_buff, max_len);
    }
}

esp_err_t storage_set_str(const char *key, const char *val)
{
    if (key == NULL)
        return ESP_ERR_INVALID_ARG;

    if (key[0] == '/')
    {
        return ESP_ERR_NOT_SUPPORTED; // add a little_fs write str function?
    }
    else
    {
        if (strlen(key) > 15)
            return ESP_ERR_INVALID_ARG;
        return nvs_hal_set_str(key, val);
    }
}

esp_err_t storage_read_buff(const char *filepath, uint8_t **out_data, size_t *out_size)
{
    return fs_hal_read_buff(filepath, out_data, out_size);
}
esp_err_t storage_write_buff(const char *filepath, const uint8_t *data, size_t size)
{
    return fs_hal_write_buff(filepath, data, size);
}