#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>

#define TOKEN_BUFFER_SIZE 512

enum task_priority_level
{
    PRIO_IDLE,
    PRIO_DEFAULT,
    PRIO_WEATHER_API,
    PRIO_SPOTIFY_API,
    PRIO_MATRIX_RENDER,
    PRIO_WEB_SERVER,
    PRIO_BUTTON_POLL
};

struct weather_data
{
    int id;

    float temperature;
    float humidity;
    float pressure;

    char icon[4];
    char main[16];
    char description[64];
    time_t last_fetched;

    bool is_dirty;
};

struct spotify_data
{
    char refresh_token[TOKEN_BUFFER_SIZE];
    char access_token[TOKEN_BUFFER_SIZE];
    time_t last_token_fetch;
};

enum app_mode
{
    APP_TIME,
    APP_WEATHER,
    APP_SPOTIFY
    // TODO
};

struct app_state
{
    enum app_mode current_mode;
    struct weather_data weather;
    struct spotify_data spotify;
    // TODO
};

extern struct app_state global_state;
extern SemaphoreHandle_t global_state_mutex;

void app_state_init(void);
