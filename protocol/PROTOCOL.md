# AI Panel wire protocol (v1)

Transport: USB serial (CH340 → ESP32-S3 UART0), **115200 baud, 8N1**.
Framing: newline-delimited JSON (`\n`), one object per line, max **4096 bytes** per line.
Every message has a `"t"` (type) field. Unknown types/fields are ignored.

**Handshake:** after opening a port, the host only sends `ping` (once a second) until the device
replies with `hello`. If no `hello` arrives within 4 s, the host closes the port and tries the next
candidate, so other USB-serial devices never receive session data. `state` is sent only after `hello`.

## Host → device

### `state` — full snapshot (sent on every change, debounced ~100 ms, and on `hello`)
```json
{
  "t": "state",
  "now": 1763130000,
  "sessions": [
    {
      "id": "8c1f…",          // stable id (first 8 chars of session_id are enough)
      "title": "ESP-AI-PANEL", // project folder name, uppercased, ≤ 24 chars (scrolls if wide)
      "activity": "Editing main.c", // what it is doing now, ≤ 32 chars (scrolls if wide)
      "state": "working",      // working | needs | idle | done | error
      "anim": "laptop"         // laptop | book | coffee | headphones | sleep | alert | done | error
    }
  ],
  "usage": {
    "s5h": { "pct": 45, "reset": 1763138100 }, // 5-hour session window
    "wk":  { "pct": 28, "reset": 1763590000 }  // weekly window
  }
}
```
* `sessions` is already ordered: cards that need the user first, then all others in a stable (first-seen) order; max 18 entries.
* `usage.s5h` / `usage.wk` may be missing or `null` → tile shows `—`.
* `reset` is unix epoch seconds; the device computes the countdown as `reset - now` and ticks locally.

### `ping` — keepalive, every 5 s
```json
{"t":"ping","now":1763130005}
```

## Device → host

### `hello` — on boot and as reply to `ping`
```json
{"t":"hello","fw":"0.1.0"}
```
On a `hello` the host immediately (re)sends the full `state`.

### `tap` — user tapped a session card (reserved for future use)
```json
{"t":"tap","id":"8c1f…"}
```

## Timeouts
If the device receives nothing for 15 s it shows the *Disconnected* screen and returns to the
main screen on the next `state`.
