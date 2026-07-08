#!/bin/bash
set -euo pipefail

cd "$(dirname "$0")"

SRC_DIR="$(cd src && pwd)"

if [[ -z "${CXX:-}" ]]; then
	if [[ -x /ucrt64/bin/g++.exe ]]; then
		CXX=/ucrt64/bin/g++.exe
	else
		CXX="$(command -v g++)"
	fi
fi

COMPILER="$CXX"
if command -v cygpath >/dev/null 2>&1; then
	COMPILER="$(cygpath -m "$CXX")"
else
	COMPILER="${COMPILER//\\//}"
fi

echo "Ensuring bmin is available..."
make -C src bmin TARGET=native >/dev/null

TMP="$(mktemp)"
trap 'rm -f "$TMP"' EXIT

make -C src -n -B all TARGET=native 2>/dev/null |
	grep -E ' -Ideps( |$).* -c .+ -o ' >"$TMP" || true

python - "$SRC_DIR" "$COMPILER" "$TMP" <<'PY'
import json
import sys
from pathlib import Path

src_dir, compiler, lines_path = sys.argv[1:4]
src_root = Path("src")
db = []

for line in Path(lines_path).read_text(encoding="utf-8").splitlines():
    parts = line.split()
    if "-c" not in parts:
        continue
    c_idx = parts.index("-c")
    source = parts[c_idx + 1].replace("\\", "/")
    args = list(parts)
    if args[0].replace("\\", "/").endswith(("g++", "g++.exe", "c++", "clang++")):
        args[0] = compiler
    else:
        args.insert(0, compiler)
    db.append({
        "directory": src_dir.replace("\\", "/"),
        "arguments": args,
        "file": source,
    })


def header_entry(template, header_rel):
    args = list(template["arguments"])
    header_rel = str(header_rel).replace("\\", "/")
    for i, arg in enumerate(args):
        if arg == "-c" and i + 1 < len(args):
            args[i + 1] = header_rel
            break
    if "-x" not in args:
        c_idx = args.index("-c")
        args.insert(c_idx, "c++-header")
        args.insert(c_idx, "-x")
    return {
        "directory": template["directory"],
        "arguments": args,
        "file": header_rel,
    }


extra = []
known = {e["file"].replace("\\", "/") for e in db}
for entry in db:
    cpp = Path(entry["file"])
    if cpp.suffix != ".cpp":
        continue
    header = cpp.with_suffix(".h")
    if not (src_root / header).exists():
        continue
    rel = str(header).replace("\\", "/")
    if rel in known:
        continue
    extra.append(header_entry(entry, header))
    known.add(rel)

template = db[0] if db else None
if template:
    for header in sorted((src_root / "lib").glob("*.h")):
        rel = str(header.relative_to(src_root)).replace("\\", "/")
        if rel in known:
            continue
        extra.append(header_entry(template, rel))
        known.add(rel)

db.extend(extra)

example_cpp = Path("example/main.cpp")
if example_cpp.exists() and template:
    example_dir = str(example_cpp.parent.resolve()).replace("\\", "/")
    args = [
        compiler,
        "-Wall",
        "-std=c++23",
        "-O0",
        "-g",
        "-I.",
        "-I../src/deps",
        "-I../src/lib",
    ] + ["-c", "main.cpp"]
    db.append({
        "directory": example_dir,
        "arguments": args,
        "file": str(example_cpp.resolve()).replace("\\", "/"),
    })

if not db:
    raise SystemExit(
        "No compile commands captured. Run 'make -C src native' once, then retry."
    )

Path("compile_commands.json").write_text(
    json.dumps(db, indent=1) + "\n",
    encoding="utf-8",
)
PY

echo "Wrote compile_commands.json ($(python -c "import json; print(len(json.load(open('compile_commands.json'))))") entries)"
