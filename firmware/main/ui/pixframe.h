#pragma once
// Pixel-art frames: fill + border with stepped ("pixelated") corners and an optional
// pixel glow ring, drawn by the object itself instead of LVGL's smooth radius/shadow.

#include <stdbool.h>

#include "lvgl.h"

typedef struct {
    lv_color_t border;
    lv_color_t fill;
    lv_color_t fill_pressed; // used while the object is pressed
    lv_color_t glow;
    lv_opa_t border_opa;
    lv_opa_t fill_opa;
    lv_opa_t glow_opa;       // 0 = no glow
    uint8_t px;              // size of one "pixel" (border thickness)
    uint8_t steps;           // corner staircase length in pixels (0 = square)
} pixframe_t;

/** Sensible defaults: card colors, 3px pixels, 3-step corners, no glow. */
pixframe_t pixframe_default(void);

/** Make `obj` draw `pf` (which must outlive obj). Clears obj's own bg/border/radius/shadow. */
void pixframe_attach(lv_obj_t *obj, pixframe_t *pf);

/** Call after changing fields of an attached frame. */
void pixframe_refresh(lv_obj_t *obj);
