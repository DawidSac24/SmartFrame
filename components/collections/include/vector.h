#pragma once
#include "allocator.h"

struct ptr_vector
{
	struct allocator *allocator;
	void *data;
	size_t elem_size;
	size_t capacity;
	size_t len;
};
