// Main screen: 3x2 session cards, pager, usage tiles; plus the "waiting for host" overlay.
// Everything here runs in the LVGL task (timers / events); the model is read under its lock.
#include "ui.h"

#include <string.h>

#include "board.h"
#include "card.h"
#include "esp_timer.h"
#include "model.h"
#include "pixframe.h"
#include "sprites.h"
#include "theme.h"
#include "tile.h"

#define REFRESH_MS        100
#define ANIM_PERIOD_MS    150
#define HOST_TIMEOUT_US   (15LL * 1000 * 1000)
#define DIM_AFTER_MS      (10 * 60 * 1000)
#define BL_ON             100
#define BL_DIM            12
#define MAX_PAGES         ((MODEL_MAX_SESSIONS + CARDS_PER_PAGE - 1) / CARDS_PER_PAGE)

static card_t s_cards[CARDS_PER_PAGE];
static tile_t s_tile_5h, s_tile_wk;
static lv_obj_t *s_prev, *s_next, *s_page_lbl, *s_dots[MAX_PAGES];
static pixframe_t s_nav_frames[2], s_dot_frames[MAX_PAGES];
static int s_n_nav;
static lv_obj_t *s_empty_lbl;
static lv_obj_t *s_overlay;

static model_t s_snap;           // UI-side copy of the model
static uint32_t s_seen_version = UINT32_MAX;
static int s_page, s_pages = 1;
static char s_needs_ids[MODEL_MAX_SESSIONS][16];
static int s_n_needs;
static int64_t s_last_activity_us;
static int s_backlight = -1;

// ---------------------------------------------------------------------------

