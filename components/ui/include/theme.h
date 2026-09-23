#pragma once

#include <stdint.h>
#include <stdbool.h>

struct screen;
struct theme;

struct theme_vtable
{
    void (*draw)(struct theme *self, struct screen *parent, float dt_ms);

    // Optional: Allows to draw transition annimations
    uint32_t (*transition_in)(struct theme *self, struct screen *parent);
    uint32_t (*transition_out)(struct theme *self, struct screen *parent);

    // Optional: Allows configuring the theme over CLI (e.g., changing colors)
    bool (*set_param)(struct theme *self, const char *key, const char *value);
};

struct theme
{
    const char *name;
    const struct theme_vtable *vtable;
};