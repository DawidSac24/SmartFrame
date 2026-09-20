#pragma once

#include "pool_allocator.h"

#include <stdbool.h>

struct slab_allocator
{
	struct allocator base;

	struct pool_allocator pool_16;
	struct pool_allocator pool_64;
	struct pool_allocator pool_256;
};

bool slab_init(struct slab_allocator *self, uint8_t *buffer,
			   size_t bytes_for_16, size_t bytes_for_64, size_t bytes_for_256);
void slab_reset(struct allocator *base);

void *slab_alloc(struct allocator *base, size_t size);
void *slab_realloc(struct allocator *base, void *ptr, size_t new_size);
void slab_free(struct allocator *base, void *ptr);
