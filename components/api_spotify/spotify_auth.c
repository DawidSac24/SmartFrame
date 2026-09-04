#include "spotify_prv.h"

#include "secrets.h"

#include <mbedtls/base64.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "spotify_client";

// private variables
static const char *TAG = "spotify_auth";

static struct spotify_auth_state
{
  char access_token[TOKEN_BUFF_SIZE];
  char refresh_token[TOKEN_BUFF_SIZE];
  char auth_header[TOKEN_BUFF_SIZE];
  int64_t expires_at; // esp_timer_get_time() + (expires_in * 1000000)
  bool has_refresh_token;
  QueueHandle_t auth_code_queue; // Owns the queue for the web server callback
} g_auth_state;

// private functions
esp_err_t spotify_auth_refresh_token();
bool spotify_auth_poll_auth_code(char *out_code);
esp_err_t spotify_auth_exchange_code(char *auth_code, size_t len);

// esp_err_t spotify_auth_get_refresh_token(char *out_token);
// esp_err_t spotify_auth_exchange_token(const char *code);

void spotify_auth_init(void)
{
  g_auth_state.access_token[0] = '\0';
  g_auth_state.refresh_token[0] = '\0';
  g_auth_state.expires_at = 0;
  g_auth_state.has_refresh_token = false;

  g_auth_state.auth_code_queue = xQueueCreate(1, TOKEN_BUFF_SIZE * sizeof(char));

  // Cache authentification header
  char id_secret[TOKEN_BUFF_SIZE];
  snprintf(id_secret, sizeof(id_secret), "%s:%s", SPOTIFY_CLIENT_ID,
           SPOTIFY_CLIENT_SECRET);

  size_t out_len = 0;
  unsigned char base64_buf[TOKEN_BUFF_SIZE];
  mbedtls_base64_encode(base64_buf, sizeof(base64_buf), &out_len,
                        (const unsigned char *)id_secret, strlen(id_secret));

  snprintf(g_auth_state.auth_header, sizeof(g_auth_state), "Basic %s",
           (char *)base64_buf);

  esp_err_t refresh_token_found = spotify_storage_get_refresh_token(&g_auth_state.refresh_token);
  if (refresh_token_found != ESP_OK)
  {
    ESP_LOGW(TAG, "spotify refresh token not found in memory");
    g_auth_state.has_refresh_token = false;
    return;
  }
  g_auth_state.has_refresh_token = true;
}

esp_err_t spotify_auth_get_token(char *out_token, size_t max_len)
{
  if (esp_timer_get_time() + (g_auth_state.expires_at * 1000000))
  {
    ESP_LOGI(TAG, "access token is not expired, passing the cached token");
  }
  else if (g_auth_state.has_refresh_token)
  {
    ESP_LOGI(TAG, "refresh token is available, refreshing the access token");
    esp_err_t res = spotify_auth_refresh_token();

    if (res != ESP_OK)
    {
      ESP_LOGE(TAG, "failed to refresh the access token");
      return res;
    }
    ESP_LOGI(TAG, "access token refreshed successfully");
  }
  else
  {
    ESP_LOGI(TAG, "no refresh token avilable, polling for the auth code");

    char auth_code[TOKEN_BUFF_SIZE];

    if (!spotify_auth_poll_auth_code(auth_code))
    {
      ESP_LOGE(TAG, "failed polling the auth code");
      return ESP_ERR_TIMEOUT;
    }
    ESP_LOGI(TAG, "auth code polled successfully, proceding with token exchange");
    esp_err_t res = spotify_auth_exchange_code(auth_code, sizeof(auth_code));

    if (res != ESP_OK)
    {
      ESP_LOGE(TAG, "failed the token exchange");
      return res;
    }
    ESP_LOGI(TAG, "token exchanged successfully");
  }

  strcpy(out_token, g_auth_state.access_token);
  return ESP_OK;
}

esp_err_t spotify_auth_refresh_token()
{
  return ESP_FAIL;
}

bool spotify_auth_poll_auth_code(char *out_code)
{
  return xQueueCRReceive(g_auth_state.auth_code_queue, out_code, portMAX_DELAY);
}
esp_err_t spotify_auth_exchange_code(char *auth_code, size_t len)
{
  struct spotify_token_response response;

  esp_err_t result = spotify_client_exchange_code(auth_code, g_auth_state.auth_header,
                                                  &response);
    if (result != ESP_OK) {
      return result;
    }


}

// esp_err_t spotify_auth_get_refresh_token(char *out_token) { return ESP_FAIL; }
// esp_err_t spotify_auth_exchange_token(const char *code) { return ESP_FAIL; }

esp_err_t spotify_auth_handle_callback(const char *code)
{
  return xQueueSend(g_auth_state.auth_code_queue, code, pdMS_TO_TICKS(1000));
}