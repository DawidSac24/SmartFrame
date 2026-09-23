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

#include "screens_init.h"
#include "ui_task.h"
#include "ui_cmd.h"
#include "wifi.h"
#include "storage.h"
#include "web_server.h"
#include "api_weather.h"
#include "cli.h"
#include "secrets.h"
#include "api_spotify.h"

enum task_priority_level
{
  PRIO_IDLE = 0,
  PRIO_LOW = 1,
  PRIO_WEB_SERVER = 2,
  PRIO_WEATHER_API = 3,
  PRIO_SPOTIFY_API = 4,
  PRIO_MATRIX_RENDER = 5,
  PRIO_BUTTON_POLL = 6
};

void app_main(void)
{
  esp_log_level_set("esp-x509-crt-bundle", ESP_LOG_WARN);
  esp_err_t ret = storage_init();
  ESP_ERROR_CHECK(ret);

  ui_task_init(4096, PRIO_MATRIX_RENDER);
  ui_cmd_set_factory(screens_create_by_name);
  screens_init_defaults();
  struct screen *sp_scr = screens_create_by_name("spotify", 15000);
  ui_task_set_spotify_screen(sp_scr);

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

  weather_init(LONGITUDE, LATITUDE, 6144, PRIO_WEATHER_API);
  spotify_init(8192, PRIO_SPOTIFY_API);
}