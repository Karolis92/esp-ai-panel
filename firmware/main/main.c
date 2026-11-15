// AI Panel firmware — Claude Code session status display for the Guition ESP32-S3-4848S040.
#include <stdlib.h>

#include "board.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "model.h"
#include "serial_link.h"
#include "ui.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "AI Panel fw " FW_VERSION);
    model_init();

    ESP_ERROR_CHECK(board_backlight_init());
    board_backlight_set(0);

    esp_lcd_panel_handle_t panel = NULL;
    ESP_ERROR_CHECK(board_display_init(&panel));

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .panel_handle = panel,
        .buffer_size = LCD_H_RES * LCD_V_RES,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {.swap_xy = false, .mirror_x = false, .mirror_y = false},
        .flags = {
            .buff_spiram = true,
            .direct_mode = true,
        },
    };
    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
            .bb_mode = true,
            .avoid_tearing = true,
        },
    };
    lv_display_t *disp = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg);
    if (!disp) {
        ESP_LOGE(TAG, "lvgl display init failed");
        abort();
    }

    esp_lcd_touch_handle_t touch = NULL;
    if (board_touch_init(&touch) == ESP_OK) {
        const lvgl_port_touch_cfg_t touch_cfg = {.disp = disp, .handle = touch};
        lvgl_port_add_touch(&touch_cfg);
    }

    lvgl_port_lock(0);
    ui_init();
    lvgl_port_unlock();

    board_backlight_set(100);
    serial_link_start();

    // Keep the console quiet so log lines rarely interleave with protocol traffic.
    esp_log_level_set("*", ESP_LOG_WARN);
}
