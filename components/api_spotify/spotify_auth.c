#include "spotify_prv.h"

#include "secrets.h"

#include "esp_log.h"
#include <mbedtls/base64.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"

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

  esp_err_t refresh_token_found = spotify_storage_get_refresh_token(g_auth_state.refresh_token, sizeof(char) * TOKEN_BUFF_SIZE);
  if (refresh_token_found != ESP_OK)
  {
    ESP_LOGW(TAG, "spotify refresh token not found in memory");
    g_auth_state.has_refresh_token = false;
    return;
  }
  g_auth_state.has_refresh_token = true;
}

esp_err_t spotify_auth_fetch_token()
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
      return res;
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
      return res;
  }
  return ESP_OK;
}

esp_err_t spotify_auth_refresh_token()
{
  struct spotify_token_response response;

  esp_err_t res = spotify_client_refresh_token(g_auth_state.refresh_token,
                                               g_auth_state.auth_header, &response);

  if (res != ESP_OK)
    return res;

  strcpy(response.access_token, g_auth_state.access_token);
  g_auth_state.expires_at = response.expires_in_sec;

  if (response.has_new_refresh_token)
  {
    strcpy(response.refresh_token, g_auth_state.refresh_token);
    g_auth_state.has_refresh_token = true;
    spotify_storage_set_refresh_token(response.refresh_token);
  }

  return ESP_OK;
}

esp_err_t spotify_auth_exchange_code(char *auth_code, size_t len)
{
  struct spotify_token_response response;

  esp_err_t res = spotify_client_exchange_code(auth_code, g_auth_state.auth_header,
                                               &response);

  if (res == ESP_OK)
    return res;

  strcpy(response.access_token, g_auth_state.access_token);
  strcpy(response.refresh_token, g_auth_state.refresh_token);
  g_auth_state.expires_at = response.expires_in_sec;
  g_auth_state.has_refresh_token = true;
  spotify_storage_set_refresh_token(response.refresh_token);

  return ESP_OK;
}

esp_err_t spotify_auth_get_token_state(struct spotify_auth_token_state *token_state)
{
  if (!g_auth_state.has_refresh_token)
    return ESP_ERR_INVALID_STATE;

  strcpy(g_auth_state.access_token, token_state->access_token);
  strcpy(g_auth_state.refresh_token, token_state->refresh_token);
  token_state->expires_at = g_auth_state.expires_at;

  return ESP_OK;
}

esp_err_t spotify_auth_send_code(const char *auth_code)
{
  return xQueueSend(g_auth_state.auth_code_queue, auth_code, 0);
}

bool spotify_auth_poll_auth_code(char *out_code)
{
  return xQueueReceive(g_auth_state.auth_code_queue, out_code, portMAX_DELAY);
}

esp_err_t spotify_auth_handle_callback(const char *code)
{
  return xQueueSend(g_auth_state.auth_code_queue, code, pdMS_TO_TICKS(1000));
}