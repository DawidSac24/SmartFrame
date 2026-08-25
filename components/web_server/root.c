#include "root.h"

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");

    const size_t size = (index_html_end - index_html_start);

    return httpd_resp_send(req, (const char *)index_html_start, size);
}

void register_root_uri_handler(httpd_handle_t server)
{
    static const httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler,
        .user_ctx = NULL};

    httpd_register_uri_handler(server, &uri_get);
}