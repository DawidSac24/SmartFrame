#include "storage_prv.h"

#include "nvs_flash.h"
#include "nvs.h"

#define NVS_NAMESPACE "smart_frame"

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

esp_err_t nvs_hal_get_str(const char *key, char *out_buff, size_t max_len)
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_OK)
    {
        err = nvs_get_str(handle, key, out_buff, &max_len);
        nvs_close(handle);
    }

    return err;
}

esp_err_t nvs_hal_set_str(const char *key, const char *val)
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);

    if (err == ESP_OK)
    {
        err = nvs_set_str(handle, key, val);
        if (err == ESP_OK)
            nvs_commit(handle);

        nvs_close(handle);
    }
    return err;
}