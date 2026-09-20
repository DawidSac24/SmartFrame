#include "ring_buffer.h"

#include <stdint.h>

#define MIN_CAPACITY 4

struct ring_buffer
{
	struct allocator *allocator;
	void **data;
	size_t head;
	size_t tail;
	size_t capacity;
	size_t len;
};

ring_buffer_t rbuf_init(struct allocator *allocator, size_t capacity)
{
	ring_buffer_t self = (ring_buffer_t)mem_alloc(allocator,
												  sizeof(struct ring_buffer));
	if (self == NULL)
	{
		return NULL;
	}

	if (capacity < MIN_CAPACITY)
		capacity = MIN_CAPACITY;
	capacity--;
	uint32_t bit_position = 32 - __builtin_clz(capacity);
	self->capacity = 1 << bit_position;

	self->data = mem_alloc(allocator, sizeof(void *) * self->capacity);
	if (self->data == NULL)
	{
		mem_free(allocator, self);
		return NULL;
	}

	self->allocator = allocator;
	self->head = 0;
	self->tail = 0;
	self->len = 0;

	return self;
}
void rbuf_free(ring_buffer_t self)
{
	if (!self)
		return;
	if (self->data)
	{
		mem_free(self->allocator, self->data);
	}
	mem_free(self->allocator, self);
}

bool rbuf_is_empty(ring_buffer_t self)
{
	return self->len == 0;
}
bool rbuf_is_full(ring_buffer_t self)
{
	return self->len == self->capacity;
}

void rbuf_push(ring_buffer_t self, void *data)
{
	if (rbuf_is_full(self))
		self->tail = (self->tail + 1) & (self->capacity - 1);
	else
		self->len++;

	self->data[self->head] = data;
	self->head = (self->head + 1) & (self->capacity - 1);
}
void *rbuf_pop(ring_buffer_t self)
{
	if (rbuf_is_empty(self))
		return NULL;
	void *val = self->data[self->tail];
	self->tail = (self->tail + 1) & (self->capacity - 1);
	self->len--;
	return val;
}

void rbuf_reset(ring_buffer_t self)
{
	self->head = 0;
	self->tail = 0;
	self->len = 0;
}
