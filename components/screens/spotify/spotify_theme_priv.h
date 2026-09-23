#pragma once
#include "theme.h"
#include "api_spotify.h"
#include <stdint.h>
#include <stdbool.h>

void sp_theme_trigger_decode(struct spotify_track_dto *track);
bool sp_theme_prepare_manual(struct spotify_track_dto *track);
bool sp_theme_prepare_auto(struct spotify_track_dto *track);
void sp_theme_get_pixel(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b);
uint8_t sp_theme_calc_luma(uint8_t r, uint8_t g, uint8_t b);
bool sp_theme_has_art(void);

extern const struct theme_vtable vt_sp_default;
extern const struct theme_vtable vt_sp_square;
extern const struct theme_vtable vt_sp_disk;