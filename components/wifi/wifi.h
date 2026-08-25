#pragma once

#include <stdint.h>
#include <stdbool.h>

void wifi_init(void);

bool is_wifi_connected(void);
void wait_for_wifi_connection(uint32_t timeout);
