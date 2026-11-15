#include "board.h"

#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_log.h"

static const char *TAG = "touch";

esp_err_t board_touch_init(esp_lcd_touch_handle_t *out_touch)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_TOUCH_SDA,
        .scl_io_num = PIN_TOUCH_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus = NULL;
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &bus), TAG, "i2c bus");

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_H_RES,
        .y_max = LCD_V_RES,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .levels = {.reset = 0, .interrupt = 0},
        .flags = {.swap_xy = 0, .mirror_x = 0, .mirror_y = 0},
    };

    // Without RST/INT control the GT911 latches either 0x5D or 0x14 at power-up; try both.
    const uint32_t addrs[] = {0x5D, 0x14};
    for (size_t i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
        esp_lcd_panel_io_i2c_config_t io_cfg = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
        io_cfg.dev_addr = addrs[i];
        io_cfg.scl_speed_hz = 400000;
        esp_lcd_panel_io_handle_t io = NULL;
        if (esp_lcd_new_panel_io_i2c(bus, &io_cfg, &io) != ESP_OK) {
            continue;
        }
        if (esp_lcd_touch_new_i2c_gt911(io, &tp_cfg, out_touch) == ESP_OK) {
            ESP_LOGI(TAG, "GT911 at 0x%02lx", (unsigned long)addrs[i]);
            return ESP_OK;
        }
        esp_lcd_panel_io_del(io);
    }
    ESP_LOGE(TAG, "GT911 not found");
    return ESP_ERR_NOT_FOUND;
}
