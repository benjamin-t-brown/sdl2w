EXE=SDL2W_EXAMPLE
EXE_SUFFIX=

MAIN_ALL=main.cpp

CODE=\

LIB_CODE=\

TARGET ?= native
BASE_BUILD_DIR = build

SDL2W_DIR_NAME = ../
SDL2W_BUILD_OUTPUT_DIR = $(SDL2W_DIR_NAME)/sdl2w

MAIN_ALL_OBJECT=$(MAIN_ALL:.cpp=.o)

OBJECTS=$(CODE:.cpp=.o)
LIB_OBJECTS=$(LIB_CODE:.cpp=.o)

DEPENDS := $(patsubst %.cpp,%.d,$(CODE)) main.d
LIB_DEPENDS := $(patsubst %.cpp,%.d,$(LIB_CODE))

SDL2W_STAMP = $(SDL2W_DIR_NAME)/.source_ok
SDL2W_LIB = lib/sdl2w/libsdl2w.a

# Subset of Emscripten port flags: needed when compiling sdl2w to .o (static lib), not only at final link.
EMCC_SDL_PORTS=\
-s USE_SDL=2\
-s USE_SDL_IMAGE=2\
-s USE_SDL_MIXER=2\
-s SDL2_IMAGE_FORMATS='["png"]'\
-s USE_SDL_TTF=2\
-s USE_SDL_GFX=2

# Link-time (includes EMCC_SDL_PORTS).
EMCC_LIBS=\
$(EMCC_SDL_PORTS)\
-s ALLOW_MEMORY_GROWTH=1\
-s SAFE_HEAP=0\
-s ASSERTIONS=1\
-s INITIAL_MEMORY=326565888\
-s ENVIRONMENT=web\
-s DISABLE_EXCEPTION_CATCHING=0\
-lidbfs.js

EMCC_EXPORTED=\
-s EXPORTED_FUNCTIONS='[\
 "_main",\
 "_enableSound",\
 "_disableSound",\
 "_setVolume",\
 "_setKeyDown",\
 "_setKeyUp",\
 "_setKeyStatus",\
 "_sendEvent"\
]'\
-s EXPORTED_RUNTIME_METHODS=['ccall']

DEBUG = true
LIBS += -Llib/sdl2w -lsdl2w -lbmin
INCLUDES += -I. -Ilib/sdl2w
ifeq ($(TARGET),wasm)
    FLAGS = -Wall -std=c++23 -Oz
    LIBS += $(EMCC_LIBS) $(EMCC_EXPORTED) --preload-file assets
    EXE_SUFFIX = .js
    CXX = em++
else
    FLAGS = -O0 -Wall -std=c++23 -fno-omit-frame-pointer
    ifndef CXX
        CXX = g++
    endif
    ifeq ($(OS),Windows_NT)
        LIBS += -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
    else
        UNAME_S := $(shell uname -s)
        ifeq ($(UNAME_S),Darwin)
            LIBS += -L/opt/homebrew/lib -L/usr/local/lib-lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
            INCLUDES += -I/opt/homebrew/include -I/usr/local/include
        else
            LIBS += -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
        endif
    endif
    ifeq ($(DEBUG),true)
        FLAGS += -g
    endif
endif

.PHONY: all sdl2w_wasm sdl2w-rebuild _copy_sdl2w_artifacts

all: sdl2w
	$(MAKE) $(EXE)

$(EXE): $(OBJECTS) $(LIB_OBJECTS) $(MAIN_ALL_OBJECT) $(SDL2W_LIB)
	$(CXX) $(FLAGS) $(INCLUDES) $(MAIN_ALL_OBJECT) $(OBJECTS) $(LIB_OBJECTS) -o $(EXE)$(EXE_SUFFIX) $(LIBS)

# Convenience alias: builds $(SDL2W_LIB) only when needed (see rule below).
sdl2w: $(SDL2W_LIB)

# Force a full sdl2w rebuild + copy (use after upgrading sdl2w or changing its build).
sdl2w_rebuild:
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" all
	@$(MAKE) _copy_sdl2w_artifacts

$(SDL2W_STAMP):
	@touch $@

$(SDL2W_LIB): $(SDL2W_STAMP)
	@echo "Building standard sdl2w library (running 'make all' in $(SDL2W_DIR_NAME)/src)...";
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" all
	@$(MAKE) _copy_sdl2w_artifacts

sdl2w_wasm: $(SDL2W_STAMP)
	@echo "Building sdl2w library for WebAssembly (running 'make wasm' in $(SDL2W_DIR_NAME)/src)...";
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" wasm
	@$(MAKE) _copy_sdl2w_artifacts

_copy_sdl2w_artifacts:
	@mkdir -p lib/sdl2w/bmin
	@echo "Copying sdl2w + bmin artifacts from $(SDL2W_BUILD_OUTPUT_DIR) to lib/sdl2w..."
	@cp -uv $(SDL2W_BUILD_OUTPUT_DIR)/lib/libsdl2w.a lib/sdl2w/
	@cp -uv $(SDL2W_BUILD_OUTPUT_DIR)/lib/libbmin.a lib/sdl2w/
	@cp -uv $(SDL2W_BUILD_OUTPUT_DIR)/include/*.h lib/sdl2w/
	@cp -Ruv $(SDL2W_BUILD_OUTPUT_DIR)/include/bmin/* lib/sdl2w/bmin/

-include $(DEPENDS)
-include $(LIB_DEPENDS)

%.o: %.cpp
	$(CXX) $(FLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

run:
	./$(EXE)

js: sdl2w_wasm
	$(MAKE) TARGET=wasm $(EXE)
	mv $(EXE).js ../web/
	mv $(EXE).data ../web/
	mv $(EXE).wasm ../web/

debug:
	gdb $(EXE)

clean:
	rm -f $(OBJECTS) $(LIB_OBJECTS) $(MAIN_ALL_OBJECT)
	rm -f $(DEPENDS) $(LIB_DEPENDS)
	rm -f $(EXE) $(EXE).exe
	rm -rf main.d
	rm -rf .build
	rm -rf lib/sdl2w
	rm -rf $(SDL2W_BUILD_OUTPUT_DIR)
	$(MAKE) -C "$(SDL2W_DIR_NAME)/src" clean
	rm -rf $(SDL2W_STAMP)

