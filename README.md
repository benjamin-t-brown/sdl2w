# SDL2W

SDL2W - SDL2 Wrapper

This is an opinionated C++23 library which wraps SDL2 functionality and includes the following features:

- Window and Renderer creation
  - GPU Mode (hardware-accelerated SDL renderer)
  - CPU Mode (software SDL renderer; same texture-based draw path)
- Asset Management
  - PNG images
  - WAV sound files
  - TTF fonts
  - Animation Definitions
  - Localization
  - WASM IndexDB Persistent Storage
- 2d Rendering
  - Sprites with Translate/Rotate/Scale
  - Alpha blending
  - Animations with Timing
  - Render to Texture
- Event management
  - Mouse events
  - Keyboard events
  - (coming soon) Joystick events
- Logging
  - Log Levels
  - Log with filename and line number

It also includes the following tools:
- Anims 
  - asset organizer for viewing pictures/animations
- Localization Parser
  - parse source code for LOC strings and generate localization file

The dependencies for this project are:

- [bmin](https://github.com/benjamin-t-brown/bmin) — fetched automatically on first build into `bmin/` at the repo root; headers and `libbmin.a` are copied into `src/deps/`
- SDL2
- SDL2_image
- SDL2_mixer
- SDL2_ttf
- SDL2_gfx

bmin is cloned to `bmin/` at the repo root and built as part of `make native` / `make wasm`. Build outputs are copied to `src/deps/` (`deps/bmin/*.h`, `deps/lib/libbmin.a`). To remove the clone and copied deps: `make clean-deps` from `src/`. To pin a version: `make native BMIN_REF=v0.1.0` (or a commit SHA).

# Build output

SDL2W provides a makefile that can build for the following platforms:

- GCC (c++23)
  - Windows x86_64
  - Mac x86_64
  - Linux x86_64
- WASM

To build GCC:

```
cd src
make clean
make native
```

To build for WASM

```
cd src
make clean
make wasm
```

The build command outputs a folder "sdl2w" in the repo which contains
```
include - include files for lib (to be ingested by your app)
lib - linkable .a file
```

# IDE setup (Cursor / VS Code)

For accurate go-to-definition, diagnostics, and refactoring, point your editor at a
[`compile_commands.json`](https://clang.llvm.org/docs/JSONCompilationDatabase.html)
at the repo root. That file is **gitignored** — generate it locally after dependencies
are in place.

sdl2w includes `<bmin/...>` headers from `src/deps/bmin/`, which only exist after bmin
has been cloned and copied there by the build. Run a native build first (or let the
script below fetch bmin for you), then generate compile commands.

From an **MSYS2 UCRT64** shell (same environment used for `make native`):

```bash
# 1. First-time setup: clone bmin, build libs, populate src/deps/
cd src
make native
cd ..

# 2. Generate compile_commands.json at the repo root
./compile-commands.sh
```

`compile-commands.sh` will also run `make -C src bmin` if needed, so step 1 is mainly
to verify the project builds before you open the IDE. If you only need IntelliSense and
have not built yet, `./compile-commands.sh` alone is enough to pull in bmin and write
the database.

Then open the **repository root** in Cursor or VS Code. Tools such as **clangd** and the
Microsoft C/C++ extension read `compile_commands.json` automatically.

Regenerate after changing Makefiles, include paths, or adding/removing source files:

```bash
./compile-commands.sh
```

The database covers `src/lib/*.cpp`, matching headers, and `example/main.cpp`. It uses
the same flags as the native `make` build (`-I. -Ideps` from `src/`).

On Windows, use the MSYS2 shell so paths and `g++` match the build:

```text
C:/progs/msys2/msys2_shell.cmd -defterm -here -no-start -ucrt64 -use-full-path
```

Optional: set `CXX` before running the script if `g++` is not on your PATH:

```bash
CXX=/ucrt64/bin/g++.exe ./compile-commands.sh
```

# Tools

To build tools

```
cd src
make tools
```

These tools will then be in:

```
src/build/tools
```

## Anims

Place the executable in same dir as your executable and it will load the same assets.  Use this to debug/edit sprites and animations.

## L10nScanner

Scans your code base for TRANSLATION macros and creates translation lines for them if they don't exist.  Preserves existing translations if they are there.

```
./L10nScanner.exe --input-dir <dir> --output-dir <dir2> en la fr
```

# Example

To build the example with GCC

```
cd example
make
```

To build the example in WASM for web, use nodejs in the web folder

```
cd web
npm i
npm run build
npm run dist
```

This starts an http server that points at the build.

<img width="1313" height="975" alt="image" src="https://github.com/user-attachments/assets/bdb04dfe-c99a-4efb-80c4-6556a611ac7d" />

# Linking SDL2W in your game

`sdl2w` builds a static library (`libsdl2w.a`) and headers.  
A matching `libbmin.a` and `include/bmin/` headers are produced from the same sdl2w build — **consumers should use that bundled bmin**, not a separate checkout, so versions stay in sync.

## Consumer workflow (recommended)

1. Clone/build sdl2w once (as a submodule, sibling directory, etc.):

   ```bash
   make -C path/to/sdl2w/src native
   ```

   This creates `path/to/sdl2w/sdl2w/` with libs and headers.

2. Copy artifacts into your game project:

   ```bash
   path/to/sdl2w/copy-sdl2w-artifacts.sh path/to/yourgame/lib/sdl2w
   ```

   Or copy manually from `sdl2w/sdl2w/` (see layout below).

3. Compile/link your game with **one** include dir and **one** lib dir:

   ```bash
   -Ipath/to/yourgame/lib/sdl2w
   -Lpath/to/yourgame/lib/sdl2w
   -lsdl2w -lbmin
   ```

   Use `#include <bmin/String.h>` and `#include "Window.h"` (or your include layout).

The `example/` project follows this pattern: `make` in `example/` builds sdl2w if needed, then copies into `example/lib/sdl2w/`.

If your game also uses bmin directly, use **only** the bmin headers and `libbmin.a` copied from sdl2w's build — do not link a second bmin.

## Artifact layout after copy

```
lib/sdl2w/
  libsdl2w.a
  libbmin.a
  Window.h, Draw.h, ...    # sdl2w headers
  bmin/
    String.h, Map.h, ...   # bmin headers (<bmin/...> includes)
```

## Required artifacts from SDL2W build (`sdl2w/sdl2w/`)

- `include/*.h`
- `include/bmin/*.h`
- `lib/libsdl2w.a`
- `lib/libbmin.a`

When consuming it, add the include path and library path, then link SDL2W + bmin + SDL2 dependencies in your final app link step.

## Native linking (g++)

Use:

- include path to SDL2W headers (for example `-I/path/to/sdl2w/include`)
- library path to SDL2W archive (for example `-L/path/to/sdl2w/lib`)
- `-lsdl2w`
- SDL libraries:
  - `-lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx`

Typical example:

```
g++ -std=c++23 -I/path/to/sdl2w/include main.cpp \
  -L/path/to/sdl2w/lib -lsdl2w -lbmin \
  -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx \
  -o game
```

## WebAssembly linking (em++)

`libsdl2w.a` itself does not need Emscripten `-s ...` settings during archive build.  
Set Emscripten options on your final game link command instead.

Typical wasm final link options include:

- `-L/path/to/sdl2w/lib -lsdl2w -lbmin`
- `-s USE_SDL=2`
- `-s USE_SDL_IMAGE=2`
- `-s USE_SDL_MIXER=2`
- `-s USE_SDL_TTF=2`
- `-s USE_SDL_GFX=2`
- any runtime/export/memory settings needed by your app (for example `-s EXPORTED_FUNCTIONS=...`, `-s EXPORTED_RUNTIME_METHODS=...`, `--preload-file ...`)

Example:

```
em++ -std=c++23 -Oz -I/path/to/sdl2w/include main.cpp \
  -L/path/to/sdl2w/lib -lsdl2w -lbmin \
  -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2 -s USE_SDL_GFX=2 \
  -s EXPORTED_FUNCTIONS='["_main"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall"]' \
  --preload-file assets \
  -o game.js
```
