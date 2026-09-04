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

- [bmin](https://github.com/benjamin-t-brown/bmin) — fetched automatically on first build into `bmin/` at the repo root (`BMIN_REF` defaults to `experiment/cpp-modules`). Its matching header and module artifacts are copied to `src/bmin/`.
- SDL2
- SDL2_image
- SDL2_mixer
- SDL2_ttf
- SDL2_gfx

bmin is cloned to `bmin/` at the repo root and built as part of `make native`.
SDL2W and bmin both ship a classic header library and a named-module library.
To remove the clone and copied dependencies, run `make clean-deps` from `src/`.

# Build output

SDL2W is dual-ship on native platforms:

- classic headers with `libsdl2w.a` + `libbmin.a`
- named modules with `libsdl2w_modules.a` + `libbmin_modules.a`

The native module toolchain currently targets GCC 15. Supported platforms:

- GCC (c++23 + `-fmodules-ts`) — native
  - Windows x86_64
  - Mac x86_64
  - Linux x86_64
- em++ (Emscripten / Clang C++20 modules) — wasm

To build native:

```
cd src
make clean
make native
```

To build wasm archives (requires [emsdk](https://emscripten.org/docs/getting_started/downloads.html); this repo looks for a sibling `../emsdk`, e.g. `progs/emsdk`):

```
cd src
make wasm
```

Override the SDK path with `EMSDK=/path/to/emsdk`.

The build command outputs a folder `sdl2w` in the repo which contains
```
include - classic SDL2W and bmin headers
lib     - both SDL2W archives and both matching bmin archives
modules - .cppm interfaces, .cpp implementation units, macros, and make helpers
```

# IDE setup (Cursor / VS Code)

For accurate go-to-definition, diagnostics, and refactoring, point your editor at a
[`compile_commands.json`](https://clang.llvm.org/docs/JSONCompilationDatabase.html)
at the repo root. That file is **gitignored** — generate it locally after dependencies
are in place.

sdl2w modules live under `src/modules/`; the parallel classic sources live under
`src/lib/`. bmin artifacts exist after bmin has been built and copied to
`src/bmin/`. Run a native build
first (or let the script below fetch bmin for you), then generate compile commands.

From an **MSYS2 UCRT64** shell (same environment used for `make native`):

```bash
# 1. First-time setup: clone bmin, build libs, populate src/bmin/modules/
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

Regenerate after changing Makefiles or adding/removing source files:

```bash
./compile-commands.sh
```

The module database covers interfaces, module implementation units, direct-import
probes, bundled bmin modules, and the module form of `example/main.cpp`. clangd needs `--experimental-modules-support`
(already in `.vscode/settings.json`). Restart clangd after regenerating.

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

Wasm for the example (emsdk must be installed; defaults to `../../emsdk` from `example/`):

```
cd example
make js
```

That writes `SDL2W_EXAMPLE.js`, `.wasm`, and `.data` to `web/`. From `web/`, `npm run build` still packages those into `dist/`.

<img width="1313" height="975" alt="image" src="https://github.com/user-attachments/assets/bdb04dfe-c99a-4efb-80c4-6556a611ac7d" />

# Linking SDL2W in your game

`sdl2w` builds both APIs. Module consumers use `libsdl2w_modules.a` and the
matching `libbmin_modules.a`; header consumers use `libsdl2w.a` and the matching
`libbmin.a`. Use the bundled bmin in either mode so versions stay in sync.

```cpp
import sdl2w;
#include "macros.h"   // TRANSLATE; logging is sdl2w::log / logAt / fail
```

See `src/modules/MODULES.md` and `INCLUDE.md`.

## Consumer workflow (recommended)

1. Clone/build sdl2w once (as a submodule, sibling directory, etc.):

   ```bash
   make -C path/to/sdl2w/src native
   ```

   This creates `path/to/sdl2w/sdl2w/` with both library forms and their
   public sources.

2. Copy artifacts into your game project:

   ```bash
   path/to/sdl2w/copy-sdl2w-artifacts.sh path/to/yourgame/lib/sdl2w
   ```

3. Include `use.mk` in your Makefile:

   ```makefile
   include path/to/yourgame/lib/sdl2w/modules/make/use.mk
   main.o: main.cpp sdl2w-bmi
   	$(CXX) $(SDL2W_CXXFLAGS) -c main.cpp -o $@
   ```

The `example/` project builds this module form and the classic header form from
the same source file.

If your game also uses bmin directly, use the bmin artifacts copied from the
same SDL2W build—modules with modules, or headers with headers.

## Artifact layout after copy

```
lib/sdl2w/
  lib/
    libsdl2w.a
    libbmin.a
    libsdl2w_modules.a
    libbmin_modules.a
  include/
    Window.h, Draw.h, ...
    bmin/
  modules/
    sdl2w.cppm, sdl2w.*.cpp, macros.h, ...
    make/use.mk
    bmin/
```
