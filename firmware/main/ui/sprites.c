// Original pixel-art sprites, defined as ASCII grids and rasterized at boot.
#include "sprites.h"

#include <stdlib.h>
#include <string.h>

#include "esp_heap_caps.h"
#include "esp_log.h"

typedef struct {
    const char *const *rows;
    uint8_t w, h, scale;
} sprite_def_t;

// Palette: '.' transparent
static lv_color32_t pal(char c)
{
    uint32_t rgb;
    switch (c) {
    case '#': rgb = 0xf0782d; break; // mascot orange
    case 'o': rgb = 0xec6a2c; break; // orange (icons, book cover)
    case 'O': rgb = 0xb94c1c; break; // dark orange
    case 'w': rgb = 0xf7e9d6; break; // cream
    case 'b': rgb = 0x9a7272; break; // headphone band (mauve grey)
    case 'g': rgb = 0xa98276; break; // laptop light
    case 'G': rgb = 0x5c3a33; break; // laptop dark
    case 'r': rgb = 0xff3b30; break; // alert red
    case 'y': rgb = 0x43d13b; break; // green
    case 's': rgb = 0xe8d6c4; break; // steam
    case 'k': rgb = 0x2b1611; break; // terminal screen / bubble dots
    case 'Y': rgb = 0xf2c14e; break; // pencil yellow
    case 'p': rgb = 0xe58a9a; break; // eraser pink
    default:  return (lv_color32_t){.blue = 0, .green = 0, .red = 0, .alpha = 0};
    }
    return (lv_color32_t){
        .blue = rgb & 0xff, .green = (rgb >> 8) & 0xff, .red = (rgb >> 16) & 0xff, .alpha = 0xff};
}

