#pragma once

#include <stddef.h>
#include <string.h>
#include <stdbool.h>

struct string_view
{
    const char *data;
    size_t length;
};

static inline struct string_view sv_from_cstr(const char *str)
{
    struct string_view sv;
    sv.data = str;
    sv.length = str ? strlen(str) : 0;
    return sv;
}

static inline struct string_view sv_create(const char *data, size_t length)
{
    struct string_view sv = {.data = data, .length = length};
    return sv;
}

static inline bool sv_equals(struct string_view a, struct string_view b)
{
    if (a.length != b.length)
        return false;
    return memcmp(a.data, b.data, a.length) == 0;
}

static inline bool sv_equals_cstr(struct string_view sv, const char *cstr)
{
    size_t cstr_len = strlen(cstr);
    if (sv.length != cstr_len)
        return false;
    return memcmp(sv.data, cstr, sv.length) == 0;
}

static inline bool json_extract_string(const char *json, const char *key, struct string_view *out_view)
{
    // Look for the exact key wrapped in quotes
    char search_key[64];
    snprintf(search_key, sizeof(search_key), "\"%s\":\"", key);

    const char *start = strstr(json, search_key);
    if (!start)
        return false;

    start += strlen(search_key); // Move pointer past the key and the colon-quote

    const char *end = strchr(start, '"'); // Find the closing quote
    if (!end)
        return false;

    out_view->data = start;
    out_view->length = end - start;
    return true;
}