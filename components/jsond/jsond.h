#pragma once
#include "string_view.h"
#include <stdbool.h>

/**
 * Highly optimized, zero-allocation JSON extractor.
 * Searches for a string value by key and returns a string_view pointing directly
 * into the original HTTP buffer.
 */
bool jsond_extract_string(const char *json, const char *key, struct string_view *out_view);