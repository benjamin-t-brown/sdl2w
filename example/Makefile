# Consumer example: import sdl2w via modules/make/use.mk.
# Native: make / make run
# Wasm:   make js   (requires emsdk; defaults to ../../emsdk)

EXE = SDL2W_EXAMPLE
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

.PHONY: all run clean js wasm
.DEFAULT_GOAL := all

ifeq ($(wildcard ../sdl2w/modules/make/use.mk),)
  include ../src/modules/make/use.mk
else
  include ../sdl2w/modules/make/use.mk
endif

all: $(EXE)$(EXE_SUFFIX)

$(MAIN_OBJ): main.cpp sdl2w-bmi
	$(CXX) $(SDL2W_CXXFLAGS) -c main.cpp -o $@

$(EXE)$(EXE_SUFFIX): $(MAIN_OBJ) sdl2w-bmi
	$(CXX) $(SDL2W_CXXFLAGS) -o $@ $(MAIN_OBJ) $(SDL2W_LDLIBS)

run: $(EXE)$(EXE_SUFFIX)
	./$(EXE)$(EXE_SUFFIX)

js wasm:
	$(MAKE) TARGET=wasm $(EXE).js
	@mkdir -p $(WEB_DIR)
	mv -f $(EXE).js $(EXE).wasm $(EXE).data $(WEB_DIR)/
	@echo "Wrote $(WEB_DIR)/$(EXE).js $(EXE).wasm $(EXE).data"

clean:
	rm -f main.o main-wasm.o $(EXE) $(EXE).exe $(EXE).js $(EXE).wasm $(EXE).data
	rm -rf gcm.cache pcm.cache .bmin-bmi .sdl2w-bmi
