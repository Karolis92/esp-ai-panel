#pragma once
#include "lvgl.h"

typedef enum {
    SPR_MASCOT = 0,
    SPR_MASCOT_STEP,   // alternate legs (walk/typing frame)
    SPR_MASCOT_SLEEP,
    SPR_LAPTOP,
    SPR_MUG,
    SPR_STEAM_A,
    SPR_STEAM_B,
    SPR_BOOK,
    SPR_THINK_1,       // thought bubble with 1..3 dots
    SPR_THINK_2,
    SPR_THINK_3,
    SPR_TERMINAL,      // ">_" with cursor
    SPR_TERMINAL_OFF,  // cursor blinked off
    SPR_NOTEPAD,
    SPR_PENCIL,
    SPR_ALERT,
    SPR_CHECK,
    SPR_CROSS,
    SPR_CLOCK,
    SPR_CALENDAR,
    SPR_REFRESH,
    SPR_SPARKLE_A,     // 4-point twinkle (big)
    SPR_SPARKLE_B,     // twinkle (small)
    SPR_COUNT
} sprite_id_t;

/** Rasterize all pixel-art sprites (ARGB8888, pre-scaled) into PSRAM. Call once before the UI. */
void sprites_init(void);
const lv_image_dsc_t *sprite(sprite_id_t id);
