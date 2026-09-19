/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  https://RandomNerdTutorials.com/esp-idf-esp32-web-server/
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sntp.h"

#include "wifi.h"
#include "storage.h"
#include "web_server.h"
#include "app_state.h"
#include "api_weather.h"
#include "cli.h"
#include "secrets.h"
#include "display_hal.h"
#include "api_spotify.h"

void app_main(void)
{
  esp_err_t ret = storage_init();
  ESP_ERROR_CHECK(ret);

  display_hal_init();

  app_state_init();
  cli_init();

  wifi_init();

  ESP_LOGI("main", "Waiting for Wi-Fi connection before starting network services...");
  wait_for_wifi_connection(portMAX_DELAY);
  ESP_LOGI("main", "Wi-Fi Connected! Booting network services...");

  start_web_server();

  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org");
  esp_sntp_init();

  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();

  // api_weather_start_task(LONGITUDE, LATITUDE);
  spotify_init();
}