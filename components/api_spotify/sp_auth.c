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

struct token
{
  char buff[DEFAULT_BUFF_SIZE];
  bool is_empty;
};

static struct sp_auth_state
{
  struct token access_token;
  struct token refresh_token;
  char auth_header[DEFAULT_BUFF_SIZE];
  int64_t access_tok_expires_at;
  QueueHandle_t auth_code_queue;
} g_auth_state;

// --- Private Internal Functions ---
// static esp_err_t refresh_token_internal(void);
// static bool poll_auth_code_internal(char *out_code);
// static esp_err_t exchange_code_internal(char *auth_code, size_t len);

void sp_auth_init(void)
{
  g_auth_state.access_token.buff[0] = '\0';
  g_auth_state.access_token.is_empty = true;
  g_auth_state.refresh_token.buff[0] = '\0';
  g_auth_state.refresh_token.is_empty = true;
  g_auth_state.access_tok_expires_at = 0;
  g_auth_state.auth_code_queue = xQueueCreate(1, DEFAULT_AUTH_CODE_BUFF_SIZE * sizeof(char));

  // Cache Base64 Basic Auth header
  char id_secret[DEFAULT_BUFF_SIZE];
  snprintf(id_secret, sizeof(id_secret), "%s:%s", SPOTIFY_CLIENT_ID, SPOTIFY_CLIENT_SECRET);

  size_t out_len = 0;
  unsigned char base64_buf[DEFAULT_BUFF_SIZE - 10]; // -10 to account for "Basic " prefix and null terminator
  mbedtls_base64_encode(base64_buf, sizeof(base64_buf), &out_len,
                        (const unsigned char *)id_secret, strlen(id_secret));
  base64_buf[out_len] = '\0';

  snprintf(g_auth_state.auth_header, sizeof(g_auth_state.auth_header), "Basic %s", (char *)base64_buf);

  if (sp_storage_get_refresh_token(g_auth_state.refresh_token.buff, DEFAULT_BUFF_SIZE) == ESP_OK)
  {
    if (strlen(g_auth_state.refresh_token.buff) > 50)
    {
      g_auth_state.refresh_token.is_empty = false;
    }
    else
    {
      ESP_LOGW(TAG, "Saved refresh token is corrupted or empty. Ignoring.");
    }
  }
}

bool sp_auth_is_access_expired()
{
  return esp_timer_get_time() >= g_auth_state.access_tok_expires_at;
}

esp_err_t sp_auth_get_access_token(char *out_buffer, size_t max_len)
{
  if (out_buffer == NULL)
    return ESP_ERR_INVALID_ARG;

  if (g_auth_state.access_token.is_empty)
    return ESP_ERR_INVALID_STATE;

  if (sp_auth_is_access_expired())
    return ESP_ERR_TIMEOUT;

  strncpy(out_buffer, g_auth_state.access_token.buff, max_len - 1);
  out_buffer[max_len - 1] = '\0';
  return ESP_OK;
}

esp_err_t sp_auth_get_refresh_token(char *out_buffer, size_t max_len)
{
  if (out_buffer == NULL)
    return ESP_ERR_INVALID_ARG;

  if (g_auth_state.refresh_token.is_empty)
  {
    return ESP_ERR_INVALID_STATE;
  }

  strncpy(out_buffer, g_auth_state.refresh_token.buff, max_len - 1);
  out_buffer[max_len - 1] = '\0';
  return ESP_OK;
}

esp_err_t sp_auth_get_auth_code(char *out_code)
{
  if (xQueueReceive(g_auth_state.auth_code_queue, out_code, portMAX_DELAY) == pdPASS)
  {
    return ESP_OK;
  }
  return ESP_FAIL;
}

void sp_auth_get_header(char *out_buff, size_t max_len)
{
  strncpy(out_buff, g_auth_state.auth_header, max_len - 1);
  out_buff[max_len - 1] = '\0';
}

void sp_auth_set_access_token(const char *buff, int expires_in_sec)
{
  strncpy(g_auth_state.access_token.buff, buff, sizeof(g_auth_state.access_token.buff));
  g_auth_state.access_token.is_empty = false;

  g_auth_state.access_tok_expires_at = esp_timer_get_time() + ((uint64_t)expires_in_sec * 1000000ULL);
}

