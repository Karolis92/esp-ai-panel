# AI Panel: notes for AI assistants

This repo is a USB-only desk display (Guition ESP32-S3-4848S040, 480×480) that shows Claude Code
sessions. `README.md` covers the user-facing setup. This file covers what you need when *changing* the code.

- `firmware/` is the ESP-IDF 5.4 + LVGL 9.3 project.
- `companion/` is the Node/TS CLI `ai-panel`: daemon, hook forwarder, statusLine, installer, demo.
- `protocol/PROTOCOL.md` documents the NDJSON wire format between the companion and the panel.
- `tools/ui-preview/` renders the firmware UI on the Mac without the board.

## Commands

```sh
# companion
cd companion && npm test          # vitest: state machine + settings.json merge
npm run typecheck && npm run build
node dist/cli.js demo             # feed fake sessions to the panel (holds the serial port)
node dist/cli.js daemon -v        # real daemon, verbose

# firmware (every new shell needs the IDF env first)
. ~/esp/esp-idf/export.sh
cd firmware && idf.py -p /dev/cu.usbserial-110 flash      # build + flash
idf.py reconfigure                # REQUIRED after adding/removing .c files (fonts are GLOBbed)

# UI without hardware: compiles firmware/main/ui/* against desktop LVGL → PNG screenshots
tools/ui-preview/build.sh         # writes tools/ui-preview/out/*.png; Read them to check layout
```

Only one process can hold the serial port at a time (`demo`, `daemon`, `idf.py monitor`, `flash`).
Stop the others before flashing (`pkill -f "dist/cli.js demo"`).

## Workflow for UI changes

1. Edit `firmware/main/ui/*`. Run `tools/ui-preview/build.sh` and look at the PNGs.
   It catches LVGL API mistakes and layout problems in seconds. The fake sessions are in `harness.c`.
2. Flash, then run `demo` and ask the user to check the real panel. The panel's colors and
   sharpness differ from the preview.

## Hardware facts (verified on the real board; don't "fix" these back)

- **Display init** (`board/display.c`): use the manufacturer's `st7701_type9` sequence, with
  `0xCD = 0x00` and **no** `0x21` inversion. The generic "type1" sequence (`CD=0x08`, `0x21`)
  scrambles the color bits on this board.
- **Data pins**: `{B0..B4, G0..G5, R0..R4}` = `{4,5,6,7,15, 8,20,3,46,9,10, 11,12,13,14,0}`.
  The LCD control pins and touch are in `board/board.h`.
- **USB is a CH340 bridge (VID 0x1A86) to UART0**, not native USB. GPIO19/20 are used by touch
  and the LCD.
- **Link baud is 115200**, the same as the ROM and console. 921600 did not work through the macOS
  CH340 driver. `CONFIG_ESP_CONSOLE_UART_BAUDRATE` in sdkconfig.defaults is ignored unless the
  UART is "custom".
- The companion releases **DTR/RTS** after opening the port, because the auto-reset circuit
  otherwise holds the chip in reset or download mode.
- The console logs share the link. The host ignores non-JSON lines, and the firmware sets the log
  level to WARN after boot.
- GT911 touch: the firmware tries I2C address 0x5D, then 0x14.
- `esp_lvgl_port` 2.6 and later needs LVGL 9.3 or newer (`LV_COLOR_FORMAT_RGB565_SWAPPED`).

## Visual style rules (pixel-art look, matching design.png)

- **Don't use LVGL `radius` or `shadow` for frames.** Use `ui/pixframe.c`
  (`pixframe_attach(obj, &pf)`), which draws stepped corners, pixel borders and pixel glow rings.
  The `pixframe_t` must outlive the object, so store it in a struct or a static.
- **Sprites** are ASCII grids in `ui/sprites.c` with `DEF(name, scale, rows...)`. One character
  is one art pixel, and the palette is in `pal()`. Every row must be the same length. To add a
  sprite: add an enum in `sprites.h`, a `DEF`, and an entry in `s_defs[]`.
- **Fonts** are pixel fonts rendered at **integer multiples of their pixel grid**, with `--bpp 1`
  (no antialiasing):
  - `font_pixel_16/24`: Silkscreen Bold. Titles and labels, uppercase look.
  - `font_pixel_48`: Silkscreen Regular, digits and `%` only. The Bold weight loses its counters
    at 48 px.
  - `font_body`: Jersey 10 at 20 px. Status text. **ASCII only**: `model_sanitize()` turns "…"
    into "...".
  - Regenerate with
    `npx lv_font_conv@1.5.3 --no-compress --bpp 1 --format lvgl --lv-include lvgl.h --font X.ttf -r 0x20-0x7E --size N -o firmware/main/ui/fonts/font_X.c --lv-font-name font_X`.
    Then strip the absolute path from the header comment and add the font's OFL text to
    `ui/fonts/`.
- Long labels scroll back and forth (`LV_LABEL_LONG_SCROLL`) at `SCROLL_SPEED` (15 px/s).
  `lv_anim_speed_clamped` caps the max time at 10000.
- Layout constants are in `ui/theme.h`. The 3 px card border is the object's padding, so
  children use content coordinates. A card's inner area is `CARD_W-6 × CARD_H-6`.

## Session → scene logic (companion/src/sessions.ts)

- Hook events map to `state` (working, needs, idle, done, error) and `anim` (thinking, laptop,
  terminal, coffee, book, plan, sleep, alert, done, error). The table is in README.md.
- **Hold time**: a working→working scene change waits until the current scene has been visible
  for `SCENE_HOLD_MS` (2 s). The latest pending scene wins. Needs, done, idle and error show
  immediately.
- A Bash `terminal` scene turns into `coffee` after `LONG_COMMAND_MS` (10 s). This doesn't reset
  the hold timer.
- Async hooks can arrive out of order. `PostToolUse` only ends the scene if its `tool_use_id`
  matches the current tool.
- Any behavior change needs a test in `companion/test/sessions.test.ts`. Use the injected clock
  (`advance()`).

## Changing the wire protocol

Keep these four in sync:
- `companion/src/protocol.ts` (types, limits)
- `protocol/PROTOCOL.md`
- `firmware/main/link/serial_link.c` (`parse_state`, `parse_anim`, JSON fields)
- `firmware/main/link/model.h` (enums, buffer sizes)

Unknown anim strings fall back to `sleep` on the device. A new anim also needs a case in
`layout_scene()` and `card_animate()` in `ui/card.c`.

## Things to ask before doing

- `ai-panel install` / `uninstall`: they edit the user's `~/.claude/settings.json` (backed up
  first) and load or unload a launchd agent.
- Flashing is fine when the user is iterating on the panel. Mention it when you do it.
