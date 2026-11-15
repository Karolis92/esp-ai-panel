#!/usr/bin/env bash
# Build + flash the firmware. Frees the serial port first and puts things back afterwards:
#   - unloads the ai-panel launchd agent (KeepAlive would otherwise grab the port again), reloads it on exit
#   - stops a running `ai-panel demo`
# Usage: tools/flash.sh [extra idf.py args, e.g. monitor]   (prompts for the serial port)
#   PORT=/dev/cu.usbserial-XXX tools/flash.sh   # skip the port prompt
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LABEL="gg.ai-panel"
PLIST="$HOME/Library/LaunchAgents/$LABEL.plist"
DOMAIN="gui/$(id -u)"

# --- port ---------------------------------------------------------------
if [[ -z "${PORT:-}" ]]; then
  ports=()
  for p in /dev/cu.*; do
    case "$p" in
      *Bluetooth*|*debug-console*|'/dev/cu.*') ;;
      *) ports+=("$p") ;;
    esac
  done
  if (( ${#ports[@]} == 0 )); then
    echo "flash: no serial ports found. Is the board plugged in?" >&2
    exit 1
  fi
  if [[ ! -t 0 ]]; then
    echo "flash: not interactive; set PORT=... (found: ${ports[*]})" >&2
    exit 1
  fi
  echo "Select serial port:"
  PS3="Port number: "
  select PORT in "${ports[@]}"; do
    [[ -n "$PORT" ]] && break
    echo "Invalid choice."
  done
  [[ -n "${PORT:-}" ]] || exit 1   # Ctrl+D
fi
echo "flash: port $PORT"

# --- free the port ------------------------------------------------------
agent_was_loaded=0
if launchctl print "$DOMAIN/$LABEL" >/dev/null 2>&1; then
  echo "flash: stopping launchd agent $LABEL"
  launchctl bootout "$DOMAIN/$LABEL" || true
  agent_was_loaded=1
fi

restore() {
  if (( agent_was_loaded )) && [[ -f "$PLIST" ]]; then
    echo "flash: restarting launchd agent $LABEL"
    launchctl bootstrap "$DOMAIN" "$PLIST" || echo "flash: failed to restart $LABEL" >&2
  fi
}
trap restore EXIT

pkill -f "dist/cli.js demo" 2>/dev/null && echo "flash: stopped demo" || true

# Wait briefly for the port to be released.
for _ in {1..20}; do
  lsof "$PORT" >/dev/null 2>&1 || break
  sleep 0.25
done
if lsof "$PORT" >/dev/null 2>&1; then
  echo "flash: $PORT is still in use by:" >&2
  lsof "$PORT" >&2
  exit 1
fi

# --- flash --------------------------------------------------------------
if [[ -z "${IDF_PATH:-}" ]]; then
  # shellcheck disable=SC1091
  . "${IDF_EXPORT:-$HOME/esp/esp-idf/export.sh}" >/dev/null
fi
cd "$ROOT/firmware"
idf.py -p "$PORT" flash "$@"
