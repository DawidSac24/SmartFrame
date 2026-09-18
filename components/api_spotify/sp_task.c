#include "api_spotify.h"
#include "sp_auth.h"
#include "sp_client.h"
#include "sp_cmd.h"

#include "esp_log.h"

#include "app_state.h"
#include "wifi.h"

#define POLL_DELAY pdMS_TO_TICKS(3000) // 3s

// private variables
static const char *TAG = "spotify_task";

// private functions
void sp_task(void *pvParameters);

esp_err_t spotify_init(void)
{
  sp_auth_init();
  sp_cmd_register();

  TaskHandle_t task_handle = NULL;
  BaseType_t returned = xTaskCreatePinnedToCore(sp_task, "spotify_task", 8192, NULL, PRIO_SPOTIFY_API, &task_handle, 0);
  if (returned != pdPASS)
  {
    ESP_LOGE(TAG, "Failed to create task!");
    return ESP_FAIL;
  }

  return ESP_OK;
}

void sp_task(void *pvParameters)
{
  struct sp_track_info track_info;
  while (1)
  {
    if (sp_auth_ensure_valid() == ESP_OK)
    {
      char token[256];
      sp_auth_get_token(token, sizeof(token));

      esp_err_t res = sp_client_fetch_track(token, &track_info);
      if (res != ESP_OK)
      {
        ESP_LOGE(TAG, "Failed to fetch track info: %s", esp_err_to_name(res));
      }
      else
      {
        ESP_LOGI(TAG, "Currently playing: '%s' by '%s'", track_info.track_name, track_info.artist_name);
      }
    }

    vTaskDelay(POLL_DELAY); // Delay for 3s
  }
}

esp_err_t spotify_send_auth_code(const char *auth_code)
{
  esp_err_t res = sp_auth_send_code(auth_code);

  if (res == ESP_OK)
    ESP_LOGI(TAG, "Spotify authentification code sent successfully.");
  else
    ESP_LOGE(TAG, "Failed to send authentification code: %s", esp_err_to_name(res));

  return res;
}