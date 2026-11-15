#!/bin/sh
# Render the firmware UI on the host (no board needed) and write PNG screenshots to out/.
#   tools/ui-preview/build.sh          → out/0_waiting.png, 1_page1.png, 2_page2.png, 3_empty.png
# First run clones LVGL (same version as firmware/main/idf_component.yml) and builds it (~1 min).
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
FW="$HERE/../../firmware/main"
LVGL_VER="${LVGL_VER:-v9.3.0}"
cd "$HERE"
mkdir -p .build out

if [ ! -d .lvgl ]; then
  git clone -q --depth 1 --branch "$LVGL_VER" https://github.com/lvgl/lvgl.git .lvgl
fi
if [ ! -f .build/liblvgl.a ]; then
  echo "building LVGL $LVGL_VER..."
  mkdir -p .build/obj
  i=0
  find .lvgl/src -name '*.c' | while read -r s; do
    i=$((i + 1))
    cc -c -O1 -w -DLV_CONF_INCLUDE_SIMPLE -I"$HERE" -I.lvgl "$s" -o ".build/obj/$i.o"
  done
  ar rcs .build/liblvgl.a .build/obj/*.o
fi

cc -O1 -Wall -Wextra -Wno-unused-parameter -DLV_CONF_INCLUDE_SIMPLE \
  -I"$HERE" -I"$HERE/stubs" -I.lvgl -I"$FW" -I"$FW/board" -I"$FW/link" -I"$FW/ui" \
  harness.c "$FW/link/model.c" "$FW"/ui/*.c "$FW"/ui/fonts/*.c .build/liblvgl.a -o .build/preview

cd out && ../.build/preview | grep -v "sanity checks" || true
for b in *.bmp; do
  if command -v sips >/dev/null; then sips -s format png "$b" --out "${b%.bmp}.png" >/dev/null && rm "$b"; fi
done
ls "$HERE/out"
