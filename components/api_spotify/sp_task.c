#include "api_spotify.h"
#include "sp_manager.h"
#include "sp_auth.h"
#include "sp_cmd.h"

#include "esp_log.h"

#include "app_state.h"
#include "wifi.h"

// private variables
const TickType_t POLL_INTERVAL_MS = pdMS_TO_TICKS(3000); // 3s

static const char *TAG = "spotify_task";
QueueHandle_t sp_cmd_queue = NULL;

// private functions
void sp_task(void *pvParameters);

esp_err_t spotify_init(void)
{
  sp_manager_init();
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

  TickType_t current_delay = 0;

  while (1)
  {
    wait_for_wifi_connection(portMAX_DELAY);

    enum sp_cmd incoming_cmd;
    if (xQueueReceive(sp_cmd_queue, &incoming_cmd, current_delay))
    {
      sp_cmd_dispatch(incoming_cmd);

      if (incoming_cmd == SP_CMD_FETCH_TRACK)
      {
        current_delay = POLL_INTERVAL_MS;
      }
    }
    else
    {
      sp_manager_fetch_and_save_track();

      current_delay = POLL_INTERVAL_MS;
    }
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