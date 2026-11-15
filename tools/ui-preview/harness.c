// Host-side UI preview: runs the firmware UI (ui/, link/model.c) on desktop LVGL with a
// memory framebuffer and dumps screenshots. See build.sh. Edit the fake sessions below freely.
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "lvgl.h"
#include "model.h"
#include "ui.h"

static int64_t s_fake_us;
int64_t esp_timer_get_time(void) { return s_fake_us; }
void board_backlight_set(int p) { (void)p; }
void serial_link_send(const char *j) { printf("TX %s\n", j); }

static uint32_t fb[480 * 480];
static uint32_t drawbuf[480 * 480];

static void flush_cb(lv_display_t *d, const lv_area_t *a, uint8_t *px)
{
    const uint32_t *src = (const uint32_t *)px;
    const int w = lv_area_get_width(a);
    for (int y = a->y1; y <= a->y2; y++) {
        memcpy(&fb[y * 480 + a->x1], &src[(y - a->y1) * w], w * 4);
    }
    lv_display_flush_ready(d);
}

static void dump(const char *path)
{
    FILE *f = fopen(path, "wb");
    const int W = 480, H = 480, row = W * 3, size = 54 + row * H;
    unsigned char hdr[54] = {'B', 'M'};
    *(int *)&hdr[2] = size; *(int *)&hdr[10] = 54; *(int *)&hdr[14] = 40;
    *(int *)&hdr[18] = W; *(int *)&hdr[22] = -H; *(short *)&hdr[26] = 1; *(short *)&hdr[28] = 24;
    fwrite(hdr, 1, 54, f);
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            uint32_t p = fb[y * W + x];
            unsigned char bgr[3] = {p & 0xff, (p >> 8) & 0xff, (p >> 16) & 0xff};
            fwrite(bgr, 1, 3, f);
        }
    fclose(f);
}

static void run(int ms)
{
    for (int t = 0; t < ms; t += 5) {
        s_fake_us += 5000;
        lv_tick_inc(5);
        lv_timer_handler();
    }
}

static void set(session_t *s, const char *id, const char *title, const char *act, card_state_t st, anim_t an)
{
    strcpy(s->id, id); strcpy(s->title, title); strcpy(s->activity, act); s->state = st; s->anim = an;
}

int main(void)
{
    lv_init();
    lv_display_t *d = lv_display_create(480, 480);
    lv_display_set_buffers(d, drawbuf, NULL, sizeof(drawbuf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(d, flush_cb);
    s_fake_us = 1000000;
    model_init();
    ui_init();
    run(300);
    dump("0_waiting.bmp");

    model_t *m = model_get();
    set(&m->sessions[0], "d2", "REVIEW PR", "Needs you", CARD_NEEDS, ANIM_ALERT);
    set(&m->sessions[1], "d1", "REFACTOR", "Editing auth.ts", CARD_WORKING, ANIM_LAPTOP);
    set(&m->sessions[2], "d3", "WRITE TESTS", "Running npm test", CARD_WORKING, ANIM_TERMINAL);
    set(&m->sessions[3], "d4", "PLAN FEATURE", "Planning...", CARD_WORKING, ANIM_PLAN);
    set(&m->sessions[4], "d5", "RESEARCH", "Researching...", CARD_WORKING, ANIM_BOOK);
    set(&m->sessions[5], "d6", "IDLE", "Waiting...", CARD_IDLE, ANIM_SLEEP);
    set(&m->sessions[6], "d7", "ESP-AI-PANEL", "Thinking...", CARD_WORKING, ANIM_THINKING);
    set(&m->sessions[7], "d8", "A-REALLY-LONG-PROJECT", "Error", CARD_ERROR, ANIM_ERROR);
    m->n_sessions = 8;
    m->s5h = (usage_window_t){true, 45, 1000000 + 2 * 3600 + 15 * 60};
    m->wk = (usage_window_t){true, 28, 1000000 + 5 * 86400 + 8 * 3600};
    m->host_now = 1000000;
    m->host_now_at_us = s_fake_us;
    m->last_rx_us = s_fake_us;
    m->version++;
    run(400);
    dump("1_page1.bmp");

    // swipe-equivalent: tap next
    lv_obj_t *scr = lv_screen_active();
    (void)scr;
    run(10);
    // simulate page 2 by marking nothing new and calling the next-button via event
    for (uint32_t i = 0; i < lv_obj_get_child_count(scr); i++) {
        lv_obj_t *c = lv_obj_get_child(scr, i);
        if (lv_obj_get_x(c) > 300 && lv_obj_get_y(c) > 290 && lv_obj_get_y(c) < 300) {
            lv_obj_send_event(c, LV_EVENT_CLICKED, NULL);
        }
    }
    run(300);
    dump("2_page2.bmp");

    m->n_sessions = 0;
    m->s5h.valid = false;
    m->version++;
    run(300);
    dump("3_empty.bmp");
    return 0;
}
