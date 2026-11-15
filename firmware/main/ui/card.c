#include "card.h"

#include <stdio.h>
#include <string.h>

#include "serial_link.h"
#include "sprites.h"
#include "theme.h"

// Inner (content) area is CARD_W-6 x CARD_H-6 because of the 3px pixel border (padding).
#define INNER_W   (CARD_W - 6)
#define GROUND_Y  62
#define MASCOT_W  80
#define MASCOT_H  40
#define MASCOT_Y  (GROUND_Y - MASCOT_H)
#define MASCOT_CX ((INNER_W - MASCOT_W) / 2)
#define NOTEPAD_X (INNER_W - 50)
#define NOTEPAD_Y (GROUND_Y - 40)
#define STATUS_MAX_W (INNER_W - 14 - 15) // status row minus dot + gap, with side margin
#define SCROLL_SPEED lv_anim_speed_clamped(15, 1000, 10000) // px/s for overflowing labels (LVGL default 40)

static void no_click(lv_obj_t *o)
{
    lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
}

static lv_obj_t *plain_obj(lv_obj_t *parent)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    no_click(o);
    return o;
}

static void on_click(lv_event_t *e)
{
    card_t *c = lv_event_get_user_data(e);
    if (!c->has) {
        return;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "{\"t\":\"tap\",\"id\":\"%s\"}", c->cur.id);
    serial_link_send(buf);
}

