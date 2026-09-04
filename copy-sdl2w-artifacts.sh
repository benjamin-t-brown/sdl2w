#!/usr/bin/env bash
# Copy both SDL2W APIs and their matching bmin artifacts into a consumer project.
#
# Prerequisite: build sdl2w first, e.g.  make -C src native
#
# Usage:
#   ./copy-sdl2w-artifacts.sh [DEST]
#
# Default DEST is ./lib/sdl2w relative to the current working directory.
# Creates:
#   DEST/lib/libsdl2w.a
#   DEST/lib/libbmin.a
#   DEST/lib/libsdl2w_modules.a
#   DEST/lib/libbmin_modules.a
#   DEST/include/             (classic SDL2W + bmin headers)
#   DEST/modules/             (sdl2w + bmin .cppm and make helpers)
#
# Consumer Makefile: include DEST/modules/make/use.mk
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
SDL2W_DIST="${ROOT}/sdl2w"
DEST="${1:-lib/sdl2w}"

for artifact in libsdl2w.a libbmin.a libsdl2w_modules.a libbmin_modules.a; do
  if [[ ! -f "${SDL2W_DIST}/lib/${artifact}" ]]; then
    echo "error: ${SDL2W_DIST}/lib/${artifact} not found." >&2
    echo "Run: make -C \"${ROOT}/src\" native" >&2
    exit 1
  fi
done

mkdir -p "${DEST}/lib" "${DEST}/include" "${DEST}/modules"
cp -v "${SDL2W_DIST}/lib/"*.a "${DEST}/lib/"
cp -Rv "${SDL2W_DIST}/include/"* "${DEST}/include/"
cp -Rv "${SDL2W_DIST}/modules/"* "${DEST}/modules/"

echo "Copied classic and module SDL2W + bmin artifacts to ${DEST}"
