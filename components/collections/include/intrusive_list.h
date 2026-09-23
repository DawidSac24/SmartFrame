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

struct intr_list
{
    struct list_node head;
};

static inline void list_init(struct intr_list *list)
{
    list->head.next = &list->head;
    list->head.prev = &list->head;
}

static inline bool list_is_empty(const struct intr_list *list)
{
    return list->head.next == &list->head;
}

static inline bool list_contains(struct intr_list *list, struct list_node *target)
{
    if (!list || !target)
        return false;

    struct list_node *current = &list->head;
    do
    {
        if (current == target)
        {
            return true;
        }
        current = current->next;
    } while (current != &list->head);

    return false;
}

static inline void list_push_back(struct intr_list *list, struct list_node *node)
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