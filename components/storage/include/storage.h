#pragma once

#include "esp_err.h"
#include "esp_littlefs.h"

esp_err_t storage_init();

esp_err_t storage_get_str(const char *key, char *out_buff, size_t max_len);
esp_err_t storage_set_str(const char *key, const char *val);

esp_err_t storage_read_buff(const char *filepath, uint8_t **out_data, size_t *out_size);
esp_err_t storage_write_buff(const char *filepath, const uint8_t *data, size_t size);

FILE *storage_open_stream(const char *filepath, const char *mode);
size_t storage_write_stream(FILE *file, const uint8_t *data, size_t size);
void storage_close_stream(FILE *file);