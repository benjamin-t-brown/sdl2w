# Consumer Make helpers for sdl2w modules (and bundled bmin modules).
#
#   include path/to/sdl2w/modules/make/use.mk
#
# Native (GCC):
#   SDL2W_CXXFLAGS   -fmodules-ts, -I …
#   SDL2W_LDLIBS     -lsdl2w_modules -lbmin_modules + SDL
#   sdl2w-bmi        builds BMIs into ./gcm.cache
#
# Wasm (em++): make TARGET=wasm
#   SDL2W_CXXFLAGS   --precompile path, SDL ports
#   SDL2W_LDLIBS     locally compiled module .o + emscripten link flags
#   sdl2w-bmi        builds pcm.cache + .o (Clang PCM must match the objects)

_SDL2W_MAKE_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
SDL2W_MODULES_DIR := $(abspath $(_SDL2W_MAKE_DIR)/..)

ifeq ($(wildcard $(SDL2W_MODULES_DIR)/../lib/libsdl2w_modules.a),)
  SDL2W_SRC_DIR ?= $(abspath $(SDL2W_MODULES_DIR)/..)
  SDL2W_LIB ?= $(SDL2W_MODULES_DIR)/build/libsdl2w_modules.a
  BMIN_MODULES_DIR ?= $(SDL2W_SRC_DIR)/bmin/modules
  BMIN_LIB ?= $(SDL2W_SRC_DIR)/bmin/libbmin_modules.a
  SDL2W_LIBDIR ?= $(SDL2W_MODULES_DIR)/build
  BMIN_LIBDIR ?= $(SDL2W_SRC_DIR)/bmin
else
  SDL2W_ROOT ?= $(abspath $(SDL2W_MODULES_DIR)/..)
  SDL2W_LIB ?= $(SDL2W_ROOT)/lib/libsdl2w_modules.a
  BMIN_MODULES_DIR ?= $(SDL2W_MODULES_DIR)/bmin
  BMIN_LIB ?= $(SDL2W_ROOT)/lib/libbmin_modules.a
  SDL2W_LIBDIR ?= $(SDL2W_ROOT)/lib
  BMIN_LIBDIR ?= $(SDL2W_ROOT)/lib
endif

SDL2W_BMI_STAMP ?= gcm.cache/.sdl2w-ready
BMIN_BMI_STAMP ?= gcm.cache/.bmin-ready

.DEFAULT_GOAL ?= all

.PHONY: sdl2w-bmi sdl2w-ensure-lib bmin-bmi

ifeq ($(TARGET),wasm)

include $(_SDL2W_MAKE_DIR)/emsdk.mk
include $(_SDL2W_MAKE_DIR)/wasm-flags.mk

SDL2W_PCMDIR ?= pcm.cache
SDL2W_BMI_STAMP = pcm.cache/.sdl2w-ready
SDL2W_CXXFLAGS ?= $(EMCC_CXXFLAGS) -fprebuilt-module-path=$(SDL2W_PCMDIR) -I$(SDL2W_MODULES_DIR) -I$(BMIN_MODULES_DIR)
SDL2W_PRELOAD ?= --preload-file assets

BMIN_WASM_OBJS = \
	.bmin-bmi/bmin.types.o \
	.bmin-bmi/bmin.detail.o \
	.bmin-bmi/bmin.dynarray.o \
	.bmin-bmi/bmin.unique_ptr.o \
	.bmin-bmi/bmin.string.o \
	.bmin-bmi/bmin.list.o \
	.bmin-bmi/bmin.queue.o \
	.bmin-bmi/bmin.hash.o \
	.bmin-bmi/bmin.map.o \
	.bmin-bmi/bmin.stringstream.o \
	.bmin-bmi/bmin.string_interop.o \
	.bmin-bmi/bmin.containers.o \
	.bmin-bmi/bmin.detail-impl.o \
	.bmin-bmi/bmin.string-impl.o \
	.bmin-bmi/bmin.stringstream-impl.o \
	.bmin-bmi/bmin.string_interop-impl.o

SDL2W_WASM_OBJS = \
	.sdl2w-bmi/sdl2w.defines.o \
	.sdl2w-bmi/sdl2w.logger.o \
	.sdl2w-bmi/sdl2w.types.o \
	.sdl2w-bmi/sdl2w.events.o \
	.sdl2w-bmi/sdl2w.animation.o \
	.sdl2w-bmi/sdl2w.store.o \
	.sdl2w-bmi/sdl2w.draw.o \
	.sdl2w-bmi/sdl2w.assets.o \
	.sdl2w-bmi/sdl2w.l10n.o \
	.sdl2w-bmi/sdl2w.emscripten.o \
	.sdl2w-bmi/sdl2w.window.o \
	.sdl2w-bmi/sdl2w.init.o \
	.sdl2w-bmi/sdl2w.o

SDL2W_LDLIBS ?= $(SDL2W_WASM_OBJS) $(BMIN_WASM_OBJS) $(EMCC_LIBS) $(EMCC_EXPORTED) $(SDL2W_PRELOAD)

sdl2w-bmi: $(SDL2W_BMI_STAMP)

$(SDL2W_BMI_STAMP):
	$(MAKE) -f $(_SDL2W_MAKE_DIR)/build-bmi-em.mk \
		CXX=$(CXX) \
		SDL2W_MOD=$(SDL2W_MODULES_DIR) \
		BMIN_MOD=$(BMIN_MODULES_DIR)
	@touch $(SDL2W_BMI_STAMP)

else

SDL2W_CXXFLAGS ?= -Wall -std=c++23 -g -fmodules-ts -I$(SDL2W_MODULES_DIR) -I$(BMIN_MODULES_DIR)
SDL2W_LDLIBS ?= -L$(SDL2W_LIBDIR) -L$(BMIN_LIBDIR) -lsdl2w_modules -lbmin_modules

ifeq ($(OS),Windows_NT)
  SDL2W_LDLIBS += -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Darwin)
    SDL2W_CXXFLAGS += -I/opt/homebrew/include -I/usr/local/include
    SDL2W_LDLIBS += -L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
  else
    SDL2W_LDLIBS += -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
  endif
endif

sdl2w-ensure-lib: $(SDL2W_LIB)

$(SDL2W_LIB):
	@echo "Building sdl2w modules library at $(SDL2W_LIB)"
	$(MAKE) -C $(SDL2W_MODULES_DIR) lib

$(BMIN_LIB):
	@echo "Building bmin modules library at $(BMIN_LIB)"
	$(MAKE) -C $(SDL2W_MODULES_DIR) bmin-bmi

bmin-bmi: $(BMIN_BMI_STAMP)

$(BMIN_BMI_STAMP): $(BMIN_LIB)
	$(MAKE) -f $(BMIN_MODULES_DIR)/make/build-bmi.mk BMIN_MOD=$(BMIN_MODULES_DIR)
	@mkdir -p gcm.cache
	@touch $(BMIN_BMI_STAMP)

sdl2w-bmi: $(SDL2W_BMI_STAMP)

$(SDL2W_BMI_STAMP): $(SDL2W_LIB) $(BMIN_BMI_STAMP)
	$(MAKE) -f $(_SDL2W_MAKE_DIR)/build-bmi.mk SDL2W_MOD=$(SDL2W_MODULES_DIR) BMIN_MOD=$(BMIN_MODULES_DIR)
	@touch $(SDL2W_BMI_STAMP)

endif
