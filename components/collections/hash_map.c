#include "hash_map.h"

#include <string.h>
#include <stdio.h>

#define FNV_OFFSET_BASIS 0x811C9DC5
#define FNV_PRIME 0x01000193

#define MIN_CAPACITY 16
#define LOAD_FACTOR 0.7
#define EXPEND_MULT 2

struct hm_entry
{
	char *key;
	void *val;
	uint32_t cashed_hash;
};

struct hash_map
{
	struct allocator *allocator;
	struct hm_entry *entries;
	size_t capacity;
	size_t length;
	size_t tombstone_count;
};

static uint32_t hash_key(const char *key);

static const char *hm_set_entry(struct allocator *allocator,
								struct hm_entry *entries, size_t capacity, const char *key, void *value,
								size_t *plength, size_t *ptombstones);
static bool hm_expend(hash_map_t self);

hash_map_t hm_init(struct allocator *allocator, size_t capacity)
{
	hash_map_t self = (hash_map_t)mem_alloc(allocator,
											sizeof(struct hash_map));
	if (self == NULL)
	{
		return NULL;
	}

	if (capacity < MIN_CAPACITY)
	{
		self->capacity = MIN_CAPACITY;
	}
	else
	{
		capacity--;
		uint32_t bit_position = 32 - __builtin_clz(capacity);
		self->capacity = 1 << bit_position;
	}

	self->entries = (struct hm_entry *)mem_calloc(allocator, self->capacity,
												  sizeof(struct hm_entry));
	if (self->entries == NULL)
	{
		mem_free(allocator, self);
		return NULL;
	}

	self->allocator = allocator;
	self->length = 0;
	self->tombstone_count = 0;

	return self;
}
void hm_free(hash_map_t self)
{
	for (size_t i = 0; i < self->capacity; i++)
	{
		if (self->entries[i].key != NULL && self->entries[i].key != (void *)-1)
			mem_free(self->allocator, (void *)self->entries[i].key);
	}

	mem_free(self->allocator, self->entries);
	mem_free(self->allocator, self);
}

void *hm_get(hash_map_t self, const char *key)
{
	uint32_t hash = hash_key(key);
	size_t index = (size_t)(hash & (uint32_t)(self->capacity - 1));

	while (self->entries[index].key != NULL)
	{
		if (self->entries[index].key != (void *)-1 && self->entries[index].cashed_hash == hash && strcmp(key, self->entries[index].key) == 0)
		{
			return self->entries[index].val;
		}

		index = (index + 1) & (self->capacity - 1);
	}
	return NULL;
}
const char *hm_set(hash_map_t self, const char *key, void *value)
{
	if (value == NULL)
	{
		return NULL;
	}

	size_t occupied_slots = self->length + self->tombstone_count;
	if (occupied_slots >= (size_t)(self->capacity * LOAD_FACTOR))
	{
		if (!hm_expend(self))
		{
			return NULL;
		}
	}

	const char *res = hm_set_entry(self->allocator, self->entries,
								   self->capacity, key, value,
								   &self->length, &self->tombstone_count);

	return res;
}

void *hm_pop(hash_map_t self, const char *key)
{
	uint32_t hash = hash_key(key);
	size_t index = (size_t)(hash & (uint32_t)(self->capacity - 1));

	while (self->entries[index].key != NULL)
	{
		if (self->entries[index].key != (void *)-1 &&
			self->entries[index].cashed_hash == hash &&
			strcmp(key, self->entries[index].key) == 0)
		{
			void *val = self->entries[index].val;

			mem_free(self->allocator, (void *)self->entries[index].key);

			self->entries[index].key = (void *)-1;

			self->length--;
			self->tombstone_count++;

			return val;
		}

		index = (index + 1) & (self->capacity - 1);
	}
	return NULL;
}
size_t hm_length(hash_map_t self)
{
	return self->length;
}

static uint32_t hash_key(const char *key)
{
	uint32_t hash = FNV_OFFSET_BASIS;

	for (const char *p = key; *p != '\0'; p++)
	{
		hash = hash ^ (*p);
		hash *= FNV_PRIME;
	}
	return hash;
}

static const char *hm_set_entry(struct allocator *allocator,
								struct hm_entry *entries, size_t capacity,
								const char *key, void *value,
								size_t *plength, size_t *ptombstones)
{
	uint32_t hash = hash_key(key);
	size_t index = (size_t)(hash & (uint32_t)(capacity - 1));

	size_t tombstone = 0;
	bool found_tombstone = false;

	while (entries[index].key != NULL)
	{
		if (entries[index].key != (void *)-1 && entries[index].cashed_hash == hash && strcmp(key, entries[index].key) == 0)
		{
			entries[index].val = value;
			return entries[index].key;
		}
		if (entries[index].key == (void *)-1 && !found_tombstone)
		{
			tombstone = index;
			found_tombstone = true;
		}

		index = (index + 1) & (capacity - 1);
	}

	if (found_tombstone)
	{
		index = tombstone;
		if (ptombstones != NULL)
		{
			(*ptombstones)--;
		}
	}

	if (plength != NULL)
	{
		size_t len = strlen(key) + 1;
		char *new_key = mem_alloc(allocator, len);
		if (new_key == NULL)
			return NULL;

		memcpy(new_key, key, len);
		(*plength)++;
		entries[index].key = new_key;
	}
	else
	{
		entries[index].key = (char *)key;
	}

	entries[index].val = value;
	entries[index].cashed_hash = hash;
	return entries[index].key;
}

static bool hm_expend(hash_map_t self)
{
	size_t new_capacity = self->capacity * EXPEND_MULT;
	if (new_capacity < self->capacity)
		return false;

	struct hm_entry *new_entries = mem_calloc(self->allocator, new_capacity, sizeof(struct hm_entry));
	if (new_entries == NULL)
		return false;

	for (size_t i = 0; i < self->capacity; i++)
	{
		struct hm_entry entry = self->entries[i];

		if (entry.key != NULL && entry.key != (void *)-1)
		{
			hm_set_entry(self->allocator, new_entries, new_capacity, entry.key,
						 entry.val, NULL, NULL);
		}
	}

	mem_free(self->allocator, self->entries);
	self->entries = new_entries;
	self->capacity = new_capacity;
	self->tombstone_count = 0;

	return true;
}
