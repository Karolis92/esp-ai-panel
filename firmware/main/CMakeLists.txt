file(GLOB UI_FONTS "ui/fonts/*.c")

idf_component_register(
    SRCS
        "main.c"
        "board/display.c"
        "board/touch.c"
        "board/backlight.c"
        "link/model.c"
        "link/serial_link.c"
        "ui/sprites.c"
        "ui/pixframe.c"
        "ui/card.c"
        "ui/tile.c"
        "ui/ui.c"
        ${UI_FONTS}
    INCLUDE_DIRS "." "board" "link" "ui"
    REQUIRES
        esp_lcd
        esp_timer
        esp_driver_uart
        esp_driver_i2c
        esp_driver_ledc
        esp_driver_gpio
        esp_psram
        json
)