void sp_auth_set_refresh_token(const char *buff)
{
  strncpy(g_auth_state.refresh_token.buff, buff, sizeof(g_auth_state.refresh_token.buff));
  g_auth_state.refresh_token.is_empty = false;
}

esp_err_t sp_auth_set_auth_code(const char *code)
{
  char temp_buff[DEFAULT_AUTH_CODE_BUFF_SIZE] = {0};
  strncpy(temp_buff, code, sizeof(temp_buff) - 1);

  if (xQueueSend(g_auth_state.auth_code_queue, temp_buff, pdMS_TO_TICKS(1000)) == pdPASS)
  {
    return ESP_OK;
  }
  return ESP_FAIL;
}

// esp_err_t sp_auth_is_valid(void)
// {
//   if (g_auth_state.access_token.is_empty || esp_timer_get_time() > g_auth_state.access_tok_expires_at)
//     return ESP_ERR_INVALID_STATE;

//   return ESP_OK;
// }

// esp_err_t sp_auth_ensure_valid(void)
// {
//   if (sp_auth_is_valid() == ESP_OK)
//   {
//     return ESP_OK;
//   }

//   if (g_auth_state.refresh_token.is_empty)
//   {
//     ESP_LOGI(TAG, "Refreshing access token...");
//     return refresh_token_internal();
//   }

//   ESP_LOGI(TAG, "Cold boot: Polling for auth code...");
//   char auth_code[DEFAULT_BUFF_SIZE];
//   if (!poll_auth_code_internal(auth_code))
//     return ESP_ERR_TIMEOUT;

//   return exchange_code_internal(auth_code, sizeof(auth_code));
// }

// esp_err_t sp_auth_get_token(char *out_token, size_t max_len)
// {
//   if (g_auth_state.access_token.is_empty)
//     return ESP_ERR_INVALID_STATE;

//   strncpy(out_token, g_auth_state.access_token.buff, max_len - 1);
//   out_token[max_len - 1] = '\0';
//   return ESP_OK;
// }

// esp_err_t sp_auth_get_token_state(struct sp_auth_token_state *token_state)

// {
//   if (g_auth_state.refresh_token.is_empty)
//     return ESP_ERR_INVALID_STATE;

//   strcpy(g_auth_state.access_token.buff, token_state->access_token);
//   strcpy(g_auth_state.refresh_token.buff, token_state->refresh_token);
//   token_state->expires_at = g_auth_state.access_tok_expires_at;
//   return ESP_OK;
// }

// // --- Private Internal Functions ---
// static esp_err_t refresh_token_internal(void)
// {
//   struct sp_token_response response;
//   esp_err_t res = sp_client_refresh_token(g_auth_state.refresh_token.buff, g_auth_state.auth_header, &response);
//   if (res != ESP_OK)
//     return res;

//   strcpy(g_auth_state.access_token.buff, response.access_token);
//   g_auth_state.access_tok_expires_at = esp_timer_get_time() + ((int64_t)response.expires_in_sec * 1000000ULL);

//   if (response.has_new_refresh_token)
//   {
//     strcpy(g_auth_state.refresh_token.buff, response.refresh_token);
//     sp_storage_set_refresh_token(g_auth_state.refresh_token.buff);
//   }
//   return ESP_OK;
// }

// static esp_err_t exchange_code_internal(char *auth_code, size_t len)
// {
//   struct sp_token_response response;
//   esp_err_t res = sp_client_exchange_code(auth_code, g_auth_state.auth_header, &response);

//   if (res != ESP_OK)
//     return res;

//   strcpy(g_auth_state.access_token.buff, response.access_token);
//   strcpy(g_auth_state.refresh_token.buff, response.refresh_token);

//   g_auth_state.access_tok_expires_at = esp_timer_get_time() + ((int64_t)response.expires_in_sec * 1000000ULL);
//   sp_storage_set_refresh_token(g_auth_state.refresh_token.buff);

//   return ESP_OK;
// }
