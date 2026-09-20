#pragma once

#include <stddef.h>
#include <string.h>

#define MEM_ALLIGN 8

struct allocator
{
	const struct allocator_vtable *vtable;
};

struct allocator_vtable
{
	void *(*alloc)(struct allocator *self, size_t size);
	void *(*realloc)(struct allocator *self, void *ptr, size_t new_size);
	void (*free)(struct allocator *self, void *ptr);
	void (*reset)(struct allocator *self);
};

static inline void *mem_alloc(struct allocator *self, size_t size)
{
	return self->vtable->alloc(self, size);
}
static inline void *mem_calloc(struct allocator *self, size_t num, size_t size)
{
	size_t total_size;
	if (__builtin_mul_overflow(num, size, &total_size))
		return NULL;

	void *ptr = mem_alloc(self, num * size);
	if (ptr != NULL)
	{
		memset(ptr, 0, total_size);
	}

	return ptr;
}

static inline void *mem_realloc(struct allocator *self, void *ptr,
								size_t new_size)
{
	return self->vtable->realloc(self, ptr, new_size);
}

static inline void mem_free(struct allocator *self, void *ptr)
{
	self->vtable->free(self, ptr);
}

static inline void mem_reset(struct allocator *self)
{
	self->vtable->reset(self);
}
