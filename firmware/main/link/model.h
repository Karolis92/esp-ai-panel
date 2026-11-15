#pragma once
// Display model: what the host last told us. Written by the serial link task,
// read by the UI (LVGL task) — always access through model_lock()/model_unlock().

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MODEL_MAX_SESSIONS 18

typedef enum {
    CARD_WORKING = 0,
    CARD_NEEDS,
    CARD_IDLE,
    CARD_DONE,
    CARD_ERROR,
} card_state_t;

typedef enum {
    ANIM_LAPTOP = 0,
    ANIM_BOOK,
    ANIM_COFFEE,
    ANIM_PLAN,
    ANIM_THINKING,
    ANIM_TERMINAL,
    ANIM_SLEEP,
    ANIM_ALERT,
    ANIM_DONE,
    ANIM_ERROR,
} anim_t;

typedef struct {
    char id[16];
    char title[32];
    char activity[48];
    card_state_t state;
    anim_t anim;
} session_t;

typedef struct {
    bool valid;
    int pct;
    int64_t reset; // unix epoch seconds, 0 = unknown
} usage_window_t;

typedef struct {
    session_t sessions[MODEL_MAX_SESSIONS];
    int n_sessions;
    usage_window_t s5h;
    usage_window_t wk;
    int64_t host_now;        // host unix time at last message
    int64_t host_now_at_us;  // esp_timer_get_time() when host_now was received
    int64_t last_rx_us;      // last valid message from host (0 = never)
    uint32_t version;        // bumped on every state update
} model_t;

void model_init(void);
void model_lock(void);
void model_unlock(void);
/** Direct access; only while holding the lock. */
model_t *model_get(void);

/** Host unix time now (estimated from the last message), or 0 if unknown. */
int64_t model_host_time_now(const model_t *m);

/** Copy `src` into `dst` keeping only ASCII (U+2026 "…" becomes "..."). */
void model_sanitize(char *dst, size_t dst_size, const char *src);
