#include "sp_auth.h"
#include "sp_client.h"
#include "sp_storage.h"

#include "secrets.h"

#include "esp_log.h"
#include <mbedtls/base64.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"

static const char *TAG = "spotify_auth";

static struct sp_auth_state
{
  char access_token[DEFAULT_BUFF_SIZE];
  char refresh_token[DEFAULT_BUFF_SIZE];
  char auth_header[DEFAULT_BUFF_SIZE];
  int64_t expires_at;
  bool has_refresh_token;
  QueueHandle_t auth_code_queue;
} g_auth_state;

// --- Private Internal Functions ---
static esp_err_t refresh_token_internal(void);
static bool poll_auth_code_internal(char *out_code);
static esp_err_t exchange_code_internal(char *auth_code, size_t len);

void sp_auth_init(void)
{
  g_auth_state.access_token[0] = '\0';
  g_auth_state.refresh_token[0] = '\0';
  g_auth_state.expires_at = 0;
  g_auth_state.has_refresh_token = false;
  g_auth_state.auth_code_queue = xQueueCreate(1, DEFAULT_BUFF_SIZE * sizeof(char));

  // Cache Base64 Basic Auth header
  char id_secret[DEFAULT_BUFF_SIZE];
  snprintf(id_secret, sizeof(id_secret), "%s:%s", SPOTIFY_CLIENT_ID, SPOTIFY_CLIENT_SECRET);

  size_t out_len = 0;
  unsigned char base64_buf[DEFAULT_BUFF_SIZE - 10]; // -10 to account for "Basic " prefix and null terminator
  mbedtls_base64_encode(base64_buf, sizeof(base64_buf), &out_len,
                        (const unsigned char *)id_secret, strlen(id_secret));

  snprintf(g_auth_state.auth_header, sizeof(g_auth_state.auth_header), "Basic %s", (char *)base64_buf);

  if (sp_storage_get_refresh_token(g_auth_state.refresh_token, DEFAULT_BUFF_SIZE) != ESP_OK)
  {
    ESP_LOGW(TAG, "No refresh token found in memory");
  }
  else
  {
    g_auth_state.has_refresh_token = true;
  }
}

esp_err_t sp_auth_is_valid(void)
{
  if (strlen(g_auth_state.access_token) > 0 && esp_timer_get_time() < g_auth_state.expires_at)
  {
    return ESP_OK;
  }
  return ESP_ERR_INVALID_STATE;
}

esp_err_t sp_auth_ensure_valid(void)
{
  if (sp_auth_is_valid() == ESP_OK)
  {
    return ESP_OK;
  }

  if (g_auth_state.has_refresh_token)
  {
    ESP_LOGI(TAG, "Refreshing access token...");
    return refresh_token_internal();
  }

  ESP_LOGI(TAG, "Cold boot: Polling for auth code...");
  char auth_code[DEFAULT_BUFF_SIZE];
  if (!poll_auth_code_internal(auth_code))
    return ESP_ERR_TIMEOUT;

  return exchange_code_internal(auth_code, sizeof(auth_code));
}

esp_err_t sp_auth_get_token(char *out_token, size_t max_len)
{
  if (strlen(g_auth_state.access_token) == 0)
    return ESP_ERR_INVALID_STATE;

  strncpy(out_token, g_auth_state.access_token, max_len - 1);
  out_token[max_len - 1] = '\0';
  return ESP_OK;
}

esp_err_t sp_auth_get_token_state(struct sp_auth_token_state *token_state)

{
  if (!g_auth_state.has_refresh_token)
    return ESP_ERR_INVALID_STATE;

  strcpy(g_auth_state.access_token, token_state->access_token);
  strcpy(g_auth_state.refresh_token, token_state->refresh_token);
  token_state->expires_at = g_auth_state.expires_at;
  return ESP_OK;
}

esp_err_t sp_auth_send_code(const char *code)
{
  return xQueueSend(g_auth_state.auth_code_queue, code, pdMS_TO_TICKS(1000));
}

// --- Private Internal Functions ---
static esp_err_t refresh_token_internal(void)
{
  struct sp_token_response response;
  esp_err_t res = sp_client_refresh_token(g_auth_state.refresh_token, g_auth_state.auth_header, &response);
  if (res != ESP_OK)
    return res;

  strcpy(g_auth_state.access_token, response.access_token);
  g_auth_state.expires_at = esp_timer_get_time() + ((int64_t)response.expires_in_sec * 1000000ULL);

  if (response.has_new_refresh_token)
  {
    strcpy(g_auth_state.refresh_token, response.refresh_token);
    g_auth_state.has_refresh_token = true;
    sp_storage_set_refresh_token(g_auth_state.refresh_token);
  }
  return ESP_OK;
}

static esp_err_t exchange_code_internal(char *auth_code, size_t len)
{
  struct sp_token_response response;
  esp_err_t res = sp_client_exchange_code(auth_code, g_auth_state.auth_header, &response);

  if (res != ESP_OK)
    return res;

  strcpy(g_auth_state.access_token, response.access_token);
  strcpy(g_auth_state.refresh_token, response.refresh_token);

  g_auth_state.expires_at = esp_timer_get_time() + ((int64_t)response.expires_in_sec * 1000000ULL);
  g_auth_state.has_refresh_token = true;
  sp_storage_set_refresh_token(g_auth_state.refresh_token);

  return ESP_OK;
}

static bool poll_auth_code_internal(char *out_code)
{
  return xQueueReceive(g_auth_state.auth_code_queue, out_code, portMAX_DELAY);
}