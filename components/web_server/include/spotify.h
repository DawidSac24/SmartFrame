#pragma once

#include "esp_http_server.h"

esp_err_t spotify_get_page_handler(httpd_req_t *req);

void register_spotify_uri_handler(httpd_handle_t server);
