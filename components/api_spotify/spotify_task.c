#include "api_spotify.h"
#include "spotify_prv.h"

#include "esp_log.h"

#include "app_state.h"
#include "wifi.h"

#define POLL_DELAY pdMS_TO_TICKS(3000) // 3s

// private variables
static const char *TAG = "spotify_task";

// private functions
void api_spotify_task(void *pvParameters);

TaskHandle_t api_spotify_start_task(void)
{
  spotify_auth_init();

  return ESP_OK;
}

void api_spotify_delete_task(TaskHandle_t task_handle)
{
  if (task_handle != NULL)
  {
    vTaskDelete(task_handle);
  }
}

void api_spotify_task(void *pvParameters)
{
  static char access_token[TOKEN_BUFF_SIZE];
  while (1)
  {
    spotify_auth_get_token(access_token, sizeof(access_token));

    ESP_LOGW(TAG, "Spotify task free stack: %d bytes", uxTaskGetStackHighWaterMark(NULL));
    vTaskDelay(POLL_DELAY); // Delay for 3s
  }
}

esp_err_t api_spotify_send_auth_code(const char *auth_code)
{
  esp_err_t res = spotify_auth_send_code(auth_code);

  if (res == ESP_OK)
    ESP_LOGI(TAG, "Spotify authentification code sent successfully.");
  else
    ESP_LOGE(TAG, "Failed to send authentification code: %s", esp_err_to_name(res));

  return res;
}