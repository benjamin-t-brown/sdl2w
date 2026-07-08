#!/usr/bin/env bash
# Per-translation-unit compile timing for sdl2w (GCC/Clang -ftime-report).
#
# From Windows (MSYS2 UCRT64):
#   C:/progs/msys2/msys2_shell.cmd -defterm -here -no-start -ucrt64 -use-full-path -c "cd /c/progs/sdl2wbmininterop/sdl2w/src && make ftime-report"
# Or:
#   bash ftime-report.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

OUT_DIR="${ROOT}/build/ftime-report"
mkdir -p "$OUT_DIR"

CODE=(
  lib/Window.cpp
  lib/Draw.cpp
  lib/Logger.cpp
  lib/Store.cpp
  lib/AssetLoader.cpp
  lib/Events.cpp
  lib/Animation.cpp
  lib/L10n.cpp
  lib/Init.cpp
  lib/EmscriptenHelpers.cpp
)

CXX="${CXX:-g++}"
FTIME_FLAGS="${FTIME_FLAGS:--Wall -std=c++23 -O0 -I. -I${ROOT}/deps}"

SDL_CFLAGS=""
if command -v pkg-config >/dev/null 2>&1; then
  SDL_CFLAGS="$(pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer SDL2_gfx 2>/dev/null || true)"
fi

echo "=== sdl2w ftime-report ==="
echo "compiler: $($CXX --version | head -1)"
echo "flags:    $FTIME_FLAGS $SDL_CFLAGS -ftime-report"
echo "output:   $OUT_DIR"
echo

SUMMARY="$OUT_DIR/summary.txt"
: >"$SUMMARY"

for f in "${CODE[@]}"; do
  base="$(basename "$f" .cpp)"
  log="$OUT_DIR/${base}.log"
  echo "--- $f ---"
  # shellcheck disable=SC2086
  $CXX $FTIME_FLAGS $SDL_CFLAGS -ftime-report -c "$f" -o "$OUT_DIR/${base}.o" >"$log" 2>&1
  total="$(grep ' TOTAL' "$log" || true)"
  if [[ -z "$total" ]]; then
    echo "FAILED (see $log)" >&2
    tail -5 "$log" >&2
    exit 1
  fi
  wall="$(echo "$total" | awk '{print $3}')"
  printf "%8s  %s\n" "$wall" "$base" | tee -a "$SUMMARY"
  grep ' phase parsing' "$log" || true
  echo
done

echo "=== summary (slowest first, TOTAL wall seconds) ==="
sort -nr "$SUMMARY"
echo
echo "Full logs: $OUT_DIR/*.log"
