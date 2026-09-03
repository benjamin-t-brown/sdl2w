#!/usr/bin/env bash
# Copy sdl2w + bundled bmin module artifacts into a consumer project directory.
#
# Prerequisite: build sdl2w first, e.g.  make -C src native
#
# Usage:
#   ./copy-sdl2w-artifacts.sh [DEST]
#
# Default DEST is ./lib/sdl2w relative to the current working directory.
# Creates:
#   DEST/libsdl2w_modules.a
#   DEST/libbmin_modules.a
#   DEST/modules/             (sdl2w + bmin .cppm and make helpers)
#
# Consumer Makefile: include DEST/modules/make/use.mk
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
SDL2W_DIST="${ROOT}/sdl2w"
DEST="${1:-lib/sdl2w}"

if [[ ! -f "${SDL2W_DIST}/lib/libsdl2w_modules.a" ]]; then
  echo "error: ${SDL2W_DIST}/lib/libsdl2w_modules.a not found." >&2
  echo "Run: make -C \"${ROOT}/src\" native" >&2
  exit 1
fi

if [[ ! -f "${SDL2W_DIST}/lib/libbmin_modules.a" ]]; then
  echo "error: ${SDL2W_DIST}/lib/libbmin_modules.a not found." >&2
  echo "Run: make -C \"${ROOT}/src\" native" >&2
  exit 1
fi

mkdir -p "${DEST}/modules"
cp -uv "${SDL2W_DIST}/lib/libsdl2w_modules.a" "${DEST}/"
cp -uv "${SDL2W_DIST}/lib/libbmin_modules.a" "${DEST}/"
cp -Ruv "${SDL2W_DIST}/modules/"* "${DEST}/modules/"

echo "Copied sdl2w + bmin module artifacts to ${DEST}"
echo "Include ${DEST}/modules/make/use.mk in your Makefile."
