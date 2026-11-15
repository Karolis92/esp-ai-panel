#include "tile.h"

#include <stdio.h>
#include <string.h>

#include "theme.h"

#define INNER_W (TILE_W - 6)
#define BAR_W   (INNER_W - 24)
#define BAR_H   18
#define BAR_PX  3

void tile_create(tile_t *t, lv_obj_t *parent, int x, int y, const char *label, sprite_id_t icon)
{
    memset(t, 0, sizeof(*t));
    t->last_pct = -2;

    lv_obj_t *r = lv_obj_create(parent);
    t->root = r;
    lv_obj_remove_flag(r, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_pos(r, x, y);
    lv_obj_set_size(r, TILE_W, TILE_H);
    t->frame = pixframe_default();
    pixframe_attach(r, &t->frame);
    lv_obj_set_style_pad_all(r, 3, 0);

    // header: icon + label, centered
    lv_obj_t *hdr = lv_obj_create(r);
    lv_obj_remove_style_all(hdr);
    lv_obj_remove_flag(hdr, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(hdr, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(hdr, 10, 0);
    lv_obj_t *ic = lv_image_create(hdr);
    lv_image_set_src(ic, sprite(icon));
    lv_obj_t *lbl = lv_label_create(hdr);
    lv_label_set_text(lbl, label);
    lv_obj_set_style_text_font(lbl, &font_pixel_16, 0);
    lv_obj_set_style_text_color(lbl, COL_TEXT, 0);

    t->pct = lv_label_create(r);
    lv_obj_set_style_text_font(t->pct, &font_pixel_48, 0);
    lv_obj_set_style_text_color(t->pct, COL_TEXT, 0);
    lv_obj_align(t->pct, LV_ALIGN_TOP_MID, 0, 32);
    lv_label_set_text(t->pct, "-");

    // Pixel progress bar: stepped track with a stepped fill inside it.
    t->bar = lv_obj_create(r);
    lv_obj_remove_flag(t->bar, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(t->bar, BAR_W, BAR_H);
    lv_obj_align(t->bar, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_pad_all(t->bar, BAR_PX, 0);
    t->bar_frame = pixframe_default();
    t->bar_frame.px = BAR_PX;
    t->bar_frame.steps = 2;
    t->bar_frame.fill = COL_TRACK;
    t->bar_frame.border = COL_GROUND;
    pixframe_attach(t->bar, &t->bar_frame);

    t->bar_fill = lv_obj_create(t->bar);
    lv_obj_remove_flag(t->bar_fill, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_pos(t->bar_fill, 0, 0);
    lv_obj_set_size(t->bar_fill, 0, BAR_H - 2 * BAR_PX);
    t->fill_frame = pixframe_default();
    t->fill_frame.px = BAR_PX;
    t->fill_frame.steps = 1;
    t->fill_frame.fill = t->fill_frame.border = COL_ORANGE_HI;
    pixframe_attach(t->bar_fill, &t->fill_frame);
    lv_obj_add_flag(t->bar_fill, LV_OBJ_FLAG_HIDDEN);

    // "⟳ Resets in …" row
    lv_obj_t *row = lv_obj_create(r);
    lv_obj_remove_style_all(row);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(row, LV_ALIGN_TOP_MID, 0, 105);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 6, 0);
    lv_obj_t *ic2 = lv_image_create(row);
    lv_image_set_src(ic2, sprite(SPR_REFRESH));

    t->reset = lv_label_create(row);
    lv_obj_set_style_text_font(t->reset, &font_body, 0);
    lv_obj_set_style_text_color(t->reset, COL_TEXT, 0);
    lv_label_set_text(t->reset, "No data yet");
}

static void fmt_reset(char *buf, size_t n, int64_t secs)
{
    if (secs <= 0) {
        snprintf(buf, n, "Resetting now");
    } else if (secs >= 86400) {
        snprintf(buf, n, "Resets in %dd %dh", (int)(secs / 86400), (int)((secs % 86400) / 3600));
    } else if (secs >= 3600) {
        snprintf(buf, n, "Resets in %dh %dm", (int)(secs / 3600), (int)((secs % 3600) / 60));
    } else {
        snprintf(buf, n, "Resets in %dm", (int)((secs + 59) / 60));
    }
}

void tile_set(tile_t *t, const usage_window_t *w, int64_t host_now)
{
    const int pct = w->valid ? w->pct : -1;
    if (pct != t->last_pct) {
        t->last_pct = pct;
        if (pct < 0) {
            lv_label_set_text(t->pct, "\xE2\x80\x94"); // em dash
        } else {
            lv_label_set_text_fmt(t->pct, "%d%%", pct);
        }
        // fill width snapped to whole pixels; needs at least 3 pixels to show its shape
        const int inner = BAR_W - 2 * BAR_PX;
        int w = pct <= 0 ? 0 : (inner * pct / 100) / BAR_PX * BAR_PX;
        if (w > 0 && w < 3 * BAR_PX) w = 3 * BAR_PX;
        lv_obj_set_width(t->bar_fill, w);
        if (w > 0) lv_obj_remove_flag(t->bar_fill, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(t->bar_fill, LV_OBJ_FLAG_HIDDEN);
    }

    char buf[32];
    if (!w->valid) {
        snprintf(buf, sizeof(buf), "No data yet");
    } else if (w->reset == 0 || host_now == 0) {
        buf[0] = '\0';
    } else {
        fmt_reset(buf, sizeof(buf), w->reset - host_now);
    }
    if (strcmp(buf, t->last_reset) != 0) {
        strcpy(t->last_reset, buf);
        lv_label_set_text(t->reset, buf);
    }
}
