# Including SDL2W in your game

This guide explains how to add [sdl2w](README.md) to a C++23 game project.

sdl2w is a **static library**. Each build also produces a matching **bmin** static library and headers. Your game should copy and use **that bundled bmin** from the sdl2w build so versions stay in sync.

## Prerequisites

- C++23 compiler (`g++` or `em++` for WebAssembly)
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

# 2. Copy libs + headers into your game tree
path/to/sdl2w/copy-sdl2w-artifacts.sh path/to/yourgame/lib/sdl2w

# 3. Compile and link your game (see below)
```

---

## Step 1 — Build sdl2w

Clone sdl2w into your workspace (submodule, sibling folder, etc.), then build:

```bash
make -C path/to/sdl2w/src native
```

For WebAssembly:

```bash
make -C path/to/sdl2w/src wasm
```

This writes a consumer bundle to:

```
path/to/sdl2w/sdl2w/
  lib/
    libsdl2w.a
    libbmin.a
  include/
    Window.h, Draw.h, Store.h, ...
    bmin/
      String.h, Map.h, DynArray.h, ...
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
mkdir -p "$DEST/bmin"

cp path/to/sdl2w/sdl2w/lib/libsdl2w.a "$DEST/"
cp path/to/sdl2w/sdl2w/lib/libbmin.a   "$DEST/"
cp path/to/sdl2w/sdl2w/include/*.h     "$DEST/"
cp -R path/to/sdl2w/sdl2w/include/bmin/* "$DEST/bmin/"
```

### Layout after copy

```
yourgame/lib/sdl2w/
  libsdl2w.a
  libbmin.a
  Window.h
  Draw.h
  Store.h
  ...
  bmin/
    String.h
    Map.h
    UniquePtr.h
    ...
```

Re-run the copy step whenever you update or rebuild sdl2w.

---

## Step 3 — Compiler and linker flags

Use **one include directory** and **one library directory**:

| Flag | Value |
|---|---|
| Include | `-Ipath/to/yourgame/lib/sdl2w` |
| Library path | `-Lpath/to/yourgame/lib/sdl2w` |
| Libraries | `-lsdl2w -lbmin` |
| Language | `-std=c++23` |

Link **one** `libbmin.a`. Do not also link a bmin built elsewhere.

### Native example (g++)

```bash
g++ -std=c++23 -Wall \
  -Ipath/to/yourgame/lib/sdl2w \
  main.cpp \
  -Lpath/to/yourgame/lib/sdl2w -lsdl2w -lbmin \
  -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx \
  -o yourgame
```

On macOS you may need extra `-L` paths for Homebrew SDL libraries.

### WebAssembly example (em++)

Set Emscripten SDL port flags on the **final** link of your game (not when building the sdl2w archive):

```bash
em++ -std=c++23 -Oz \
  -Ipath/to/yourgame/lib/sdl2w \
  main.cpp \
  -Lpath/to/yourgame/lib/sdl2w -lsdl2w -lbmin \
  -s USE_SDL=2 \
  -s USE_SDL_IMAGE=2 \
  -s USE_SDL_MIXER=2 \
  -s USE_SDL_TTF=2 \
  -s USE_SDL_GFX=2 \
  --preload-file assets \
  -o yourgame.js
```

Add any extra `-s EXPORTED_FUNCTIONS=...`, `-s EXPORTED_RUNTIME_METHODS=...`, or memory settings your app needs.

---

## Includes in source code

With `-Ipath/to/yourgame/lib/sdl2w`:

```cpp
#include <bmin/String.h>

#include "Window.h"
#include "Draw.h"
#include "Store.h"
#include "AssetLoader.h"
```

- sdl2w headers: `#include "Window.h"` (or your preferred layout)
- bmin headers: `#include <bmin/String.h>` (note the `bmin/` prefix)

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

---

## Using bmin in your game

If your game uses bmin types directly (`bmin::String`, `bmin::Map`, etc.):

1. Include headers from the **copied** tree: `#include <bmin/String.h>`
2. Link the **copied** `libbmin.a` from sdl2w's build
3. Do **not** add a second bmin include path or link another `libbmin.a`

That keeps `bmin::String` and other types identical across your code and sdl2w.

---

## Makefile integration

The `example/` project shows a common pattern:

1. Build sdl2w when needed: `make -C path/to/sdl2w/src native`
2. Copy artifacts into `lib/sdl2w/`
3. Compile with `-Ilib/sdl2w`
4. Link with `-Llib/sdl2w -lsdl2w -lbmin`

Minimal Makefile variables:

```makefile
SDL2W_DIR = path/to/sdl2w
SDL2W_LIB = lib/sdl2w/libsdl2w.a

INCLUDES += -Ilib/sdl2w
LIBS     += -Llib/sdl2w -lsdl2w -lbmin

$(SDL2W_LIB):
	$(MAKE) -C $(SDL2W_DIR)/src native
	$(SDL2W_DIR)/copy-sdl2w-artifacts.sh lib/sdl2w
```

See [example/Makefile](example/Makefile) for a full native and wasm setup.

---

## Updating sdl2w

When you pull a new sdl2w revision:

```bash
make -C path/to/sdl2w/src clean
make -C path/to/sdl2w/src native
path/to/sdl2w/copy-sdl2w-artifacts.sh path/to/yourgame/lib/sdl2w
```

Rebuild your game. Copy both `libsdl2w.a` and `libbmin.a` together — they are built as a pair.

---

## Common mistakes

| Mistake | Problem |
|---|---|
| Linking two different `libbmin.a` files | Duplicate symbol errors at link time |
| Mixing bmin headers from another install | Subtle type/ABI mismatches |
| Using `<String.h>` instead of `<bmin/String.h>` | Wrong or missing headers |
| Forgetting `-lbmin` | Unresolved symbols from sdl2w |
| Pointing includes at `sdl2w/src/deps` | Internal build path; use the copied `lib/sdl2w` bundle instead |

---

## Reference: what sdl2w ships

After `make -C src native`, the `sdl2w/sdl2w/` directory contains everything a game needs:

| Path | Purpose |
|---|---|
| `lib/libsdl2w.a` | SDL2W static library |
| `lib/libbmin.a` | bmin static library (matched to this sdl2w build) |
| `include/*.h` | SDL2W public headers |
| `include/bmin/*.h` | bmin public headers |

Copy those into your project, add the flags above, and link SDL2.
