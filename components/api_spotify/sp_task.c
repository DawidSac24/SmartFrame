#include "api_spotify.h"
#include "sp_manager.h"
#include "sp_auth.h"

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
  sp_manager_init();
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
  while (1)
  {
    sp_manager_fetch_and_save_track();
    vTaskDelay(POLL_DELAY); // Delay for 3s
  }
}

esp_err_t spotify_send_auth_code(const char *auth_code)
{
  esp_err_t res = sp_auth_set_auth_code(auth_code);

  if (res == ESP_OK)
    ESP_LOGI(TAG, "Spotify authentification code sent successfully.");
  else
    ESP_LOGE(TAG, "Failed to send authentification code: %s", esp_err_to_name(res));

  return res;
}