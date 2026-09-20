#include "arena_allocator.h"

static const struct allocator_vtable s_arena_vtable = {.alloc = arena_alloc,
													   .realloc = arena_realloc,
													   .free = arena_free,
													   .reset = arena_reset};

void arena_init(struct arena_allocator *self, uint8_t *buffer, size_t size)
{
	self->start = buffer;
	self->end = buffer + size;
	self->pos = buffer;

	self->base.vtable = &s_arena_vtable;
}

void arena_reset(struct allocator *base)
{
	struct arena_allocator *self = (struct arena_allocator *)base;

	self->pos = self->start;
}

void *arena_alloc(struct allocator *base, size_t size)
{
	struct arena_allocator *self = (struct arena_allocator *)base;

	uint8_t *const last_pos = self->pos;
	size = (size + MEM_ALLIGN - 1) & ~(MEM_ALLIGN - 1);

	if (last_pos + size > self->end || last_pos + size < self->start)
	{
		return NULL;
	}

	self->pos += size;
	return last_pos;
}
void *arena_realloc(struct allocator *base, void *ptr, size_t new_size)
{
	return NULL;
}
void arena_free(struct allocator *base, void *ptr)
{
	return;
}
