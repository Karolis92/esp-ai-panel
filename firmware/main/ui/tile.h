#pragma once
#include "lvgl.h"
#include "model.h"
#include "pixframe.h"
#include "sprites.h"

typedef struct {
    lv_obj_t *root;
    lv_obj_t *pct;
    lv_obj_t *bar;      // track
    lv_obj_t *bar_fill; // indicator
    pixframe_t frame, bar_frame, fill_frame;
    lv_obj_t *reset;
    int last_pct;
    char last_reset[32];
} tile_t;

void tile_create(tile_t *t, lv_obj_t *parent, int x, int y, const char *label, sprite_id_t icon);
/** host_now: current host unix time (0 = unknown). */
void tile_set(tile_t *t, const usage_window_t *w, int64_t host_now);
