# Using SDL2W C++ modules

SDL2W ships two parallel native APIs from the same checkout:

| Consumer | API | Link libraries |
|---|---|---|
| Classic | `#include "Window.h"` | `-lsdl2w -lbmin` |
| Modules | `import sdl2w` | `-lsdl2w_modules -lbmin_modules` |

Do not mix the header and module forms of SDL2W or bmin in one program. They
are parallel APIs, not aliases for the same C++ entities. The shipped example
compiles once through each API to keep their public surfaces aligned.

## Module layout

Each named module has a small `.cppm` interface containing its public
declarations and required inline code. Non-template definitions live in the
matching `.cpp` module implementation unit. The umbrella `sdl2w.cppm` only
re-exports the supported public modules.

After `make -C src native`:

```text
sdl2w/lib/libsdl2w.a
sdl2w/lib/libbmin.a
sdl2w/include/*.h
sdl2w/include/bmin/
sdl2w/lib/libsdl2w_modules.a
sdl2w/lib/libbmin_modules.a
sdl2w/modules/*.cppm
sdl2w/modules/*.cpp
sdl2w/modules/bmin/
sdl2w/modules/make/use.mk
```

## Module consumer Makefile

```makefile
include path/to/sdl2w/modules/make/use.mk

main.o: main.cpp | sdl2w-bmi
	$(CXX) $(SDL2W_CXXFLAGS) -c main.cpp -o $@

app: main.o
	$(CXX) $(SDL2W_CXXFLAGS) -o $@ main.o $(SDL2W_LDLIBS)
```

```cpp
import sdl2w;

LOG(INFO) << "Game started" << LOG_ENDL;
const char* title = TRANSLATE("Welcome!");
```

The convenience spellings are exported functions and constants rather than
macros. `LOG_LINE` and `THROW_RUNTIME_ERROR` preserve the call site with
`std::source_location`. Classic consumers retain the macros from the headers.

`import sdl2w` is the supported public entry point. The `sdl2w.*` component
module names are implementation details used to organize the library build and
may change without compatibility guarantees. Import `bmin.string_interop`
separately for its `std::string_view` helpers.

BMIs are compiler-local and are intentionally rebuilt by `use.mk`. Cache stamps
include the selected compiler, compiler version, and module flags, so changing
toolchains or flags invalidates incompatible BMIs automatically. The native
bmin + SDL2W module pair currently targets GCC 15; on macOS the helpers select
`g++-15` because `/usr/bin/g++` is Apple Clang. Every `.cppm` producer receives
`-x c++` explicitly.

## Checks

```bash
make -C src native                 # builds and installs both APIs
make -C src/modules check          # graph, public import, internal components
make -C example clean all          # same application, both APIs
make -C src test                   # runs all three checks above
```

Wasm continues to compile the module sources with em++/Clang PCMs. GCC BMIs
(`gcm.cache`) and Clang PCMs (`pcm.cache`) are not interchangeable.
