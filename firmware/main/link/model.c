#include "model.h"

#include <string.h>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static model_t s_model;
static SemaphoreHandle_t s_mutex;

void model_init(void)
{
    memset(&s_model, 0, sizeof(s_model));
    s_mutex = xSemaphoreCreateMutex();
}

void model_lock(void) { xSemaphoreTake(s_mutex, portMAX_DELAY); }
void model_unlock(void) { xSemaphoreGive(s_mutex); }
model_t *model_get(void) { return &s_model; }

int64_t model_host_time_now(const model_t *m)
{
    if (m->host_now == 0) {
        return 0;
    }
    return m->host_now + (esp_timer_get_time() - m->host_now_at_us) / 1000000;
}

void model_sanitize(char *dst, size_t dst_size, const char *src)
{
    size_t o = 0;
    if (dst_size == 0) {
        return;
    }
    for (const unsigned char *p = (const unsigned char *)src; p && *p && o + 1 < dst_size;) {
        if (*p >= 0x20 && *p < 0x7F) {
            dst[o++] = (char)*p++;
        } else if (p[0] == 0xE2 && p[1] == 0x80 && p[2] == 0xA6) { // "…" → "..." (body font is ASCII-only)
            if (o + 3 >= dst_size) {
                break;
            }
            memcpy(dst + o, "...", 3);
            o += 3;
            p += 3;
        } else {
            // Skip the whole UTF-8 sequence, emit one placeholder.
            int len = (*p >= 0xF0) ? 4 : (*p >= 0xE0) ? 3 : (*p >= 0xC0) ? 2 : 1;
            for (int i = 0; i < len && *p; i++) {
                p++;
            }
            dst[o++] = '?';
        }
    }
    dst[o] = '\0';
}
