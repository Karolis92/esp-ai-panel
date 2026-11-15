/*******************************************************************************
 * Size: 20 px
 * Bpp: 1
 * Opts: --no-compress --bpp 1 --format lvgl --lv-include lvgl.h --font Jersey10-Regular.ttf -r 0x20-0x7E --size 20 -o font_body.c --lv-font-name font_body
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef FONT_BODY
#define FONT_BODY 1
#endif

#if FONT_BODY

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xff, 0x3c,

    /* U+0022 "\"" */
    0xde, 0xf6,

    /* U+0023 "#" */
    0xc, 0x60, 0xc6, 0x3f, 0xf1, 0x8c, 0x18, 0xc3,
    0x9c, 0x31, 0x83, 0x18, 0xff, 0xc6, 0x60, 0x66,
    0x0,

    /* U+0024 "$" */
    0x10, 0xfb, 0xfe, 0xbd, 0x1f, 0x9f, 0xb, 0x17,
    0xaf, 0xfb, 0xe1, 0x0,

    /* U+0025 "%" */
    0x20, 0x27, 0x6, 0xd8, 0xc7, 0x18, 0x26, 0x0,
    0xc4, 0x8, 0x41, 0xa, 0x31, 0xb6, 0xe, 0x40,
    0x40,

    /* U+0026 "&" */
    0x7c, 0x1f, 0x83, 0x30, 0x66, 0x7, 0xc1, 0xf9,
    0xb3, 0x36, 0x18, 0xc3, 0x1f, 0xf9, 0xf3, 0x80,

    /* U+0027 "'" */
    0xfc,

    /* U+0028 "(" */
    0x36, 0xcc, 0xcc, 0xcc, 0xcc, 0xc6, 0x30,

    /* U+0029 ")" */
    0xc6, 0x33, 0x33, 0x33, 0x33, 0x36, 0xc0,

    /* U+002A "*" */
    0x25, 0x48, 0xa0,

    /* U+002B "+" */
    0x30, 0xc3, 0x3f, 0xfc, 0xc3, 0x0,

    /* U+002C "," */
    0xf6,

    /* U+002D "-" */
    0xff, 0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x18, 0xc6, 0x63, 0x19, 0xcc, 0x63, 0x31, 0x8c,
    0x0,

    /* U+0030 "0" */
    0x7f, 0xfc, 0xf3, 0xcf, 0x3c, 0xf3, 0xcf, 0xf7,
    0xc0,

    /* U+0031 "1" */
    0xfd, 0xb6, 0xdb, 0x6d, 0x80,

    /* U+0032 "2" */
    0x7d, 0xff, 0x18, 0x71, 0xc7, 0x1c, 0x30, 0xe1,
    0xff, 0xf8,

    /* U+0033 "3" */
    0x7d, 0xff, 0x18, 0x33, 0xc7, 0x81, 0x3, 0xc7,
    0xfd, 0xf0,

    /* U+0034 "4" */
    0xc, 0x73, 0xdb, 0xcf, 0x3c, 0xff, 0xfc, 0x30,
    0xc0,

    /* U+0035 "5" */
    0xff, 0xfc, 0x30, 0xff, 0xf0, 0xc3, 0xcf, 0xf7,
    0xc0,

    /* U+0036 "6" */
    0x7d, 0xff, 0x1e, 0xf, 0xdf, 0xb1, 0xe3, 0xc7,
    0xfd, 0xf0,

    /* U+0037 "7" */
    0xff, 0xf0, 0xc3, 0xc, 0x73, 0x8e, 0x30, 0xc3,
    0x0,

    /* U+0038 "8" */
    0x7d, 0xff, 0x1e, 0x37, 0xcf, 0xb1, 0xe3, 0xc7,
    0xfd, 0xf0,

    /* U+0039 "9" */
    0x7d, 0xff, 0x1e, 0x3f, 0xef, 0xc1, 0x83, 0xc7,
    0xfd, 0xf0,

    /* U+003A ":" */
    0xf0, 0x3, 0xc0,

    /* U+003B ";" */
    0xf0, 0x3, 0xd8,

    /* U+003C "<" */
    0x7, 0x1f, 0x7c, 0xf0, 0xe0, 0xe0, 0x70, 0x1c,
    0x1f, 0x7,

    /* U+003D "=" */
    0xff, 0xf0, 0x0, 0xff, 0xf0,

    /* U+003E ">" */
    0xe0, 0xf8, 0x3e, 0xf, 0x7, 0x7, 0xe, 0x38,
    0xf8, 0xe0,

    /* U+003F "?" */
    0x7f, 0xfc, 0xc7, 0x3c, 0xc3, 0x0, 0x0, 0xc3,
    0x0,

    /* U+0040 "@" */
    0x3f, 0xc7, 0xfe, 0xe0, 0xfc, 0xf3, 0xdf, 0x3d,
    0xb3, 0xdb, 0x3c, 0xd2, 0xcc, 0xee, 0x0, 0x7f,
    0xc3, 0xf0,

    /* U+0041 "A" */
    0x10, 0x71, 0xf7, 0x7c, 0x78, 0xf1, 0xff, 0xff,
    0x8f, 0x18,

    /* U+0042 "B" */
    0xfd, 0xff, 0x1e, 0x3f, 0xdf, 0xb1, 0xe3, 0xc7,
    0xff, 0xf0,

    /* U+0043 "C" */
    0x7d, 0xff, 0x9e, 0xc, 0x18, 0x30, 0x60, 0xe7,
    0xfd, 0xf0,

    /* U+0044 "D" */
    0xfd, 0xff, 0x3e, 0x3c, 0x78, 0xf1, 0xe3, 0xcf,
    0xff, 0xf0,

    /* U+0045 "E" */
    0xff, 0xff, 0x6, 0xf, 0xdf, 0xb0, 0x60, 0xc1,
    0xff, 0xf8,

    /* U+0046 "F" */
    0xff, 0xff, 0x6, 0xf, 0xdf, 0xb0, 0x60, 0xc1,
    0x83, 0x0,

    /* U+0047 "G" */
    0x7d, 0xff, 0x9e, 0xd, 0xfb, 0xf1, 0xe3, 0xe7,
    0xfd, 0xf0,

    /* U+0048 "H" */
    0xc7, 0x8f, 0x1e, 0x3f, 0xff, 0xf1, 0xe3, 0xc7,
    0x8f, 0x18,

    /* U+0049 "I" */
    0xff, 0xff, 0xfc,

    /* U+004A "J" */
    0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xdf, 0xf7,
    0xc0,

    /* U+004B "K" */
    0xcf, 0x3c, 0xf6, 0xf3, 0xcd, 0xb3, 0xcf, 0x3c,
    0xc0,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xff,
    0xc0,

    /* U+004D "M" */
    0xc0, 0xf8, 0xff, 0x7f, 0xff, 0xdc, 0xf2, 0x3c,
    0xf, 0x3, 0xc0, 0xf0, 0x3c, 0xc,

    /* U+004E "N" */
    0xe1, 0xf0, 0xfc, 0x7e, 0x3f, 0x1e, 0xcf, 0x67,
    0x9f, 0xcf, 0xe7, 0xf1, 0xe0,

    /* U+004F "O" */
    0x7d, 0xff, 0xbe, 0x3c, 0x78, 0xf1, 0xe3, 0xef,
    0xfd, 0xf0,

    /* U+0050 "P" */
    0xfd, 0xff, 0x1e, 0x3c, 0x7f, 0xff, 0x60, 0xc1,
    0x83, 0x0,

    /* U+0051 "Q" */
    0x7d, 0xff, 0xbe, 0x1c, 0x38, 0x70, 0xe1, 0xef,
    0xfd, 0xf0, 0xf0, 0xe0,

    /* U+0052 "R" */
    0xfd, 0xff, 0x1e, 0x3c, 0x7f, 0xbf, 0x63, 0xc7,
    0x8f, 0x18,

    /* U+0053 "S" */
    0x7d, 0xff, 0x1e, 0xf, 0xcf, 0x81, 0x83, 0xc7,
    0xfd, 0xf0,

    /* U+0054 "T" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0x0,

    /* U+0055 "U" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xef,
    0xfd, 0xf0,

    /* U+0056 "V" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xf7, 0x7c,
    0x70, 0x40,

    /* U+0057 "W" */
    0xcc, 0x79, 0x8f, 0x31, 0xe6, 0x3c, 0xc7, 0x98,
    0xf3, 0x1e, 0x63, 0xff, 0xee, 0x38, 0x82, 0x0,

    /* U+0058 "X" */
    0xc7, 0x8f, 0xbb, 0xe3, 0x87, 0x1b, 0x63, 0xc7,
    0x8f, 0x18,

    /* U+0059 "Y" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3e, 0x7b, 0x30, 0xf8,
    0x30, 0x18, 0xc, 0x6, 0x0,

    /* U+005A "Z" */
    0xff, 0xff, 0x7, 0xe, 0x1c, 0x38, 0x70, 0x60,
    0xe0, 0xff, 0xff,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcf,

    /* U+005C "\\" */
    0xc6, 0x30, 0xc6, 0x31, 0xc6, 0x31, 0x86, 0x31,
    0x80,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0x33, 0x3f,

    /* U+005E "^" */
    0x38, 0xfb, 0xbe, 0x30,

    /* U+005F "_" */
    0xff, 0xff,

    /* U+0060 "`" */
    0x99, 0x80,

    /* U+0061 "a" */
    0x7d, 0xfc, 0x1b, 0xf7, 0xf8, 0xf1, 0xff, 0x76,

    /* U+0062 "b" */
    0xc1, 0x83, 0x77, 0xfc, 0x78, 0xf1, 0xe3, 0xc7,
    0xff, 0xf0,

    /* U+0063 "c" */
    0x7d, 0xff, 0x1e, 0xc, 0x18, 0x31, 0xff, 0x7c,

    /* U+0064 "d" */
    0x6, 0xd, 0xff, 0xfc, 0x78, 0xf1, 0xe3, 0xc7,
    0xfd, 0xd8,

    /* U+0065 "e" */
    0x7d, 0xff, 0x1f, 0xff, 0xf8, 0x30, 0x7f, 0x7c,

    /* U+0066 "f" */
    0x3b, 0xd9, 0xff, 0xb1, 0x8c, 0x63, 0x18,

    /* U+0067 "g" */
    0x7f, 0xff, 0x1e, 0x3c, 0x7f, 0xdd, 0x83, 0x7,
    0xfd, 0xf0,

    /* U+0068 "h" */
    0xc1, 0x83, 0x77, 0xfc, 0x78, 0xf1, 0xe3, 0xc7,
    0x8f, 0x18,

    /* U+0069 "i" */
    0xf3, 0xff, 0xfc,

    /* U+006A "j" */
    0x6c, 0x36, 0xdb, 0x6d, 0xfc,

    /* U+006B "k" */
    0xc3, 0xc, 0xf3, 0xdb, 0xcd, 0x32, 0xcf, 0x3c,
    0xc0,

    /* U+006C "l" */
    0xff, 0xff, 0xfc,

    /* U+006D "m" */
    0xd8, 0xdf, 0xff, 0x31, 0xe6, 0x3c, 0xc7, 0x98,
    0xf3, 0x1e, 0x63, 0xcc, 0x60,

    /* U+006E "n" */
    0xdd, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc6,

    /* U+006F "o" */
    0x7d, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xff, 0x7c,

    /* U+0070 "p" */
    0xdd, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xff, 0xfd,
    0x83, 0x0,

    /* U+0071 "q" */
    0x7f, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xff, 0x76,
    0xc, 0x18,

    /* U+0072 "r" */
    0xdb, 0xfe, 0xb0, 0xc3, 0xc, 0x30, 0xc0,

    /* U+0073 "s" */
    0x7f, 0xfc, 0x3f, 0x7c, 0x30, 0xff, 0x7c,

    /* U+0074 "t" */
    0x23, 0x19, 0xff, 0xb1, 0x8c, 0x63, 0xce,

    /* U+0075 "u" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xff, 0x76,

    /* U+0076 "v" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x6d, 0x8e, 0x1c, 0x10,

    /* U+0077 "w" */
    0xcc, 0x79, 0x8f, 0x31, 0xe6, 0x3c, 0xc7, 0x98,
    0xfb, 0x3b, 0xfe, 0x31, 0x80,

    /* U+0078 "x" */
    0xc7, 0xdd, 0xf1, 0xc3, 0x8d, 0xb1, 0xe3, 0xc6,

    /* U+0079 "y" */
    0xcf, 0x3c, 0xf3, 0xcd, 0xb7, 0xce, 0x73, 0x8c,
    0x0,

    /* U+007A "z" */
    0xff, 0xfc, 0x38, 0xe1, 0xc7, 0x1c, 0x7f, 0xfe,

    /* U+007B "{" */
    0x36, 0x66, 0x6c, 0xc6, 0x66, 0x66, 0x30,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xc0,

    /* U+007D "}" */
    0xc6, 0x66, 0x63, 0x36, 0x66, 0x66, 0xc0,

    /* U+007E "~" */
    0x73, 0xff, 0xfc, 0xe0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 69, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 51, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 103, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 6, .adv_w = 206, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 23, .adv_w = 137, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 35, .adv_w = 206, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 52, .adv_w = 189, .box_w = 11, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 51, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 69, .adv_w = 86, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 76, .adv_w = 86, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 83, .adv_w = 103, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 86, .adv_w = 120, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 92, .adv_w = 51, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 93, .adv_w = 120, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 95, .adv_w = 51, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 103, .box_w = 5, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 105, .adv_w = 120, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 69, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 119, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 120, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 120, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 120, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 176, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 51, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 51, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 202, .adv_w = 154, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 212, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 217, .adv_w = 154, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 120, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 236, .adv_w = 206, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 254, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 274, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 294, .adv_w = 120, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 304, .adv_w = 120, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 334, .adv_w = 51, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 337, .adv_w = 120, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 346, .adv_w = 120, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 103, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 171, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 378, .adv_w = 154, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 391, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 401, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 137, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 423, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 433, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 443, .adv_w = 120, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 452, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 462, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 472, .adv_w = 189, .box_w = 11, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 488, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 498, .adv_w = 154, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 511, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 522, .adv_w = 86, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 528, .adv_w = 103, .box_w = 5, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 537, .adv_w = 86, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 543, .adv_w = 137, .box_w = 7, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 547, .adv_w = 137, .box_w = 8, .box_h = 2, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 549, .adv_w = 51, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 10},
    {.bitmap_index = 551, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 559, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 569, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 577, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 587, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 595, .adv_w = 103, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 602, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 612, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 622, .adv_w = 51, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 625, .adv_w = 69, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 630, .adv_w = 120, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 639, .adv_w = 51, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 642, .adv_w = 189, .box_w = 11, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 655, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 663, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 671, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 681, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 691, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 698, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 705, .adv_w = 103, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 712, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 720, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 728, .adv_w = 189, .box_w = 11, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 741, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 749, .adv_w = 120, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 758, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 766, .adv_w = 86, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 773, .adv_w = 51, .box_w = 2, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 777, .adv_w = 86, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 784, .adv_w = 137, .box_w = 7, .box_h = 4, .ofs_x = 0, .ofs_y = 3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t font_body = {
#else
lv_font_t font_body = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if FONT_BODY*/

