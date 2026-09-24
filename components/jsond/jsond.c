#include "jsond.h"
#include <string.h>
#include <stdio.h>

bool jsond_extract_string(const char *json, const char *key, struct string_view *out_view)
{
    if (!json || !key || !out_view)
        return false;

    // Look for the exact key wrapped in quotes, followed by a colon and the opening quote
    char search_key[64];
    snprintf(search_key, sizeof(search_key), "\"%s\":\"", key);

    const char *start = strstr(json, search_key);
    if (!start)
        return false;

    start += strlen(search_key); // Move pointer past the key and the colon-quote

    // Find the closing quote of the value
    const char *end = strchr(start, '"');
    if (!end)
        return false;

    // Use your existing collection helper to populate the struct!
    *out_view = sv_create(start, end - start);
    return true;
}