void card_create(card_t *c, lv_obj_t *parent, int x, int y)
{
    memset(c, 0, sizeof(*c));
    lv_obj_t *r = lv_obj_create(parent);
    c->root = r;
    lv_obj_remove_flag(r, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(r, x, y);
    lv_obj_set_size(r, CARD_W, CARD_H);
    c->frame = pixframe_default();
    pixframe_attach(r, &c->frame);
    lv_obj_set_style_pad_all(r, 3, 0); // content area = inside the 3px pixel border
    lv_obj_add_event_cb(r, on_click, LV_EVENT_CLICKED, c);

    c->ground = plain_obj(r);
    lv_obj_set_pos(c->ground, 6, GROUND_Y);
    lv_obj_set_size(c->ground, INNER_W - 12, 3);
    lv_obj_set_style_bg_color(c->ground, COL_GROUND, 0);
    lv_obj_set_style_bg_opa(c->ground, LV_OPA_COVER, 0);

    c->prop = lv_image_create(r); // z-order fixed up in layout_scene()
    c->mascot = lv_image_create(r);
    lv_image_set_src(c->mascot, sprite(SPR_MASCOT));
    c->deco = lv_image_create(r);

    c->zzz = lv_label_create(r);
    lv_obj_set_style_text_font(c->zzz, &font_pixel_16, 0);
    lv_obj_set_style_text_color(c->zzz, COL_TEXT, 0);
    lv_label_set_text(c->zzz, "z Z Z");

    c->title = lv_label_create(r);
    lv_obj_set_width(c->title, INNER_W - 8);
    lv_obj_set_pos(c->title, 4, 70);
    lv_label_set_long_mode(c->title, LV_LABEL_LONG_SCROLL); // long folder names slide back and forth
    lv_obj_set_style_anim_duration(c->title, SCROLL_SPEED, 0);
    lv_obj_set_style_text_align(c->title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(c->title, &font_pixel_16, 0);
    lv_obj_set_style_text_color(c->title, COL_TEXT, 0);

    c->status_row = plain_obj(r);
    lv_obj_set_size(c->status_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_max_width(c->status_row, INNER_W - 14, 0); // keep clear of the border
    lv_obj_align(c->status_row, LV_ALIGN_TOP_MID, 0, 96);
    lv_obj_set_flex_flow(c->status_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(c->status_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(c->status_row, 5, 0);

    c->dot = plain_obj(c->status_row);
    lv_obj_set_size(c->dot, 10, 10);
    c->dot_frame = pixframe_default();
    c->dot_frame.px = 2;
    c->dot_frame.steps = 1; // 5x5 pixel dot with the corners cut
    pixframe_attach(c->dot, &c->dot_frame);

    c->status = lv_label_create(c->status_row);
    lv_obj_set_style_text_font(c->status, &font_body, 0);
    lv_obj_set_style_text_color(c->status, COL_TEXT_MUTED, 0);
    // Width is set per text in card_set(): content-sized if it fits, else fixed and scrolling.
    lv_label_set_long_mode(c->status, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_anim_duration(c->status, SCROLL_SPEED, 0);
    lv_obj_set_height(c->status, lv_font_get_line_height(&font_body)); // single line

    c->pill = plain_obj(r);
    lv_obj_set_size(c->pill, INNER_W - 12, 28);
    lv_obj_align(c->pill, LV_ALIGN_TOP_MID, 0, 93);
    c->pill_frame = pixframe_default();
    c->pill_frame.px = 2;
    c->pill_frame.steps = 2;
    c->pill_frame.fill = COL_RED;
    c->pill_frame.border = COL_RED_HI;
    pixframe_attach(c->pill, &c->pill_frame);

    lv_obj_t *needs = lv_label_create(c->pill);
    lv_label_set_text(needs, "NEEDS YOU");
    lv_obj_set_style_text_font(needs, &font_pixel_16, 0);
    lv_obj_set_style_text_color(needs, COL_YELLOW, 0);
    lv_obj_center(needs);

    card_set(c, NULL);
}

static void show(lv_obj_t *o, bool on)
{
    if (on) lv_obj_remove_flag(o, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
}

static void place(lv_obj_t *img, sprite_id_t id, int x, int y)
{
    lv_image_set_src(img, sprite(id));
    lv_obj_set_pos(img, x, y);
    show(img, true);
}

static void layout_scene(card_t *c, const session_t *s)
{
    show(c->prop, false);
    show(c->deco, false);
    show(c->zzz, false);
    c->mascot_x = MASCOT_CX;
    c->mascot_y = MASCOT_Y;
    sprite_id_t body = SPR_MASCOT;

    switch (s->anim) {
    case ANIM_LAPTOP:
        c->mascot_x = 4;
        place(c->prop, SPR_LAPTOP, 80, GROUND_Y - 36);
        break;
    case ANIM_THINKING:
        c->mascot_x = 12;
        place(c->deco, SPR_THINK_1, INNER_W - 50, 0);
        break;
    case ANIM_TERMINAL:
        c->mascot_x = 4;
        place(c->prop, SPR_TERMINAL, 84, GROUND_Y - 36);
        break;
    case ANIM_COFFEE:
        c->mascot_x = 4;
        place(c->prop, SPR_MUG, 92, GROUND_Y - 28);
        place(c->deco, SPR_STEAM_A, 100, 10);
        break;
    case ANIM_BOOK:
        place(c->prop, SPR_BOOK, (INNER_W - 72) / 2, GROUND_Y - 28);
        break;
    case ANIM_PLAN:
        c->mascot_x = 4;
        place(c->prop, SPR_NOTEPAD, NOTEPAD_X, NOTEPAD_Y);
        place(c->deco, SPR_PENCIL, NOTEPAD_X + 4, NOTEPAD_Y + 8 - 21);
        break;
    case ANIM_ALERT:
        place(c->deco, SPR_ALERT, INNER_W - 26, 2);
        break;
    case ANIM_DONE:
        place(c->deco, SPR_CHECK, INNER_W - 32, 6);
        break;
    case ANIM_ERROR:
        place(c->deco, SPR_CROSS, INNER_W - 26, 6);
        break;
    case ANIM_SLEEP:
    default:
        body = SPR_MASCOT_SLEEP;
        lv_obj_set_pos(c->zzz, INNER_W - 50, 2);
        show(c->zzz, true);
        break;
    }
    lv_image_set_src(c->mascot, sprite(body));
    lv_obj_set_pos(c->mascot, c->mascot_x, c->mascot_y);

    // Z-order: props in front of the mascot (laptop lid, book, notepad), deco on top.
    lv_obj_move_foreground(c->prop);
    lv_obj_move_foreground(c->deco);
}

void card_set(card_t *c, const session_t *s)
{
    if (!s) {
        if (!c->has && lv_obj_has_flag(c->mascot, LV_OBJ_FLAG_HIDDEN)) {
            return;
        }
        c->has = false;
        memset(&c->cur, 0, sizeof(c->cur));
        c->frame.border_opa = LV_OPA_30;
        c->frame.border = COL_ORANGE;
        c->frame.fill_opa = LV_OPA_40;
        c->frame.glow_opa = 0;
        pixframe_refresh(c->root);
        const lv_obj_t *kids[] = {c->ground, c->mascot, c->prop, c->deco, c->zzz, c->title, c->status_row, c->pill};
        for (size_t i = 0; i < sizeof(kids) / sizeof(kids[0]); i++) {
            show((lv_obj_t *)kids[i], false);
        }
        return;
    }

    const bool was = c->has;
    if (was && memcmp(&c->cur, s, sizeof(*s)) == 0) {
        return;
    }
    const bool scene_changed = !was || c->cur.anim != s->anim;
    const bool state_changed = !was || c->cur.state != s->state;
    c->cur = *s;
    c->has = true;

    if (!was) {
        c->frame.border_opa = LV_OPA_COVER;
        c->frame.fill_opa = LV_OPA_COVER;
        pixframe_refresh(c->root);
        show(c->ground, true);
        show(c->mascot, true);
        show(c->title, true);
    }
    if (scene_changed) {
        layout_scene(c, s);
    }
    lv_label_set_text(c->title, s->title);

    if (state_changed) {
        const bool needs = s->state == CARD_NEEDS;
        show(c->pill, needs);
        show(c->status_row, !needs);
        c->frame.border = needs ? COL_YELLOW : COL_ORANGE;
        c->frame.glow_opa = needs ? LV_OPA_70 : 0;
        pixframe_refresh(c->root);
        lv_color_t dot = COL_GREEN;
        if (s->state == CARD_IDLE) dot = COL_IDLE_DOT;
        if (s->state == CARD_ERROR) dot = COL_RED_HI;
        c->dot_frame.fill = c->dot_frame.border = dot;
        pixframe_refresh(c->dot);
    }
    lv_label_set_text(c->status, s->activity);
    const int32_t tw = lv_text_get_width(s->activity, strlen(s->activity), &font_body, 0);
    lv_obj_set_width(c->status, tw > STATUS_MAX_W ? STATUS_MAX_W : LV_SIZE_CONTENT);
}

void card_animate(card_t *c, uint32_t tick)
{
    if (!c->has) {
        return;
    }
    int dx = 0, dy = 0;
    switch (c->cur.anim) {
    case ANIM_LAPTOP:
        // typing: bob + leg shuffle
        dy = (tick / 2) % 2 ? -4 : 0;
        lv_image_set_src(c->mascot, sprite((tick / 2) % 2 ? SPR_MASCOT_STEP : SPR_MASCOT));
        break;
    case ANIM_COFFEE:
        dy = (tick / 4) % 2 ? -4 : 0;
        lv_image_set_src(c->deco, sprite((tick / 3) % 2 ? SPR_STEAM_B : SPR_STEAM_A));
        break;
    case ANIM_BOOK:
        dy = (tick / 6) % 2 ? -4 : 0; // slow reading nod
        break;
    case ANIM_THINKING: {
        static const sprite_id_t dots[] = {SPR_THINK_1, SPR_THINK_2, SPR_THINK_3};
        lv_image_set_src(c->deco, sprite(dots[(tick / 3) % 3]));
        dy = (tick / 8) % 2 ? -4 : 0; // slow, pensive bob
        break;
    }
    case ANIM_TERMINAL:
        dy = (tick / 2) % 2 ? -4 : 0; // typing
        lv_image_set_src(c->mascot, sprite((tick / 2) % 2 ? SPR_MASCOT_STEP : SPR_MASCOT));
        lv_image_set_src(c->prop, sprite((tick / 3) % 2 ? SPR_TERMINAL_OFF : SPR_TERMINAL));
        break;
    case ANIM_PLAN: {
        // pencil writes along the notepad lines (rows 2,4,6,8), then starts over
        const int line = (int)((tick / 8) % 4);
        const int prog = (int)(tick % 8);
        lv_obj_set_pos(c->deco, NOTEPAD_X + 4 + prog * 4, NOTEPAD_Y + (2 + 2 * line) * 4 - 21);
        dy = (tick / 6) % 2 ? -4 : 0;
        break;
    }
    case ANIM_ALERT:
        // shake in bursts
        if ((tick % 12) < 4) dx = (tick % 2) ? 4 : -4;
        break;
    case ANIM_SLEEP:
        lv_obj_set_y(c->zzz, 2 - (int)((tick / 5) % 3) * 2);
        break;
    default:
        break;
    }
    if (c->cur.state == CARD_NEEDS && tick % 3 == 0) {
        // pulse the pixel glow
        c->frame.glow_opa = (tick / 3) % 2 ? LV_OPA_80 : LV_OPA_40;
        pixframe_refresh(c->root);
    }
    lv_obj_set_pos(c->mascot, c->mascot_x + dx, c->mascot_y + dy);
}
