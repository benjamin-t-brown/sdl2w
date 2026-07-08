#!/usr/bin/env bash
# Copy sdl2w + bundled bmin build artifacts into a consumer project directory.
#
# Prerequisite: build sdl2w first, e.g.  make -C src native
#
# Usage:
#   ./copy-sdl2w-artifacts.sh [DEST]
#
# Default DEST is ./lib/sdl2w relative to the current working directory.
# Creates:
#   DEST/libsdl2w.a
#   DEST/libbmin.a
#   DEST/*.h
#   DEST/bmin/*.h
#
# Consumer compile flags (typical):
#   -I/path/to/DEST -L/path/to/DEST -lsdl2w -lbmin
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
SDL2W_DIST="${ROOT}/sdl2w"
DEST="${1:-lib/sdl2w}"

if [[ ! -f "${SDL2W_DIST}/lib/libsdl2w.a" ]]; then
  echo "error: ${SDL2W_DIST}/lib/libsdl2w.a not found." >&2
  echo "Run: make -C \"${ROOT}/src\" native" >&2
  exit 1
fi

if [[ ! -f "${SDL2W_DIST}/lib/libbmin.a" ]]; then
  echo "error: ${SDL2W_DIST}/lib/libbmin.a not found." >&2
  echo "Run: make -C \"${ROOT}/src\" native" >&2
  exit 1
fi

mkdir -p "${DEST}/bmin"
cp -uv "${SDL2W_DIST}/lib/libsdl2w.a" "${DEST}/"
cp -uv "${SDL2W_DIST}/lib/libbmin.a" "${DEST}/"
cp -uv "${SDL2W_DIST}/include/"*.h "${DEST}/"
cp -Ruv "${SDL2W_DIST}/include/bmin/"* "${DEST}/bmin/"

echo "Copied sdl2w + bmin artifacts to ${DEST}"
echo "Use: -I${DEST} -L${DEST} -lsdl2w -lbmin"
