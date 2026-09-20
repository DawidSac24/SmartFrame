#pragma once
#include "allocator.h"

#include <stdint.h>
#include <stdbool.h>

struct hash_map;
typedef struct hash_map *hash_map_t;

hash_map_t hm_init(struct allocator *allocator, size_t capacity);
void hm_free(hash_map_t self);

void *hm_get(hash_map_t self, const char *key);
const char *hm_set(hash_map_t self, const char *key, void *value);
void *hm_pop(hash_map_t self, const char *key);

size_t hm_length(hash_map_t self);
