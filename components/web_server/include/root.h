#pragma once

#include "esp_http_server.h"

esp_err_t root_get_handler(httpd_req_t *req);

void register_root_uri_handler(httpd_handle_t server);
