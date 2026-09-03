# Using sdl2w C++ modules

sdl2w is modules-only. Native consumers `import sdl2w` and link
`libsdl2w_modules.a` + `libbmin_modules.a`. There is no header / `libsdl2w.a`
path.

Wasm consumers use the same `.cppm` sources with em++ (`make TARGET=wasm`).
Clang BMIs (`pcm.cache`) are not interchangeable with GCC `gcm.cache`.
`use.mk` rebuilds PCMs and links the resulting `.o` files.

Do not `#include` bmin headers in the same program as `import` of bmin modules
(they are parallel APIs, not the same types).

## Layout (after `make native`)

```
sdl2w/lib/libsdl2w_modules.a
sdl2w/lib/libbmin_modules.a
sdl2w/modules/*.cppm            # one file per named module (interface + bodies)
sdl2w/modules/macros.h          # TRANSLATE (and optional LOG wrappers)
sdl2w/modules/bmin/             # bmin module sources + make helpers
sdl2w/modules/make/use.mk
sdl2w/modules/make/build-bmi.mk
```

## Consumer Makefile

```makefile
include path/to/sdl2w/modules/make/use.mk

main.o: main.cpp sdl2w-bmi
	$(CXX) $(SDL2W_CXXFLAGS) -c main.cpp -o $@

app: main.o
	$(CXX) $(SDL2W_CXXFLAGS) -o $@ main.o $(SDL2W_LDLIBS)

# Wasm (em++). Requires emsdk; set EMSDK if it is not ../emsdk from the repo.
js:
	$(MAKE) TARGET=wasm app.js
```

```cpp
import sdl2w;
#include "macros.h"   // TRANSLATE (logging is exported: log / logAt / fail / endl)

int main() {
  sdl2w::Window::init();
  sdl2w::Store store;
  sdl2w::Window window(store, {.title = "hi", .w = 640, .h = 480,
                               .x = 0, .y = 0, .renderW = 640, .renderH = 480});
  sdl2w::log(sdl2w::INFO) << "ok" << sdl2w::endl;
  sdl2w::Window::unInit();
}
```

Prefer `import sdl2w` unless you need a smaller surface.
Import `bmin.string_interop` separately for extra `std::string_view` helpers.

## clangd / editor

```bash
./compile-commands.sh
```

Then restart clangd. Repo settings enable `--experimental-modules-support`.
