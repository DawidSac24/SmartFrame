#include "spotify.h"

#include "api_spotify.h"
#include "esp_log.h"
#include "secrets.h"

#define REDIRECT_URI "https%3A%2F%2Fsmartframe.local%2Fcallback"

extern const uint8_t spotify_html_start[] asm("_binary_spotify_html_start");
extern const uint8_t spotify_html_end[] asm("_binary_spotify_html_end");

esp_err_t spotify_get_page_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");

    const size_t size = (spotify_html_end - spotify_html_start);

    return httpd_resp_send(req, (const char *)spotify_html_start, size);
}

static esp_err_t spotify_login_handler(httpd_req_t *req)
{
    char spotify_url[512];

    snprintf(spotify_url, sizeof(spotify_url),
             "https://accounts.spotify.com/authorize"
             "?client_id=%s"
             "&response_type=code"
             "&redirect_uri=%s"
             "&scope=user-read-currently-playing",
             SPOTIFY_CLIENT_ID, REDIRECT_URI);

    httpd_resp_set_status(req, "302 Found");

    httpd_resp_set_hdr(req, "Location", spotify_url);

    // Send empty response body since it is a redirect
    httpd_resp_send(req, NULL, 0);

    ESP_LOGI("SERVER", "User clicked login. Redirecting to Spotify...");
    return ESP_OK;
}

static esp_err_t spotify_callback_handler(httpd_req_t *req)
{
    char query_string[512];

    if (httpd_req_get_url_query_str(req, query_string, sizeof(query_string)) == ESP_OK)
    {

        char auth_code[512];

        if (httpd_query_key_value(query_string, "code", auth_code, sizeof(auth_code)) == ESP_OK)
        {
            esp_err_t err = spotify_send_auth_code(auth_code);

            if (err == ESP_OK)
            {
                const char *success_html =
                    "<body style='font-family: Arial; text-align: center; padding: 50px;'>"
                    "<h1 style='color: #1DB954;'>Success!</h1>"
                    "<p>Your Smart Frame is now connected to Spotify.</p>"
                    "<p>You can close this window.</p></body>";

                httpd_resp_set_hdr(req, "Connection", "close"); // Force SSL memory cleanup!
                httpd_resp_send(req, success_html, HTTPD_RESP_USE_STRLEN);
                return ESP_OK;
            }
            else
            {
                const char *overload_html =
                    "<body style='font-family: Arial; text-align: center; padding: 50px;'>"
                    "<h1 style='color: #E22134;'>System Busy</h1>"
                    "<p>The Smart Frame is currently processing another request.</p>"
                    "<a href='/login'>Click here to try again</a></body>";

                httpd_resp_set_status(req, "503 Service Unavailable");
                httpd_resp_send(req, overload_html, HTTPD_RESP_USE_STRLEN);
                return ESP_ERR_TIMEOUT;
            }
        }
    }

    const char *error_html = "<h1>Error</h1><p>Spotify authorization failed or was canceled.</p>";
    httpd_resp_send(req, error_html, HTTPD_RESP_USE_STRLEN);

    return ESP_ERR_NOT_ALLOWED;
}

void register_spotify_uri_handler(httpd_handle_t server)
{
    static const httpd_uri_t uri_get = {
        .uri = "/spotify",
        .method = HTTP_GET,
        .handler = spotify_get_page_handler,
        .user_ctx = NULL};

    static const httpd_uri_t uri_login = {
        .uri = "/login",
        .method = HTTP_GET,
        .handler = spotify_login_handler,
        .user_ctx = NULL};

    static const httpd_uri_t uri_callback = {
        .uri = "/callback",
        .method = HTTP_GET,
        .handler = spotify_callback_handler,
        .user_ctx = NULL};

    httpd_register_uri_handler(server, &uri_get);
    httpd_register_uri_handler(server, &uri_login);
    httpd_register_uri_handler(server, &uri_callback);
}
