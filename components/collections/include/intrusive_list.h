#pragma once

#include <stddef.h>
#include <stdbool.h>

#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

struct list_node
{
    struct list_node *next;
    struct list_node *prev;
};

struct list
{
    struct list_node head;
};

static inline void list_init(struct list *list)
{
    list->head.next = &list->head;
    list->head.prev = &list->head;
}

static inline bool list_is_empty(const struct list *list)
{
    return list->head.next == &list->head;
}

static inline void list_push_back(struct list *list, struct list_node *node)
{
    node->prev = list->head.prev;
    node->next = &list->head;
    list->head.prev->next = node;
    list->head.prev = node;
}

static inline void list_remove(struct list_node *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = NULL;
    node->prev = NULL;
}