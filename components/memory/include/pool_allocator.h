#pragma once

#include "allocator.h"

#include <stdio.h>
#include <stdint.h>

struct pool_allocator
{
	struct allocator base;
	uint8_t *start;
	uint8_t *end;

	void *next_free_block;
	size_t free_count;
	size_t block_size;
	size_t total_blocks;
};

int pool_init(struct pool_allocator *self, uint8_t *buffer, size_t size,
			  size_t block_size);
void pool_reset(struct allocator *base);

void *pool_alloc(struct allocator *base, size_t size);
void *pool_realloc(struct allocator *base, void *ptr, size_t new_size);
void pool_free(struct allocator *base, void *ptr);