static void render_page(void)
{
    for (int i = 0; i < CARDS_PER_PAGE; i++) {
        const int idx = s_page * CARDS_PER_PAGE + i;
        card_set(&s_cards[i], idx < s_snap.n_sessions ? &s_snap.sessions[idx] : NULL);
    }
    lv_label_set_text_fmt(s_page_lbl, "%d/%d", s_page + 1, s_pages);
    for (int i = 0; i < MAX_PAGES; i++) {
        if (i < s_pages) lv_obj_remove_flag(s_dots[i], LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(s_dots[i], LV_OBJ_FLAG_HIDDEN);
        s_dot_frames[i].fill = s_dot_frames[i].border = i == s_page ? COL_ORANGE_HI : COL_DOT_OFF;
        pixframe_refresh(s_dots[i]);
    }
    const lv_opa_t nav = s_pages > 1 ? LV_OPA_COVER : LV_OPA_30;
    lv_obj_set_style_opa(s_prev, nav, 0);
    lv_obj_set_style_opa(s_next, nav, 0);
    if (s_snap.n_sessions == 0) lv_obj_remove_flag(s_empty_lbl, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(s_empty_lbl, LV_OBJ_FLAG_HIDDEN);
}

static void go_page(int p)
{
    if (s_pages <= 1) {
        p = 0;
    } else {
        p = (p % s_pages + s_pages) % s_pages; // wrap around
    }
    if (p != s_page) {
        s_page = p;
        render_page();
    }
}

static bool was_needing(const char *id)
{
    for (int i = 0; i < s_n_needs; i++) {
        if (!strcmp(s_needs_ids[i], id)) return true;
    }
    return false;
}

static void apply_snapshot(void)
{
    s_pages = s_snap.n_sessions == 0 ? 1 : (s_snap.n_sessions + CARDS_PER_PAGE - 1) / CARDS_PER_PAGE;
    if (s_page >= s_pages) s_page = s_pages - 1;

    // Auto-jump to a card that just started needing attention.
    int jump = -1;
    bool any_active = false;
    int n_needs = 0;
    static char needs_now[MODEL_MAX_SESSIONS][16];
    for (int i = 0; i < s_snap.n_sessions; i++) {
        const session_t *s = &s_snap.sessions[i];
        if (s->state != CARD_IDLE) any_active = true;
        if (s->state == CARD_NEEDS) {
            if (jump < 0 && !was_needing(s->id)) jump = i / CARDS_PER_PAGE;
            strcpy(needs_now[n_needs++], s->id);
        }
    }
    memcpy(s_needs_ids, needs_now, sizeof(needs_now));
    s_n_needs = n_needs;
    if (jump >= 0) s_page = jump;
    if (any_active || jump >= 0) s_last_activity_us = esp_timer_get_time();

    render_page();
}

static void update_backlight(void)
{
    const int64_t idle_ms = (esp_timer_get_time() - s_last_activity_us) / 1000;
    const uint32_t touch_idle_ms = lv_display_get_inactive_time(NULL);
    const bool dim = idle_ms > DIM_AFTER_MS && touch_idle_ms > DIM_AFTER_MS;
    const int want = dim ? BL_DIM : BL_ON;
    if (want != s_backlight) {
        s_backlight = want;
        board_backlight_set(want);
    }
}

static void refresh_timer_cb(lv_timer_t *t)
{
    (void)t;
    bool changed = false;
    model_lock();
    const model_t *m = model_get();
    const bool connected = m->last_rx_us != 0 && (esp_timer_get_time() - m->last_rx_us) < HOST_TIMEOUT_US;
    if (m->version != s_seen_version) {
        memcpy(&s_snap, m, sizeof(s_snap));
        s_seen_version = m->version;
        changed = true;
    } else {
        s_snap.host_now = m->host_now;
        s_snap.host_now_at_us = m->host_now_at_us;
    }
    model_unlock();

    if (changed) {
        apply_snapshot();
    }
    const int64_t now = model_host_time_now(&s_snap);
    tile_set(&s_tile_5h, &s_snap.s5h, now);
    tile_set(&s_tile_wk, &s_snap.wk, now);

    if (connected == lv_obj_has_flag(s_overlay, LV_OBJ_FLAG_HIDDEN)) {
        // nothing to do: overlay hidden while connected / shown while disconnected
    } else if (connected) {
        lv_obj_add_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
        s_last_activity_us = esp_timer_get_time();
    } else {
        lv_obj_remove_flag(s_overlay, LV_OBJ_FLAG_HIDDEN);
    }
    update_backlight();
}

static void anim_timer_cb(lv_timer_t *t)
{
    (void)t;
    static uint32_t tick;
    tick++;
    if (!lv_obj_has_flag(s_overlay, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }
    for (int i = 0; i < CARDS_PER_PAGE; i++) {
        card_animate(&s_cards[i], tick);
    }
}

static void on_prev(lv_event_t *e) { (void)e; go_page(s_page - 1); }
static void on_next(lv_event_t *e) { (void)e; go_page(s_page + 1); }

static void on_gesture(lv_event_t *e)
{
    (void)e;
    const lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_LEFT) go_page(s_page + 1);
    else if (dir == LV_DIR_RIGHT) go_page(s_page - 1);
}

// ---------------------------------------------------------------------------

static lv_obj_t *nav_button(lv_obj_t *parent, int x, const char *txt, lv_event_cb_t cb)
{
    lv_obj_t *b = lv_obj_create(parent);
    lv_obj_remove_flag(b, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(b, x, PAGER_Y);
    lv_obj_set_size(b, 130, PAGER_H);
    lv_obj_set_style_pad_all(b, 0, 0);
    pixframe_t *pf = &s_nav_frames[s_n_nav++];
    *pf = pixframe_default();
    pf->fill_pressed = COL_TRACK;
    pixframe_attach(b, pf);
    lv_obj_set_ext_click_area(b, 6);
    lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *l = lv_label_create(b);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_font(l, &font_pixel_24, 0);
    lv_obj_set_style_text_color(l, COL_TEXT, 0);
    lv_obj_center(l);
    return b;
}

static void build_overlay(void)
{
    s_overlay = lv_obj_create(lv_layer_top());
    lv_obj_remove_flag(s_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(s_overlay, LCD_H_RES, LCD_V_RES);
    lv_obj_set_style_radius(s_overlay, 0, 0);
    lv_obj_set_style_bg_color(s_overlay, COL_BG, 0);
    lv_obj_set_style_bg_opa(s_overlay, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_overlay, 3, 0);
    lv_obj_set_style_border_color(s_overlay, COL_ORANGE, 0);

    lv_obj_t *m = lv_image_create(s_overlay);
    lv_image_set_src(m, sprite(SPR_MASCOT_SLEEP));
    lv_obj_align(m, LV_ALIGN_CENTER, 0, -70);

    lv_obj_t *z = lv_label_create(s_overlay);
    lv_label_set_text(z, "z Z Z");
    lv_obj_set_style_text_font(z, &font_pixel_16, 0);
    lv_obj_set_style_text_color(z, COL_TEXT, 0);
    lv_obj_align(z, LV_ALIGN_CENTER, 70, -110);

    lv_obj_t *t = lv_label_create(s_overlay);
    lv_label_set_text(t, "WAITING FOR HOST");
    lv_obj_set_style_text_font(t, &font_pixel_24, 0);
    lv_obj_set_style_text_color(t, COL_TEXT, 0);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *s = lv_label_create(s_overlay);
    lv_label_set_text(s, "Connect USB and run: ai-panel daemon");
    lv_obj_set_style_text_font(s, &font_body, 0);
    lv_obj_set_style_text_color(s, COL_TEXT_MUTED, 0);
    lv_obj_align(s, LV_ALIGN_CENTER, 0, 36);
}

void ui_init(void)
{
    sprites_init();
    s_last_activity_us = esp_timer_get_time();

    lv_obj_t *scr = lv_screen_active();
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(scr, COL_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(scr, on_gesture, LV_EVENT_GESTURE, NULL);

    for (int i = 0; i < CARDS_PER_PAGE; i++) {
        const int col = i % 3, row = i / 3;
        card_create(&s_cards[i], scr, UI_MARGIN + col * (CARD_W + CARD_GAP), UI_MARGIN + row * (CARD_H + ROW_GAP));
    }

    s_empty_lbl = lv_label_create(scr);
    lv_label_set_text(s_empty_lbl, "NO CLAUDE SESSIONS\n\nstart `claude` in a terminal");
    lv_obj_set_style_text_align(s_empty_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(s_empty_lbl, &font_pixel_16, 0);
    lv_obj_set_style_text_color(s_empty_lbl, COL_TEXT, 0);
    lv_obj_align(s_empty_lbl, LV_ALIGN_TOP_MID, 0, UI_MARGIN + CARD_H - 20);

    s_prev = nav_button(scr, UI_MARGIN, "<", on_prev);
    s_next = nav_button(scr, LCD_H_RES - UI_MARGIN - 130, ">", on_next);

    lv_obj_t *mid = lv_obj_create(scr);
    lv_obj_remove_style_all(mid);
    lv_obj_remove_flag(mid, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(mid, LCD_H_RES - 2 * (UI_MARGIN + 130 + 6), PAGER_H);
    lv_obj_set_pos(mid, UI_MARGIN + 130 + 6, PAGER_Y);
    lv_obj_set_flex_flow(mid, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(mid, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(mid, 10, 0);
    for (int i = 0; i < MAX_PAGES; i++) {
        s_dots[i] = lv_obj_create(mid);
        lv_obj_remove_style_all(s_dots[i]);
        lv_obj_remove_flag(s_dots[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(s_dots[i], 18, 18);
        s_dot_frames[i] = pixframe_default();
        s_dot_frames[i].steps = 2; // 6x6 pixel octagon
        pixframe_attach(s_dots[i], &s_dot_frames[i]);
    }
    s_page_lbl = lv_label_create(mid);
    lv_obj_set_style_text_font(s_page_lbl, &font_pixel_24, 0);
    lv_obj_set_style_text_color(s_page_lbl, COL_TEXT, 0);
    lv_obj_set_style_pad_left(s_page_lbl, 8, 0);

    tile_create(&s_tile_5h, scr, UI_MARGIN, TILES_Y, "SESSION (5H)", SPR_CLOCK);
    tile_create(&s_tile_wk, scr, LCD_H_RES - UI_MARGIN - TILE_W, TILES_Y, "WEEKLY", SPR_CALENDAR);

    build_overlay();
    render_page();

    lv_timer_create(refresh_timer_cb, REFRESH_MS, NULL);
    lv_timer_create(anim_timer_cb, ANIM_PERIOD_MS, NULL);
}
