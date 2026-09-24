#include "esp_http_server.h"
#include "esp_log.h"
#include "api_custom_img.h"
#include <stdio.h>

static const char *TAG = "api_upload";

esp_err_t image_upload_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Receiving custom image... Size: %d bytes", req->content_len);

    FILE *fd = fopen("/fs/custom.png", "w");
    if (!fd)
        return ESP_FAIL;

    char buf[1024];
    int remaining = req->content_len;
    while (remaining > 0)
    {
        int to_read = (remaining < sizeof(buf)) ? remaining : sizeof(buf);
        int ret = httpd_req_recv(req, buf, to_read);
        if (ret <= 0)
        {
            fclose(fd);
            return ESP_FAIL;
        }
        fwrite(buf, 1, ret, fd);
        remaining -= ret;
    }
    fclose(fd);

    ESP_LOGI(TAG, "Image saved! Notifying UI...");
    custom_img_set_state(CUSTOM_IMG_NEW_FILE);
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

esp_err_t image_clear_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Clearing custom image...");
    remove("/fs/custom.png"); // Delete the file from LittleFS

    // Force the screen to reload. It will see the file is missing and draw "No Image"
    custom_img_set_state(CUSTOM_IMG_NEW_FILE);

    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

void register_upload_api_uri_handler(httpd_handle_t server)
{
    static const httpd_uri_t uri_post = {
        .uri = "/api/upload_image",
        .method = HTTP_POST,
        .handler = image_upload_post_handler,
        .user_ctx = NULL};

    // --- ADD THIS BLOCK ---
    static const httpd_uri_t uri_clear = {
        .uri = "/api/clear_image",
        .method = HTTP_POST,
        .handler = image_clear_post_handler,
        .user_ctx = NULL};

    httpd_register_uri_handler(server, &uri_post);
    httpd_register_uri_handler(server, &uri_clear);
}