#define DEF(name, scale_, ...)                                           \
    static const char *const name##_rows[] = {__VA_ARGS__};              \
    static const sprite_def_t name = {name##_rows, 0,                    \
        sizeof(name##_rows) / sizeof(name##_rows[0]), scale_}

DEF(d_mascot, 4,
    "..################..",
    "..################..",
    "..####..####..####..",
    "..####..####..####..",
    "####################",
    "####################",
    "..################..",
    "..################..",
    "..##.##......##.##..",
    "..##.##......##.##..");

DEF(d_mascot_step, 4,
    "..################..",
    "..################..",
    "..####..####..####..",
    "..####..####..####..",
    "####################",
    "####################",
    "..################..",
    "..################..",
    "...##.##....##.##...",
    "...##.##....##.##...");

DEF(d_mascot_sleep, 4,
    "..################..",
    "..################..",
    "..################..",
    "..###.####..####.###",
    "####################",
    "####################",
    "..################..",
    "..################..",
    "..##.##......##.##..",
    "..##.##......##.##..");

DEF(d_laptop, 4,
    "..gggggggggggg",
    "..gGGGGGGGGGGg",
    "..gGGGGoGGGGGg",
    "..gGGGoooGGGGg",
    "..gGGGGoGGGGGg",
    "..gGGGGGGGGGGg",
    "..gggggggggggg",
    "gggggggggggggg",
    ".gggggggggggg.");

DEF(d_mug, 4,
    ".wwwwww...",
    ".wwwwwwww.",
    ".wwwwww.w.",
    ".wwwwww.w.",
    ".wwwwwwww.",
    ".wwwwww...",
    "..wwww....");

DEF(d_steam_a, 3,
    ".s..s.",
    "s..s..",
    ".s..s.",
    "..s..s",
    ".s..s.",
    "s..s..");

DEF(d_steam_b, 3,
    "s..s..",
    ".s..s.",
    "..s..s",
    ".s..s.",
    "s..s..",
    ".s..s.");

DEF(d_book, 4,
    "..wwwwww..wwwwww..",
    ".owwwwwwwwwwwwwwo.",
    ".owbbbbwwwwbbbbwo.",
    ".owwwwwwwwwwwwwwo.",
    ".owbbbbwwwwbbbbwo.",
    ".owwwwwwOOwwwwwwo.",
    "oooooooooooooooooo",
    ".oooooooooooooooo.");

#define BUBBLE_TOP \
    "...wwwwwwwwww...", \
    ".wwwwwwwwwwwwww.", \
    "wwwwwwwwwwwwwwww"
#define BUBBLE_BOTTOM \
    "wwwwwwwwwwwwwwww", \
    ".wwwwwwwwwwwwww.", \
    "...wwwwwwwwww...", \
    "..ww............", \
    ".w.............."

DEF(d_think_1, 3, BUBBLE_TOP,
    "wwwkkwwwwwwwwwww",
    "wwwkkwwwwwwwwwww", BUBBLE_BOTTOM);
DEF(d_think_2, 3, BUBBLE_TOP,
    "wwwkkwwkkwwwwwww",
    "wwwkkwwkkwwwwwww", BUBBLE_BOTTOM);
DEF(d_think_3, 3, BUBBLE_TOP,
    "wwwkkwwkkwwkkwww",
    "wwwkkwwkkwwkkwww", BUBBLE_BOTTOM);

DEF(d_terminal, 4,
    "gggggggggggggg",
    "gkkkkkkkkkkkkg",
    "gkykkkkkkkkkkg",
    "gkkykkkkkkkkkg",
    "gkykkyyykkkkkg",
    "gkkkkkkkkkkkkg",
    "gggggggggggggg",
    ".....gggg.....",
    "...gggggggg...");

DEF(d_terminal_off, 4,
    "gggggggggggggg",
    "gkkkkkkkkkkkkg",
    "gkykkkkkkkkkkg",
    "gkkykkkkkkkkkg",
    "gkykkkkkkkkkkg",
    "gkkkkkkkkkkkkg",
    "gggggggggggggg",
    ".....gggg.....",
    "...gggggggg...");

DEF(d_notepad, 4,
    "bbbbbbbbbbb",
    "wwwwwwwwwww",
    "wbbbbbbbbww",
    "wwwwwwwwwww",
    "wbbbbbbwwww",
    "wwwwwwwwwww",
    "wbbbbbbbbbw",
    "wwwwwwwwwww",
    "wbbbbwwwwww",
    "wwwwwwwwwww");

DEF(d_pencil, 3,
    "......pp",
    ".....YYp",
    "....YYY.",
    "...YYY..",
    "..YYY...",
    ".YYY....",
    ".GY.....",
    "G.......");

DEF(d_alert, 3,
    "..rr.rr",
    "..rr.rr",
    ".rr.rr.",
    ".rr.rr.",
    ".rr.rr.",
    "rr.rr..",
    ".......",
    "rr.rr..",
    "rr.rr..");

DEF(d_check, 3,
    "........y",
    ".......yy",
    "y.....yy.",
    "yy...yy..",
    ".yy.yy...",
    "..yyy....",
    "...y.....");

DEF(d_cross, 3,
    "rr...rr",
    ".rr.rr.",
    "..rrr..",
    "..rrr..",
    ".rr.rr.",
    "rr...rr");

DEF(d_sparkle_a, 3,
    "...w...",
    "...Y...",
    "...Y...",
    "wYYwYYw",
    "...Y...",
    "...Y...",
    "...w...");

DEF(d_sparkle_b, 3,
    ".......",
    ".......",
    "...Y...",
    "..YwY..",
    "...Y...",
    ".......",
    ".......");

DEF(d_clock, 2,
    "...ooooo...",
    ".oo.....oo.",
    ".o...o...o.",
    "o....o....o",
    "o....o....o",
    "o....ooo..o",
    "o.........o",
    "o.........o",
    ".o.......o.",
    ".oo.....oo.",
    "...ooooo...");

DEF(d_calendar, 2,
    "..o.....o..",
    "ooooooooooo",
    "ooooooooooo",
    "o.........o",
    "o.o.o.o.o.o",
    "o.........o",
    "o.o.o.o.o.o",
    "o.........o",
    "ooooooooooo");

DEF(d_refresh, 2,
    "..www.w",
    ".w...ww",
    "w...www",
    "w......",
    "......w",
    "www...w",
    "ww...w.",
    "w.www..");

static const sprite_def_t *const s_defs[SPR_COUNT] = {
    [SPR_MASCOT] = &d_mascot,
    [SPR_MASCOT_STEP] = &d_mascot_step,
    [SPR_MASCOT_SLEEP] = &d_mascot_sleep,
    [SPR_LAPTOP] = &d_laptop,
    [SPR_MUG] = &d_mug,
    [SPR_STEAM_A] = &d_steam_a,
    [SPR_STEAM_B] = &d_steam_b,
    [SPR_BOOK] = &d_book,
    [SPR_THINK_1] = &d_think_1,
    [SPR_THINK_2] = &d_think_2,
    [SPR_THINK_3] = &d_think_3,
    [SPR_TERMINAL] = &d_terminal,
    [SPR_TERMINAL_OFF] = &d_terminal_off,
    [SPR_NOTEPAD] = &d_notepad,
    [SPR_PENCIL] = &d_pencil,
    [SPR_ALERT] = &d_alert,
    [SPR_CHECK] = &d_check,
    [SPR_CROSS] = &d_cross,
    [SPR_CLOCK] = &d_clock,
    [SPR_CALENDAR] = &d_calendar,
    [SPR_REFRESH] = &d_refresh,
    [SPR_SPARKLE_A] = &d_sparkle_a,
    [SPR_SPARKLE_B] = &d_sparkle_b,
};

static lv_image_dsc_t s_imgs[SPR_COUNT];

static void build(const sprite_def_t *def, lv_image_dsc_t *out)
{
    const int gw = (int)strlen(def->rows[0]);
    const int gh = def->h;
    const int sc = def->scale;
    const int w = gw * sc, h = gh * sc;
    const size_t size = (size_t)w * h * sizeof(lv_color32_t);
    lv_color32_t *px = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (!px) {
        px = malloc(size);
    }
    for (int y = 0; y < h; y++) {
        const char *row = def->rows[y / sc];
        const int rlen = (int)strlen(row);
        for (int x = 0; x < w; x++) {
            const int gx = x / sc;
            px[y * w + x] = pal(gx < rlen ? row[gx] : '.');
        }
    }
    memset(out, 0, sizeof(*out));
    out->header.magic = LV_IMAGE_HEADER_MAGIC;
    out->header.cf = LV_COLOR_FORMAT_ARGB8888;
    out->header.w = w;
    out->header.h = h;
    out->header.stride = w * sizeof(lv_color32_t);
    out->data_size = size;
    out->data = (const uint8_t *)px;
}

void sprites_init(void)
{
    for (int i = 0; i < SPR_COUNT; i++) {
        build(s_defs[i], &s_imgs[i]);
    }
}

const lv_image_dsc_t *sprite(sprite_id_t id)
{
    return &s_imgs[id];
}
