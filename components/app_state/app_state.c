#include "app_state.h"

struct app_state global_state;
SemaphoreHandle_t global_state_mutex;

void app_state_init(void)
{
    global_state_mutex = xSemaphoreCreateMutex();

    global_state.current_mode = 0;

    // WEATHER
    global_state.weather.id = 0;
    global_state.weather.temperature = 0.0f;
    global_state.weather.humidity = 0.0f;
    global_state.weather.pressure = 0.0f;
    global_state.weather.icon[0] = '\0';
    global_state.weather.main[0] = '\0';
    global_state.weather.description[0] = '\0';
    global_state.weather.last_fetched = 0;

    // SPOTIFY


    // TODO
}
