#include "spotify.h"
#include "api_spotify.h"
#include "esp_log.h"
#include "secrets.h"

#define REDIRECT_URI "https%3A%2F%2Fsmartframe.local%2Fcallback"

// The /login endpoint redirects the browser to Spotify's official auth page
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

// Spotify redirects back to this endpoint with the auth code
static esp_err_t spotify_callback_handler(httpd_req_t *req)
{
    // Note for later: This is where your Slab Allocator will be perfect to replace
    // these large stack arrays! (char query_string[512])
    char query_string[512];

    if (httpd_req_get_url_query_str(req, query_string, sizeof(query_string)) == ESP_OK)
    {
        char auth_code[512];

        if (httpd_query_key_value(query_string, "code", auth_code, sizeof(auth_code)) == ESP_OK)
        {
            esp_err_t err = spotify_send_auth_code(auth_code);

            if (err == ESP_OK)
            {
                // Auto-redirects back to the root UI after 2 seconds
                const char *success_html =
                    "<html><head><meta http-equiv='refresh' content='2;url=/'></head>"
                    "<body style='font-family: system-ui, sans-serif; background: #09090b; color: #fff; text-align: center; padding-top: 20%;'>"
                    "<h1 style='color: #22c55e;'>Successfully Authenticated!</h1>"
                    "<p style='color: #a1a1aa;'>Redirecting back to the control panel...</p></body></html>";

                httpd_resp_set_hdr(req, "Connection", "close"); // Force SSL memory cleanup
                httpd_resp_send(req, success_html, HTTPD_RESP_USE_STRLEN);
                return ESP_OK;
            }
            else
            {
                const char *overload_html =
                    "<html><body style='font-family: system-ui, sans-serif; background: #09090b; color: #fff; text-align: center; padding-top: 20%;'>"
                    "<h1 style='color: #ef4444;'>System Busy</h1>"
                    "<p>The Smart Frame is currently processing another request.</p>"
                    "<a href='/login' style='color: #f97316;'>Click here to try again</a></body></html>";

                httpd_resp_set_status(req, "503 Service Unavailable");
                httpd_resp_send(req, overload_html, HTTPD_RESP_USE_STRLEN);
                return ESP_ERR_TIMEOUT;
            }
        }
    }

    const char *error_html =
        "<html><body style='font-family: system-ui, sans-serif; background: #09090b; color: #fff; text-align: center; padding-top: 20%;'>"
        "<h1 style='color: #ef4444;'>Error</h1>"
        "<p>Spotify authorization failed or was canceled.</p>"
        "<a href='/' style='color: #f97316;'>Return to Control Panel</a></body></html>";

    httpd_resp_send(req, error_html, HTTPD_RESP_USE_STRLEN);
    return ESP_ERR_NOT_ALLOWED;
}

void register_spotify_uri_handler(httpd_handle_t server)
{
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

    httpd_register_uri_handler(server, &uri_login);
    httpd_register_uri_handler(server, &uri_callback);
}