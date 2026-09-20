#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

enum sp_cmd
{
    SP_CMD_FETCH_TRACK = 0,
    SP_CMD_PRINT,
    SP_CMD_CLEAR,
    SP_CMD_UNKNOWN
};

extern QueueHandle_t sp_cmd_queue;

void sp_cmd_register(void);

void sp_cmd_send(enum sp_cmd cmd);
esp_err_t sp_cmd_dispatch(enum sp_cmd cmd);

enum sp_cmd sp_str_to_cmd(const char *str);