# Including SDL2W in your game

This guide explains how to add [sdl2w](README.md) to a C++23 game project.

sdl2w is a **C++ modules** static library (`import sdl2w`). Each build also
produces a matching **bmin** modules library. Your game should copy and use
**that bundled bmin** from the sdl2w build so versions stay in sync.

There is no header / `#include "Window.h"` API.

Wasm uses the same modules with em++ (`make TARGET=wasm`). `use.mk` finds
emsdk at `EMSDK` or a sibling `../emsdk` (for this repo: `progs/emsdk`).

## Prerequisites

- GCC with C++23 modules (`g++` + `-fmodules-ts`) for native
- emsdk / em++ for wasm (`make TARGET=wasm` or `make -C example js`)
- SDL2 development libraries:
  - SDL2
  - SDL2_image
  - SDL2_mixer
  - SDL2_ttf
  - SDL2_gfx

You do **not** need a separate bmin checkout in your game if you follow this guide.

---

## Quick start

```bash
# 1. Build sdl2w (from your sdl2w clone/submodule)
make -C path/to/sdl2w/src native

# 2. Copy libs + module sources into your game tree
path/to/sdl2w/copy-sdl2w-artifacts.sh path/to/yourgame/lib/sdl2w

# 3. Include use.mk and compile (see below)
```

---

## Step 1 — Build sdl2w

Clone sdl2w into your workspace (submodule, sibling folder, etc.), then build:

```bash
make -C path/to/sdl2w/src native
```

This writes a consumer bundle to:

```
path/to/sdl2w/sdl2w/
  lib/
    libsdl2w_modules.a
    libbmin_modules.a
  modules/
    sdl2w.cppm, macros.h, ...
    make/use.mk
    bmin/
```

The first build also clones and builds bmin inside the sdl2w repo. You do not manage that separately.

---

## Step 2 — Copy artifacts into your game

### Option A: copy script (recommended)

From your game project (or any directory):

```bash
path/to/sdl2w/copy-sdl2w-artifacts.sh path/to/yourgame/lib/sdl2w
```

Default destination (if you omit the argument) is `./lib/sdl2w` relative to the current working directory.

### Option B: manual copy

```bash
DEST=path/to/yourgame/lib/sdl2w
mkdir -p "$DEST/modules"

cp path/to/sdl2w/sdl2w/lib/libsdl2w_modules.a "$DEST/"
cp path/to/sdl2w/sdl2w/lib/libbmin_modules.a  "$DEST/"
cp -R path/to/sdl2w/sdl2w/modules/*           "$DEST/modules/"
```

### Layout after copy

```
yourgame/lib/sdl2w/
  libsdl2w_modules.a
  libbmin_modules.a
  modules/
    sdl2w.cppm
    macros.h
    make/use.mk
    bmin/
```

Re-run the copy step whenever you update or rebuild sdl2w.

---

## Step 3 — Makefile

`use.mk` rebuilds BMIs with **your** compiler and sets flags:

```makefile
include path/to/yourgame/lib/sdl2w/modules/make/use.mk

main.o: main.cpp sdl2w-bmi
	$(CXX) $(SDL2W_CXXFLAGS) -c main.cpp -o $@

yourgame: main.o
	$(CXX) $(SDL2W_CXXFLAGS) -o $@ main.o $(SDL2W_LDLIBS)
```

`SDL2W_CXXFLAGS` includes `-fmodules-ts` and the module `-I` paths.
`SDL2W_LDLIBS` is `-lsdl2w_modules -lbmin_modules` plus SDL2.

Link **one** `libbmin_modules.a`. Do not also link a bmin built elsewhere.

The `example/` project follows this pattern.

---

## Imports in source code

```cpp
import sdl2w;
#include "macros.h"   // TRANSLATE — macros cannot be exported

sdl2w::log(sdl2w::INFO) << "ok" << sdl2w::endl;
sdl2w::logAt(sdl2w::ERROR) << "failed";
sdl2w::fail("cannot open font");
draw.drawText(TRANSLATE("Welcome"), params);
```

Import `bmin.string_interop` separately for extra `std::string_view` helpers.

Most sdl2w APIs take `std::string_view`, so string literals work without extra conversion:

```cpp
window.playSound("click");
draw.drawText("Hello", params);
```

When a function returns `bmin::String`, pass it to sdl2w APIs via `.sliceView()` or `.cStr()`:

```cpp
bmin::String contents = sdl2w::loadFileAsString("save.txt");
draw.drawText(contents.sliceView(), params);
```

Do not `#include` bmin headers in the same program.

---

## Using bmin in your game

If your game uses bmin types directly (`bmin::String`, `bmin::Map`, etc.):

1. `import bmin.containers` (or the bundled modules under `modules/bmin/`)
2. Link the **copied** `libbmin_modules.a` from sdl2w's build
3. Do **not** add a second bmin include path or link another bmin library

That keeps `bmin::String` and other types identical across your code and sdl2w.

---

## Updating sdl2w

When you pull a new sdl2w revision:

```bash
make -C path/to/sdl2w/src clean
make -C path/to/sdl2w/src native
path/to/sdl2w/copy-sdl2w-artifacts.sh path/to/yourgame/lib/sdl2w
```

Rebuild your game. Copy both `libsdl2w_modules.a` and `libbmin_modules.a` together — they are built as a pair.

---

## Common mistakes

| Mistake | Problem |
|---|---|
| Linking two different `libbmin_modules.a` files | Duplicate symbol errors at link time |
| Mixing `#include` of bmin headers with `import` of bmin modules | Parallel APIs; types are not the same |
| Forgetting `#include "macros.h"` | `TRANSLATE` is a macro (logging is `sdl2w::log` / `fail`) |
| Forgetting `-lbmin_modules` | Unresolved symbols from sdl2w |
| Pointing includes at `sdl2w/src/bmin` headers | There is no header API; use `modules/make/use.mk` |

---

## Reference: what sdl2w ships

After `make -C src native`, the `sdl2w/sdl2w/` directory contains everything a game needs:

| Path | Purpose |
|---|---|
| `lib/libsdl2w_modules.a` | sdl2w module object code |
| `lib/libbmin_modules.a` | bmin module object code (matched to this sdl2w build) |
| `modules/*.cppm` | sdl2w named modules |
| `modules/macros.h` | `TRANSLATE` (and optional `LOG` wrappers) |
| `modules/bmin/` | bmin `.cppm` sources + make helpers |
| `modules/make/use.mk` | consumer Make helper |

See [src/modules/MODULES.md](src/modules/MODULES.md) and [example/](example/).
