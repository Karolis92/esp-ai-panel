#pragma once
// Guition ESP32-S3-4848S040 (480x480 ST7701S RGB panel, GT911 touch, CH340 USB-UART)

#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"

#define LCD_H_RES 480
#define LCD_V_RES 480

// ST7701S 3-wire SPI (init only)
#define PIN_LCD_CS    39
#define PIN_LCD_SCK   48
#define PIN_LCD_SDA   47

// RGB interface
#define PIN_LCD_DE    18
#define PIN_LCD_VSYNC 17
#define PIN_LCD_HSYNC 16
#define PIN_LCD_PCLK  21
// data_gpio_nums order for RGB565: B0..B4, G0..G5, R0..R4
#define PIN_LCD_DATA  {4, 5, 6, 7, 15,  8, 20, 3, 46, 9, 10,  11, 12, 13, 14, 0}

#define PIN_LCD_BL    38

// GT911 touch (I2C). INT/RST are not wired to GPIOs on this board.
#define PIN_TOUCH_SDA 19
#define PIN_TOUCH_SCL 45

// USB link: CH340 → UART0
#define LINK_UART_NUM  0
#define LINK_BAUD      115200 // same as console + ROM; CH340 on macOS is unreliable above this

esp_err_t board_display_init(esp_lcd_panel_handle_t *out_panel);
esp_err_t board_touch_init(esp_lcd_touch_handle_t *out_touch);
esp_err_t board_backlight_init(void);
/** 0..100 % */
void board_backlight_set(int percent);
