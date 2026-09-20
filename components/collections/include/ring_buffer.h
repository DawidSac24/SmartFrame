#pragma once
#include "allocator.h"

#include <stddef.h>
#include <stdbool.h>

struct ring_buffer;
typedef struct ring_buffer *ring_buffer_t;

ring_buffer_t rbuf_init(struct allocator *allocator, size_t capacity);
void rbuf_free(ring_buffer_t self);

bool rbuf_is_empty(ring_buffer_t self);
bool rbuf_is_full(ring_buffer_t self);

void rbuf_push(ring_buffer_t self, void *data);
void *rbuf_pop(ring_buffer_t self);

void rbuf_reset(ring_buffer_t self);
