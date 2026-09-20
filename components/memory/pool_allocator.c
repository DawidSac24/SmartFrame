#include "pool_allocator.h"

#define MIN_BLOCK_SIZE 4

struct block
{
	struct block *next;
};

static const struct allocator_vtable s_pool_vtable = {.alloc = pool_alloc,
													  .realloc = pool_realloc,
													  .free = pool_free,
													  .reset = pool_reset};

int pool_init(struct pool_allocator *self, uint8_t *buffer, size_t size,
			  size_t block_size)
{
	self->start = buffer;
	self->end = buffer + size;

	if (block_size < sizeof(struct block))
		block_size = sizeof(struct block);
	struct block *block;

	block_size = (block_size + (MEM_ALLIGN - 1)) & ~(MEM_ALLIGN - 1);
	self->next_free_block = (struct block *)self->start;

	size_t segments = (self->end - self->start) / block_size;
	if (segments <= 0)
	{
		self->next_free_block = NULL;
		return 0;
	}

	for (int i = 0; i < segments - 1; i++)
	{
		block = (struct block *)(self->start + (i * block_size));
		block->next = (struct block *)(self->start + ((i + 1) * block_size));
	}

	block = (struct block *)(self->start + ((segments - 1) * block_size));
	block->next = NULL;

	self->base.vtable = &s_pool_vtable;
	self->free_count = segments;
	self->block_size = block_size;
	self->total_blocks = segments;

	return segments;
}
void pool_reset(struct allocator *base)
{
	struct pool_allocator *self = (struct pool_allocator *)base;

	self->next_free_block = (struct block *)self->start;
	self->free_count = self->total_blocks;

	struct block *current = self->next_free_block;

	for (size_t i = 0; i < self->total_blocks - 1; i++)
	{
		struct block *next = (struct block *)((uint8_t *)current + self->block_size);
		current->next = next;
		current = next;
	}
	current->next = NULL;
}

void *pool_alloc(struct allocator *base, size_t size)
{
	struct pool_allocator *self = (struct pool_allocator *)base;
	if (size > self->block_size)
		return NULL;

	struct block *block = self->next_free_block;
	if (!block)
		return NULL;

	self->next_free_block = block->next;
	self->free_count--;
	return block;
}
void *pool_realloc(struct allocator *base, void *ptr, size_t new_size)
{
	return NULL;
}
void pool_free(struct allocator *base, void *ptr)
{
	struct pool_allocator *self = (struct pool_allocator *)base;

	if ((uint8_t *)ptr >= self->start && (uint8_t *)ptr < self->end)
	{
		struct block *block = (struct block *)ptr;
		block->next = self->next_free_block;
		self->next_free_block = block;
		self->free_count++;
	}
}
