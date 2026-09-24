#include "web_server.h"

#include "root.h"
#include "spotify.h"
#include "api_screens.h"

#include "esp_https_server.h"
#include "esp_log.h"

// 1. ADD THIS LINE so web_server.c knows the function exists in api_upload.c
extern void register_upload_api_uri_handler(httpd_handle_t server);

extern const unsigned char cacert_pem_start[] asm("_binary_cacert_pem_start");
extern const unsigned char cacert_pem_end[] asm("_binary_cacert_pem_end");
extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
extern const unsigned char prvtkey_pem_end[] asm("_binary_prvtkey_pem_end");

static const char *TAG = "web_server";

static httpd_handle_t run_web_server(void)
{
    httpd_handle_t server = NULL;
    httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();

    conf.httpd.max_uri_len = 2048;

    conf.servercert = cacert_pem_start;
    conf.servercert_len = cacert_pem_end - cacert_pem_start;
    conf.prvtkey_pem = prvtkey_pem_start;
    conf.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;
    conf.httpd.max_open_sockets = 2;

    ESP_LOGI("SERVER", "Starting HTTPS Server on port 443...");

    if (httpd_ssl_start(&server, &conf) == ESP_OK)
    {
        register_root_uri_handler(server);
        register_spotify_uri_handler(server);
        register_screens_api_uri_handler(server);

        // 2. ADD THIS LINE to actually link the URL to the web server
        register_upload_api_uri_handler(server);

        ESP_LOGI("SERVER", "HTTPS Server started successfully!");
    }
    else
    {
        ESP_LOGE("SERVER", "Failed to start HTTPS Server!");
    }
    return server;
}

void start_web_server(void)
{
    httpd_handle_t server = run_web_server();
    if (server)
    {
        ESP_LOGI(TAG, "Web Server initialized. Waiting for Wi-Fi connection...");
    }
}