#pragma once

#include "intrusive_list.h"
#include "theme.h"

#include <stdint.h>

struct screen
{
    const struct screen_vtable *vtable;
    struct list_node node;
    const struct theme *active_theme;
    const char *name;
    uint32_t display_duration_ms;
};

struct screen_vtable
{
    void (*init)(struct screen *self);
    void (*prepare)(struct screen *self);
    void (*draw)(struct screen *self, float dt_ms);
    uint32_t (*transition_in)(struct screen *self);
    uint32_t (*transition_out)(struct screen *self);
    void (*destroy)(struct screen *self);
    const struct theme *(*get_theme)(const char *name);
    bool (*set_param)(struct screen *self, const char *key, const char *val);
};