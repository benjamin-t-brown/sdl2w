# Consumer example: import sdl2w via modules/make/use.mk.
# Native: make / make run
# Wasm:   make js   (requires emsdk; defaults to ../../emsdk)

EXE = SDL2W_EXAMPLE
HEADER_EXE = SDL2W_EXAMPLE_HEADERS
EXE_SUFFIX =
TARGET ?= native
WEB_DIR = ../web

MAIN_OBJ = main.o

ifeq ($(TARGET),wasm)
  EXE_SUFFIX = .js
  MAIN_OBJ = main-wasm.o
else ifeq ($(OS),Windows_NT)
  EXE_SUFFIX = .exe
endif

.PHONY: all modules headers run clean js wasm
.DEFAULT_GOAL := all

ifeq ($(wildcard ../sdl2w/modules/make/use.mk),)
  include ../src/modules/make/use.mk
else
  include ../sdl2w/modules/make/use.mk
endif

SDL2W_HEADER_ROOT ?= $(abspath ../sdl2w)
SDL2W_HEADER_LIB = $(SDL2W_HEADER_ROOT)/lib/libsdl2w.a
SDL2W_HEADER_CXXFLAGS ?= -Wall -std=c++23 -g -I$(SDL2W_HEADER_ROOT)/include
SDL2W_HEADER_LDLIBS = -L$(SDL2W_HEADER_ROOT)/lib -lsdl2w -lbmin

ifeq ($(OS),Windows_NT)
  SDL2W_HEADER_LDLIBS += -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
else ifeq ($(SDL2W_HOST_OS),Darwin)
  SDL2W_HEADER_CXXFLAGS += -I/opt/homebrew/include -I/usr/local/include
  SDL2W_HEADER_LDLIBS += -L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
else
  SDL2W_HEADER_LDLIBS += -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
endif

all: modules headers
modules: $(EXE)$(EXE_SUFFIX)
headers: $(HEADER_EXE)$(EXE_SUFFIX)

$(MAIN_OBJ): main.cpp | sdl2w-bmi
	$(CXX) $(SDL2W_CXXFLAGS) -DSDL2W_USE_MODULES=1 -c main.cpp -o $@

$(EXE)$(EXE_SUFFIX): $(MAIN_OBJ) | sdl2w-bmi
	$(CXX) $(SDL2W_CXXFLAGS) -o $@ $(MAIN_OBJ) $(SDL2W_LDLIBS)

$(SDL2W_HEADER_LIB):
	$(MAKE) -C ../src native

main-headers.o: main.cpp $(SDL2W_HEADER_LIB)
	$(CXX) $(SDL2W_HEADER_CXXFLAGS) -c main.cpp -o $@

$(HEADER_EXE)$(EXE_SUFFIX): main-headers.o $(SDL2W_HEADER_LIB)
	$(CXX) $(SDL2W_HEADER_CXXFLAGS) -o $@ main-headers.o $(SDL2W_HEADER_LDLIBS)

run: $(EXE)$(EXE_SUFFIX)
	./$(EXE)$(EXE_SUFFIX)

js wasm:
	$(MAKE) TARGET=wasm $(EXE).js
	@mkdir -p $(WEB_DIR)
	mv -f $(EXE).js $(EXE).wasm $(EXE).data $(WEB_DIR)/
	@echo "Wrote $(WEB_DIR)/$(EXE).js $(EXE).wasm $(EXE).data"

clean:
	rm -f main.o main-wasm.o main-headers.o $(EXE) $(EXE).exe $(HEADER_EXE) $(HEADER_EXE).exe $(EXE).js $(EXE).wasm $(EXE).data
	rm -rf gcm.cache pcm.cache .bmin-bmi .sdl2w-bmi
