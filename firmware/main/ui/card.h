#pragma once
#include <stdbool.h>

#include "lvgl.h"
#include "model.h"
#include "pixframe.h"

typedef struct {
    lv_obj_t *root;
    lv_obj_t *ground;
    lv_obj_t *mascot;
    lv_obj_t *prop;     // laptop / mug / book / headphones (overlay)
    lv_obj_t *deco;     // steam / note / alert / check / cross
    lv_obj_t *zzz;      // sleeping label
    lv_obj_t *title;
    lv_obj_t *status_row;
    lv_obj_t *dot;
    lv_obj_t *status;
    lv_obj_t *pill;     // "! NEEDS YOU"
    pixframe_t frame;
    pixframe_t pill_frame;
    pixframe_t dot_frame;
    session_t cur;
    bool has;
    int mascot_x;
    int mascot_y;
    bool anim_restart;  // scene changed: card_animate() re-bases anim_t0 on its next tick
    uint32_t anim_t0;   // tick when the current scene started
} card_t;

void card_create(card_t *c, lv_obj_t *parent, int x, int y);
/** NULL → empty slot. Only touches LVGL objects when something changed. */
void card_set(card_t *c, const session_t *s);
/** Advance idle animations; called every ANIM_PERIOD_MS from the LVGL task. */
void card_animate(card_t *c, uint32_t tick);
