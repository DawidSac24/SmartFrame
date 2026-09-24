#pragma once
#include "api_weather.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// State
esp_err_t wea_state_init(void);
esp_err_t wea_state_set(const struct weather_dto *new_data);
void weather_set_override(const struct weather_dto *forced_dto); // <--- THIS WAS MISSING!

// Client & Parse
esp_err_t wea_client_fetch_weather(float lat, float lon, char **out_json);
esp_err_t wea_parse(const char *json_str, struct weather_dto *out_dto);
esp_err_t wea_client_download_img(const char *icon_code, const char *dest_path); // <-- New

// Manager Orchestration
esp_err_t wea_manager_init(void);
esp_err_t wea_manager_fetch_and_save_weather(float lat, float lon);

// Commands
enum wea_cmd
{
    WEA_CMD_FETCH,
    WEA_CMD_PRINT,
    WEA_CMD_TEST,
    WEA_CMD_UNKNOWN
};

extern QueueHandle_t wea_cmd_queue;
void wea_cmd_register(void);
esp_err_t wea_cmd_dispatch(enum wea_cmd cmd);