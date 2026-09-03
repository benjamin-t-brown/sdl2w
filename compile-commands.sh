#!/bin/bash
# Generate compile_commands.json for sdl2w C++ modules (clangd).
#
# Prefers clang++ (set CLANGXX to override). clangd builds its own BMIs when
# started with --experimental-modules-support; this DB must list every .cppm
# and every TU that imports them.

set -euo pipefail

cd "$(dirname "$0")"
ROOT="$(pwd)"

echo "Ensuring bmin modules are available..."
make -C src bmin TARGET=native >/dev/null

if [[ -z "${CLANGXX:-}" ]]; then
	if command -v clang++ >/dev/null 2>&1; then
		CLANGXX="$(command -v clang++)"
	elif [[ -x /c/progs/msys2/ucrt64/bin/clang++.exe ]]; then
		CLANGXX=/c/progs/msys2/ucrt64/bin/clang++.exe
	elif [[ -x /ucrt64/bin/clang++.exe ]]; then
		CLANGXX=/ucrt64/bin/clang++.exe
	elif [[ -x "/c/Program Files/LLVM/bin/clang++.exe" ]]; then
		CLANGXX="/c/Program Files/LLVM/bin/clang++.exe"
	else
		CLANGXX=clang++
		echo "warning: clang++ not found; using 'clang++' as the driver name" >&2
	fi
fi

if [[ -z "${PYTHON:-}" ]]; then
	if command -v python >/dev/null 2>&1; then
		PYTHON=python
	elif command -v python3 >/dev/null 2>&1; then
		PYTHON=python3
	else
		echo "python not found on PATH" >&2
		exit 1
	fi
fi

COMPILER="$CLANGXX"
if command -v cygpath >/dev/null 2>&1; then
	COMPILER="$(cygpath -m "$CLANGXX")"
	ROOT="$(cygpath -m "$ROOT")"
else
	COMPILER="${COMPILER//\\//}"
fi

"$PYTHON" - "$ROOT" "$COMPILER" <<'PY'
import json
import sys
from pathlib import Path

root = Path(sys.argv[1]).resolve()
compiler = sys.argv[2].replace("\\", "/")
mod_dir = root / "src" / "modules"
bmin_mod = root / "src" / "bmin" / "modules"
example_dir = root / "example"
tools_dir = root / "src" / "tools"

# clangd --experimental-modules-support scans the CDB for every interface
# that anything imports. Missing bmin .cppm entries make sdl2w.* BMIs fail,
# which then shows as "Module 'sdl2w.assets' not found" on every consumer.
SDL2W_IFACES = [
    "sdl2w.defines.cppm",
    "sdl2w.logger.cppm",
    "sdl2w.types.cppm",
    "sdl2w.events.cppm",
    "sdl2w.animation.cppm",
    "sdl2w.store.cppm",
    "sdl2w.draw.cppm",
    "sdl2w.assets.cppm",
    "sdl2w.l10n.cppm",
    "sdl2w.emscripten.cppm",
    "sdl2w.window.cppm",
    "sdl2w.init.cppm",
    "sdl2w.cppm",
]

BMIN_IFACES = [
    "bmin.types.cppm",
    "bmin.detail.cppm",
    "bmin.dynarray.cppm",
    "bmin.unique_ptr.cppm",
    "bmin.string.cppm",
    "bmin.list.cppm",
    "bmin.queue.cppm",
    "bmin.hash.cppm",
    "bmin.map.cppm",
    "bmin.stringstream.cppm",
    "bmin.string_interop.cppm",
    "bmin.containers.cppm",
]

BMIN_IMPLS = [
    "bmin.detail.cpp",
    "bmin.string.cpp",
    "bmin.stringstream.cpp",
    "bmin.string_interop.cpp",
]

# Clang/clangd C++20 modules — do not pass GCC's -fmodules-ts.
COMMON = [
    "-Wall",
    "-std=c++23",
    f"-I{mod_dir.as_posix()}",
]
if bmin_mod.is_dir():
    COMMON.append(f"-I{bmin_mod.as_posix()}")


def entry(directory: Path, source: Path) -> dict:
    args = [compiler, *COMMON]
    if source.suffix == ".cppm":
        args.extend(["-x", "c++-module"])
    args.extend(["-c", source.as_posix(), "-o", (directory / (source.stem + ".o")).as_posix()])
    return {
        "directory": directory.as_posix(),
        "arguments": args,
        "file": source.as_posix(),
    }


db = []

if bmin_mod.is_dir():
    for name in BMIN_IFACES:
        src = bmin_mod / name
        if src.is_file():
            db.append(entry(bmin_mod, src))
    for name in BMIN_IMPLS:
        src = bmin_mod / name
        if src.is_file():
            db.append(entry(bmin_mod, src))

for name in SDL2W_IFACES:
    src = mod_dir / name
    if src.is_file():
        db.append(entry(mod_dir, src))

smoke = mod_dir / "smoke.cpp"
if smoke.is_file():
    db.append(entry(mod_dir, smoke))

main = example_dir / "main.cpp"
if main.is_file():
    db.append(entry(example_dir, main))

anims = tools_dir / "Anims.cpp"
if anims.is_file():
    db.append(entry(tools_dir, anims))

scanner = tools_dir / "L10nScanner.cpp"
if scanner.is_file():
    db.append(entry(tools_dir, scanner))

if not db:
    raise SystemExit(
        "No compile commands captured. Run 'make -C src native' once, then retry."
    )

out = root / "compile_commands.json"
out.write_text(json.dumps(db, indent=1) + "\n", encoding="utf-8")
print(f"Wrote {out} ({len(db)} entries)")
print(f"Compiler driver: {compiler}")
print("Restart clangd after running this (Command Palette: clangd: Restart language server).")
PY
