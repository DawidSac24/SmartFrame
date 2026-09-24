#include "http_client.h"
#include "storage.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include <string.h>

struct http_response
{
    char *buffer;
    int length;
};

SemaphoreHandle_t s_network_mutex = NULL;

void http_client_init()
{
    if (s_network_mutex == NULL)
    {
        s_network_mutex = xSemaphoreCreateMutex();
    }
}

bool take_network_mutex()
{
    if (s_network_mutex != NULL)
        return xSemaphoreTake(s_network_mutex, portMAX_DELAY) == pdTRUE;
    return false;
}

void give_network_mutex()
{
    if (s_network_mutex != NULL)
        xSemaphoreGive(s_network_mutex);
}

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    struct http_response *res = (struct http_response *)evt->user_data;

    if (evt->event_id == HTTP_EVENT_ON_DATA)
    {
        char *new_ptr = realloc(res->buffer, res->length + evt->data_len + 1);
        if (new_ptr == NULL)
        {
            ESP_LOGE("http_client", "Out of memory during download!");
            return ESP_FAIL;
        }

        res->buffer = new_ptr;
        memcpy(res->buffer + res->length, evt->data, evt->data_len);
        res->length += evt->data_len;
        res->buffer[res->length] = '\0';
    }
    return ESP_OK;
}

esp_err_t http_client_request(const char *url,
                              enum http_client_method method,
                              const char *auth_header,
                              const char *content_type,
                              const char *post_body,
                              char **out_response,
                              int *out_status_code)
{
    if (!take_network_mutex())
        return ESP_ERR_INVALID_STATE;

    esp_err_t final_err = ESP_FAIL;
    struct http_response response_data = {.buffer = NULL, .length = 0};

    esp_http_client_config_t config = {
        .url = url,
        .method = method,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 15000,
        .event_handler = http_event_handler,
        .user_data = &response_data,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client)
    {
        give_network_mutex();
        return ESP_FAIL;
    }

    if (auth_header)
        esp_http_client_set_header(client, "Authorization", auth_header);
    if (content_type)
        esp_http_client_set_header(client, "Content-Type", content_type);
    if (post_body)
        esp_http_client_set_post_field(client, post_body, strlen(post_body));

    final_err = esp_http_client_perform(client);

    if (final_err == ESP_OK)
    {
        *out_status_code = esp_http_client_get_status_code(client);
        *out_response = response_data.buffer;
    }
    else
    {
        if (response_data.buffer)
            free(response_data.buffer);
        *out_response = NULL;
    }

    esp_http_client_cleanup(client);
    give_network_mutex();
    return final_err;
}

esp_err_t http_client_download_file(const char *url, const char *filepath)
{
    if (!take_network_mutex())
        return ESP_ERR_INVALID_STATE;

    esp_http_client_config_t config = {
        .url = url,
        .crt_bundle_attach = esp_crt_bundle_attach};

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client)
    {
        give_network_mutex();
        return ESP_FAIL;
    }

    if (esp_http_client_open(client, 0) != ESP_OK)
    {
        esp_http_client_cleanup(client);
        give_network_mutex();
        return ESP_FAIL;
    }

    esp_http_client_fetch_headers(client);

    FILE *file = storage_open_stream(filepath, "wb");
    if (file == NULL)
    {
        esp_http_client_cleanup(client);
        give_network_mutex();
        return ESP_FAIL;
    }

    uint8_t buffer[1024];
    int bytes_read;
    int total_bytes = 0;

    while ((bytes_read = esp_http_client_read(client, (char *)buffer, sizeof(buffer))) > 0)
    {
        storage_write_stream(file, buffer, bytes_read);
        total_bytes += bytes_read;
    }

    storage_close_stream(file);
    esp_http_client_cleanup(client);
    give_network_mutex();

    ESP_LOGD("HTTP", "Download complete. Total bytes written: %d", total_bytes);
    return ESP_OK;
}