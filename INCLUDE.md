# Including SDL2W in your game

SDL2W and bmin ship together in two parallel forms. Pick one form for an
executable and do not mix header and module entities in the same program.

| Form | SDL2W | Matching bmin |
|---|---|---|
| Classic | headers + `libsdl2w.a` | headers + `libbmin.a` |
| Modules | `import sdl2w` + `libsdl2w_modules.a` | bmin modules + `libbmin_modules.a` |

## Build and copy the bundle

```bash
make -C path/to/sdl2w/src native
path/to/sdl2w/copy-sdl2w-artifacts.sh path/to/yourgame/lib/sdl2w
```

The copied layout is:

```text
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
    sdl2w.cppm
    sdl2w.*.cppm
    sdl2w.*.cpp
    bmin/
    make/use.mk
```

Use the bmin pair from this bundle rather than another checkout. SDL2W public
types contain bmin types, so the versions must remain aligned.

## Classic headers

Add `lib/sdl2w/include` to the include path and `lib/sdl2w/lib` to the library
path, then link `-lsdl2w -lbmin` plus the SDL2 libraries.

```cpp
#include "Window.h"
#include "AssetLoader.h"
#include "L10n.h"
```

The bmin headers are below `include/bmin`, matching the includes used by the
SDL2W headers.

## Named modules

The helper rebuilds compiler-local BMIs and supplies the matching library and
SDL flags:

```makefile
include lib/sdl2w/modules/make/use.mk

main.o: main.cpp sdl2w-bmi
	$(CXX) $(SDL2W_CXXFLAGS) -c main.cpp -o $@

yourgame: main.o
	$(CXX) $(SDL2W_CXXFLAGS) -o $@ main.o $(SDL2W_LDLIBS)
```

```cpp
import sdl2w;

LOG(INFO) << "Game started" << LOG_ENDL;
const char* title = TRANSLATE("Welcome!");
```

`LOG`, `LOG_LINE`, `LOG_ENDL`, `THROW_RUNTIME_ERROR`, `TRANSLATE`, and the log
levels are exported typed declarations in the module API. The classic header
API continues to provide its existing macros.

Import `bmin.string_interop` separately when its `std::string_view` helpers
are required. Do not also include bmin headers in a module consumer.

The native module pair targets GCC 15. On macOS, the installed helper selects
`g++-15` because `/usr/bin/g++` is Apple Clang. Wasm uses em++ and locally
generated Clang PCMs.

See [src/modules/MODULES.md](src/modules/MODULES.md) and the dual-mode
[example](example/Makefile).
