#include "slab_allocator.h"

#include <string.h>

static const struct allocator_vtable s_slab_vtable = {.alloc = slab_alloc,
													  .realloc = slab_realloc,
													  .free = slab_free,
													  .reset = slab_reset};

bool slab_init(struct slab_allocator *self, uint8_t *buffer,
			   size_t bytes_for_16, size_t bytes_for_64, size_t bytes_for_256)
{

	int blocks_16 = pool_init(&self->pool_16, buffer, bytes_for_16, 16);
	if (blocks_16 == 0)
		return false;

	buffer += bytes_for_16;

	int blocks_64 = pool_init(&self->pool_64, buffer, bytes_for_64, 64);
	if (blocks_64 == 0)
		return false;

	buffer += bytes_for_64;

	int blocks_256 = pool_init(&self->pool_256, buffer, bytes_for_256, 256);
	if (blocks_256 == 0)
		return false;

	self->base.vtable = &s_slab_vtable;

	return true;
}
void slab_reset(struct allocator *base)
{
	struct slab_allocator *self = (struct slab_allocator *)base;

	pool_reset((struct allocator *)&self->pool_16);
	pool_reset((struct allocator *)&self->pool_64);
	pool_reset((struct allocator *)&self->pool_256);
}

void *slab_alloc(struct allocator *base, size_t size)
{
	struct slab_allocator *self = (struct slab_allocator *)base;

	if (size <= 16)
	{
		return pool_alloc(&self->pool_16.base, size);
	}
	else if (size <= 64)
	{
		return pool_alloc(&self->pool_64.base, size);
	}
	else if (size <= 256)
	{
		return pool_alloc(&self->pool_256.base, size);
	}

	return NULL;
}
void *slab_realloc(struct allocator *base, void *ptr, size_t new_size)
{
	if (ptr == NULL)
	{
		return slab_alloc(base, new_size);
	}

	if (new_size == 0)
	{
		slab_free(base, ptr);
		return NULL;
	}

	struct slab_allocator *self = (struct slab_allocator *)base;
	uint8_t *p = (uint8_t *)ptr;
	size_t old_size = 0;

	if (p >= self->pool_16.start && p < self->pool_16.end)
	{
		old_size = 16;
	}
	else if (p >= self->pool_64.start && p < self->pool_64.end)
	{
		old_size = 64;
	}
	else if (p >= self->pool_256.start && p < self->pool_256.end)
	{
		old_size = 256;
	}
	else
	{
		return NULL;
	}

	if (new_size <= old_size)
	{
		return ptr;
	}

	void *new_ptr = slab_alloc(base, new_size);
	if (!new_ptr)
	{
		return NULL;
	}

	memcpy(new_ptr, ptr, old_size);

	slab_free(base, ptr);

	return new_ptr;
}
void slab_free(struct allocator *base, void *ptr)
{
	struct slab_allocator *self = (struct slab_allocator *)base;
	uint8_t *p = (uint8_t *)ptr;

	if (p >= self->pool_16.start && p < self->pool_16.end)
	{
		pool_free(&self->pool_16.base, ptr);
	}
	else if (p >= self->pool_64.start && p < self->pool_64.end)
	{
		pool_free(&self->pool_64.base, ptr);
	}
	else if (p >= self->pool_256.start && p < self->pool_256.end)
	{
		pool_free(&self->pool_256.base, ptr);
	}
}
