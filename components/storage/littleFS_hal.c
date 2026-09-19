#include "storage.h"

#include "esp_log.h"
#include "esp_err.h"
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

esp_err_t fs_hal_init(void)
{
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/fs",
        .partition_label = "storage",
        .format_if_mount_failed = true,
        .dont_mount = false,
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK)
    {
        ESP_LOGE("STORAGE", "Failed to mount LittleFS: %s", esp_err_to_name(ret));
        return ret;
    }

    size_t total = 0, used = 0;
    esp_littlefs_info(conf.partition_label, &total, &used);
    ESP_LOGI("STORAGE", "LittleFS mounted. Total: %d, Used: %d", total, used);

    return ESP_OK;
}

esp_err_t storage_write_buffer(const char *filepath, const uint8_t *data, size_t size)
{
    if (filepath == NULL || data == NULL || size == 0)
        return ESP_ERR_INVALID_ARG;

    FILE *f = fopen(filepath, "wb");
    if (f == NULL)
    {
        return ESP_FAIL;
    }

    size_t written = fwrite(data, 1, size, f);
    fclose(f);

    return (written == size) ? ESP_OK : ESP_FAIL;
}

esp_err_t storage_read_buffer(const char *filepath, uint8_t **out_data, size_t *out_size)
{
    if (filepath == NULL || out_data == NULL || out_size == NULL)
        return ESP_ERR_INVALID_ARG;

    struct stat st;
    if (stat(filepath, &st) != 0)
    {
        return ESP_ERR_NOT_FOUND;
    }
    *out_size = st.st_size;

    *out_data = malloc(*out_size);
    if (*out_data == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    FILE *f = fopen(filepath, "rb");
    if (f == NULL)
    {
        free(*out_data);
        return ESP_FAIL;
    }

    size_t read_bytes = fread(*out_data, 1, *out_size, f);
    fclose(f);

    if (read_bytes != *out_size)
    {
        free(*out_data);
        return ESP_FAIL;
    }

    return ESP_OK;
}

FILE *storage_open_stream(const char *filepath, const char *mode)
{
    if (filepath == NULL || mode == NULL)
        return NULL;
    return fopen(filepath, mode);
}

size_t storage_write_stream(FILE *file, const uint8_t *data, size_t size)
{
    if (file == NULL || data == NULL || size == 0)
        return 0;
    return fwrite(data, 1, size, file);
}

void storage_close_stream(FILE *file)
{
    if (file != NULL)
    {
        fclose(file);
    }
}