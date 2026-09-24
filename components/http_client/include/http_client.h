#pragma once

#include "esp_http_client.h"

#define HTTP_REQUEST_BODY_BUFF_SIZE 512

enum http_client_method
{
    HTTP_CLIENT_GET = HTTP_METHOD_GET,
    HTTP_CLIENT_POST = HTTP_METHOD_POST,
    HTTP_CLIENT_PUT = HTTP_METHOD_PUT,
    HTTP_CLIENT_PATCH = HTTP_METHOD_PATCH,
    HTTP_CLIENT_DELETE = HTTP_METHOD_DELETE
};

void http_client_init();

esp_err_t http_client_request(const char *url,
                              enum http_client_method method,
                              const char *auth_header,
                              const char *content_type,
                              const char *post_body,
                              char **out_response,
                              int *out_status_code);

esp_err_t http_client_download_file(const char *url, const char *filepath);