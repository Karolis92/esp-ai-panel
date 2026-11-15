#pragma once
#include "lvgl.h"

// Palette sampled from design.png
#define COL_BG          lv_color_hex(0x140906)
#define COL_CARD_BG     lv_color_hex(0x1d0d09)
#define COL_ORANGE      lv_color_hex(0xec6a2c)
#define COL_ORANGE_HI   lv_color_hex(0xf5862f)
#define COL_GROUND      lv_color_hex(0x9c3f22)
#define COL_TRACK       lv_color_hex(0x3a1c16)
#define COL_TEXT        lv_color_hex(0xf7e6d0)
#define COL_TEXT_MUTED  lv_color_hex(0xd7a8b4)
#define COL_GREEN       lv_color_hex(0x43d13b)
#define COL_IDLE_DOT    lv_color_hex(0xa87878)
#define COL_RED         lv_color_hex(0xd2261f)
#define COL_RED_HI      lv_color_hex(0xff4a3d)
#define COL_YELLOW      lv_color_hex(0xffd23f)
#define COL_GLOW        lv_color_hex(0xff5a1f)
#define COL_DOT_OFF     lv_color_hex(0x5a2a22)

LV_FONT_DECLARE(font_pixel_16);  // Silkscreen Bold 16 — titles, labels
LV_FONT_DECLARE(font_pixel_24);  // Silkscreen Bold 24 — pager
LV_FONT_DECLARE(font_pixel_48);  // Silkscreen Regular 48 — percentages (digits, %, -, —)
LV_FONT_DECLARE(font_body);      // Jersey 10 @20px (2x pixel grid) — status / small text, ASCII only

// Layout (480x480)
#define UI_MARGIN     8
#define CARD_W        150
#define CARD_H        136
#define CARD_GAP      7
#define ROW_GAP       6
#define PAGER_Y       (UI_MARGIN + 2 * CARD_H + ROW_GAP + ROW_GAP)
#define PAGER_H       44
#define TILES_Y       (PAGER_Y + PAGER_H + ROW_GAP)
#define TILE_W        229
#define TILE_H        130
#define CARDS_PER_PAGE 6
