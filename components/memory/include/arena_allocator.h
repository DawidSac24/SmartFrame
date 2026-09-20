#pragma once

#include "allocator.h"

#include <stdio.h>
#include <stdint.h>

struct arena_allocator
{
	struct allocator base;
	uint8_t *start;
	uint8_t *end;
	uint8_t *pos;
};

void arena_init(struct arena_allocator *arena, uint8_t *buffer, size_t size);
void arena_reset(struct allocator *self);

void *arena_alloc(struct allocator *self, size_t size);
void *arena_realloc(struct allocator *self, void *ptr, size_t new_size);
void arena_free(struct allocator *self, void *ptr);
