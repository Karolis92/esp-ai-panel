// Host link over UART0 (CH340 USB bridge): newline-delimited JSON, see protocol/PROTOCOL.md.
#include "serial_link.h"

#include <string.h>

#include "board.h"
#include "cJSON.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "model.h"

static const char *TAG = "link";

#define LINK_LINE_MAX 4096

static SemaphoreHandle_t s_tx_mutex;

void serial_link_send(const char *json)
{
    xSemaphoreTake(s_tx_mutex, portMAX_DELAY);
    uart_write_bytes(LINK_UART_NUM, "\n", 1); // terminate any partial log line first
    uart_write_bytes(LINK_UART_NUM, json, strlen(json));
    uart_write_bytes(LINK_UART_NUM, "\n", 1);
    xSemaphoreGive(s_tx_mutex);
}

static void send_hello(void)
{
    serial_link_send("{\"t\":\"hello\",\"fw\":\"" FW_VERSION "\"}");
}

static card_state_t parse_state(const char *s)
{
    if (!s) return CARD_IDLE;
    if (!strcmp(s, "working")) return CARD_WORKING;
    if (!strcmp(s, "needs")) return CARD_NEEDS;
    if (!strcmp(s, "done")) return CARD_DONE;
    if (!strcmp(s, "error")) return CARD_ERROR;
    return CARD_IDLE;
}

static anim_t parse_anim(const char *s)
{
    if (!s) return ANIM_SLEEP;
    if (!strcmp(s, "laptop")) return ANIM_LAPTOP;
    if (!strcmp(s, "book")) return ANIM_BOOK;
    if (!strcmp(s, "coffee")) return ANIM_COFFEE;
    if (!strcmp(s, "plan") || !strcmp(s, "headphones")) return ANIM_PLAN;
    if (!strcmp(s, "thinking")) return ANIM_THINKING;
    if (!strcmp(s, "terminal")) return ANIM_TERMINAL;
    if (!strcmp(s, "alert")) return ANIM_ALERT;
    if (!strcmp(s, "done")) return ANIM_DONE;
    if (!strcmp(s, "error")) return ANIM_ERROR;
    return ANIM_SLEEP;
}

static const char *str_or(const cJSON *obj, const char *key, const char *dflt)
{
    const cJSON *v = cJSON_GetObjectItemCaseSensitive(obj, key);
    return cJSON_IsString(v) ? v->valuestring : dflt;
}

static void parse_usage(const cJSON *usage, const char *key, usage_window_t *out)
{
    const cJSON *w = usage ? cJSON_GetObjectItemCaseSensitive(usage, key) : NULL;
    const cJSON *pct = w ? cJSON_GetObjectItemCaseSensitive(w, "pct") : NULL;
    if (!cJSON_IsObject(w) || !cJSON_IsNumber(pct)) {
        out->valid = false;
        return;
    }
    const cJSON *reset = cJSON_GetObjectItemCaseSensitive(w, "reset");
    out->valid = true;
    out->pct = pct->valueint < 0 ? 0 : pct->valueint > 100 ? 100 : pct->valueint;
    out->reset = cJSON_IsNumber(reset) ? (int64_t)reset->valuedouble : 0;
}

static void handle_line(const char *line)
{
    cJSON *root = cJSON_Parse(line);
    if (!root) {
        return;
    }
    const char *t = str_or(root, "t", "");
    const cJSON *now = cJSON_GetObjectItemCaseSensitive(root, "now");
    const int64_t now_us = esp_timer_get_time();

    if (!strcmp(t, "state")) {
        model_lock();
        model_t *m = model_get();
        const cJSON *sessions = cJSON_GetObjectItemCaseSensitive(root, "sessions");
        int n = 0;
        const cJSON *s;
        cJSON_ArrayForEach(s, sessions) {
            if (n >= MODEL_MAX_SESSIONS) break;
            session_t *d = &m->sessions[n++];
            model_sanitize(d->id, sizeof(d->id), str_or(s, "id", ""));
            model_sanitize(d->title, sizeof(d->title), str_or(s, "title", "CLAUDE"));
            model_sanitize(d->activity, sizeof(d->activity), str_or(s, "activity", ""));
            d->state = parse_state(str_or(s, "state", NULL));
            d->anim = parse_anim(str_or(s, "anim", NULL));
        }
        m->n_sessions = n;
        const cJSON *usage = cJSON_GetObjectItemCaseSensitive(root, "usage");
        parse_usage(usage, "s5h", &m->s5h);
        parse_usage(usage, "wk", &m->wk);
        if (cJSON_IsNumber(now)) {
            m->host_now = (int64_t)now->valuedouble;
            m->host_now_at_us = now_us;
        }
        m->last_rx_us = now_us;
        m->version++;
        model_unlock();
    } else if (!strcmp(t, "ping")) {
        model_lock();
        model_t *m = model_get();
        if (cJSON_IsNumber(now)) {
            m->host_now = (int64_t)now->valuedouble;
            m->host_now_at_us = now_us;
        }
        m->last_rx_us = now_us;
        model_unlock();
        send_hello();
    }
    cJSON_Delete(root);
}

static void link_task(void *arg)
{
    static char line[LINK_LINE_MAX];
    static uint8_t buf[512];
    size_t len = 0;
    bool overflow = false;

    send_hello();
    while (true) {
        const int n = uart_read_bytes(LINK_UART_NUM, buf, sizeof(buf), pdMS_TO_TICKS(50));
        for (int i = 0; i < n; i++) {
            const char c = (char)buf[i];
            if (c == '\n' || c == '\r') {
                if (len > 0 && !overflow) {
                    line[len] = '\0';
                    if (line[0] == '{') {
                        handle_line(line);
                    }
                }
                len = 0;
                overflow = false;
            } else if (len + 1 < LINK_LINE_MAX) {
                line[len++] = c;
            } else {
                overflow = true; // drop the rest of an oversized line
            }
        }
    }
}

void serial_link_start(void)
{
    s_tx_mutex = xSemaphoreCreateMutex();
    const uart_config_t cfg = {
        .baud_rate = LINK_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_driver_install(LINK_UART_NUM, 16 * 1024, 2048, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(LINK_UART_NUM, &cfg));
    xTaskCreatePinnedToCore(link_task, "link", 6144, NULL, 5, NULL, 0);
    ESP_LOGI(TAG, "link up on UART%d @ %d", LINK_UART_NUM, LINK_BAUD);
}
