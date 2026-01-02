# SDL2W

SDL2W - SDL2 Wrapper

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

Additionally there are some helper macros for includes in the toolchains for the following platforms:
- miyooa30
- miyoomini

# Tools

Anims

Place the executable in same dir as your executable and it will load the same assets.  Use this to debug/edit sprites and animations.

L10nScanner

Scans your code base for TRANSLATION macros and creates translation lines for them if they don't exist.  Preserves existing translations if they are there.

```
./L10nScanner.exe --input-dir <dir> --output-dir <dir2> en la fr
```
