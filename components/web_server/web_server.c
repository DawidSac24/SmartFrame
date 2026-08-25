#include "web_server.h"

#include "root.h"
#include "spotify.h"

#include "esp_https_server.h"
#include "esp_log.h"

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

    ESP_LOGI("SERVER", "Starting HTTPS Server on port 443...");

    if (httpd_ssl_start(&server, &conf) == ESP_OK)
    {
        register_root_uri_handler(server);
        register_spotify_uri_handler(server);
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