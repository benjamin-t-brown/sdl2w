# SDL2W

SDL2W - SDL2 Wrapper

<img width="1313" height="975" alt="image" src="https://github.com/user-attachments/assets/bdb04dfe-c99a-4efb-80c4-6556a611ac7d" />


This is an opinionated C++17 library which wraps SDL2 functionality and includes the following features:

- Window and Renderer creation
  - GPU Mode
  - CPU Mode
- Asset Management
  - PNG images
  - WAV sound files
  - TTF fonts
  - Animation Definitions
  - Localization
  - WASM IndexDB Persistent Storage
- 2d Rendering
  - Sprites with Translate/Rotate/Scale
  - Cached flipped sprites
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

The only dependencies for this project are SDL2 libs.

SDL2
SDL2_image
SDL2_mixer
SDL2_ttf
SDL2_gfx

# Build output

SDL2W provides a makefile that can build for the following platforms:

- GCC (c++17)
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

# Linking SDL2W in your game

`sdl2w` builds a static library (`libsdl2w.a`) and headers.  
When consuming it, add the include path and library path, then link SDL2W + SDL2 dependencies in your final app link step.

## Required artifacts from SDL2W

- `sdl2w/include/*.h`
- `sdl2w/lib/libsdl2w.a`

## Native linking (g++)

Use:

- include path to SDL2W headers (for example `-I/path/to/sdl2w/include`)
- library path to SDL2W archive (for example `-L/path/to/sdl2w/lib`)
- `-lsdl2w`
- SDL libraries:
  - `-lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx`

Typical example:

```
g++ -std=c++17 -I/path/to/sdl2w/include main.cpp \
  -L/path/to/sdl2w/lib -lsdl2w \
  -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx \
  -o game
```

## WebAssembly linking (em++)

`libsdl2w.a` itself does not need Emscripten `-s ...` settings during archive build.  
Set Emscripten options on your final game link command instead.

Typical wasm final link options include:

- `-L/path/to/sdl2w/lib -lsdl2w`
- `-s USE_SDL=2`
- `-s USE_SDL_IMAGE=2`
- `-s USE_SDL_MIXER=2`
- `-s USE_SDL_TTF=2`
- `-s USE_SDL_GFX=2`
- any runtime/export/memory settings needed by your app (for example `-s EXPORTED_FUNCTIONS=...`, `-s EXPORTED_RUNTIME_METHODS=...`, `--preload-file ...`)

Example:

```
em++ -std=c++17 -Oz -I/path/to/sdl2w/include main.cpp \
  -L/path/to/sdl2w/lib -lsdl2w \
  -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2 -s USE_SDL_GFX=2 \
  -s EXPORTED_FUNCTIONS='["_main"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall"]' \
  --preload-file assets \
  -o game.js
```
