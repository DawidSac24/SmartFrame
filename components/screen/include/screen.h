#pragma once

#include "intrusive_list.h"

#include <stdint.h>

struct screen
{
    const struct screen_vtable *vtable;
    struct list_node node;
    uint32_t display_duration_ms;
};

struct screen_vtable
{
    void (*init)(struct screen *self);
    void (*prepare)(struct screen *self);
    void (*draw)(struct screen *self, float dt_ms);
    void (*transition_in)(struct screen *self);
    void (*transition_out)(struct screen *self);
    void (*destroy)(struct screen *self);
};