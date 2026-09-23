#pragma once
#include <stdint.h>
#include "screen.h"

void screens_init_defaults(void);

struct screen *screens_create_by_name(const char *name, uint32_t duration_ms);