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
include $(_SDL2W_MAKE_DIR)/config.mk
include $(_SDL2W_MAKE_DIR)/module-graph.mk
SDL2W_MODULES_DIR := $(abspath $(_SDL2W_MAKE_DIR)/..)

ifeq ($(wildcard $(SDL2W_MODULES_DIR)/../lib/libsdl2w_modules.a),)
  SDL2W_SRC_DIR ?= $(abspath $(SDL2W_MODULES_DIR)/..)
  BMIN_REPO_DIR ?= $(abspath $(SDL2W_SRC_DIR)/../bmin)
  SDL2W_LIB ?= $(SDL2W_MODULES_DIR)/build/libsdl2w_modules.a
  BMIN_MODULES_DIR ?= $(BMIN_REPO_DIR)/src/modules
  BMIN_LIB ?= $(BMIN_REPO_DIR)/bmin/lib/libbmin_modules.a
  SDL2W_LIBDIR ?= $(SDL2W_MODULES_DIR)/build
  BMIN_LIBDIR ?= $(BMIN_REPO_DIR)/bmin/lib
else
  SDL2W_ROOT ?= $(abspath $(SDL2W_MODULES_DIR)/..)
  SDL2W_LIB ?= $(SDL2W_ROOT)/lib/libsdl2w_modules.a
  BMIN_MODULES_DIR ?= $(SDL2W_MODULES_DIR)/bmin
  BMIN_LIB ?= $(SDL2W_ROOT)/lib/libbmin_modules.a
  SDL2W_LIBDIR ?= $(SDL2W_ROOT)/lib
  BMIN_LIBDIR ?= $(SDL2W_ROOT)/lib
endif

include $(BMIN_MODULES_DIR)/make/module-graph.mk
BMIN_MODULE_SOURCE_FILES := \
	$(addprefix $(BMIN_MODULES_DIR)/,$(addsuffix .cppm,$(BMIN_INTERFACE_MODULES)))
SDL2W_MODULE_SOURCE_FILES := \
	$(addprefix $(SDL2W_MODULES_DIR)/,$(addsuffix .cppm,$(SDL2W_INTERFACE_MODULES)))

.DEFAULT_GOAL ?= all

.PHONY: sdl2w-bmi sdl2w-ensure-lib bmin-bmi

ifeq ($(TARGET),wasm)

include $(_SDL2W_MAKE_DIR)/emsdk.mk
include $(_SDL2W_MAKE_DIR)/wasm-flags.mk

SDL2W_PCMDIR ?= pcm.cache
SDL2W_CXXFLAGS ?= $(EMCC_CXXFLAGS) -fprebuilt-module-path=$(SDL2W_PCMDIR) -I$(SDL2W_MODULES_DIR) -I$(BMIN_MODULES_DIR)
SDL2W_PRELOAD ?= --preload-file assets

BMIN_WASM_OBJS = \
	$(patsubst %,.bmin-bmi/%.o,$(BMIN_INTERFACE_MODULES)) \
	$(patsubst %,.bmin-bmi/%-impl.o,$(BMIN_IMPLEMENTATION_MODULES))

SDL2W_WASM_OBJS = \
	$(patsubst %,.sdl2w-bmi/%.o,$(SDL2W_INTERFACE_MODULES)) \
	$(patsubst %,.sdl2w-bmi/%-impl.o,$(SDL2W_IMPLEMENTATION_MODULES))

SDL2W_LDLIBS ?= $(SDL2W_WASM_OBJS) $(BMIN_WASM_OBJS) $(EMCC_LIBS) $(EMCC_EXPORTED) $(SDL2W_PRELOAD)
SDL2W_CACHE_KEY := $(shell sh $(BMIN_MODULES_DIR)/make/cache-key.sh "$(CXX)" "$(subst ",\",$(SDL2W_CXXFLAGS))")
SDL2W_BMI_STAMP = pcm.cache/.sdl2w-ready-$(SDL2W_CACHE_KEY)

sdl2w-bmi: $(SDL2W_BMI_STAMP)

$(SDL2W_BMI_STAMP): $(SDL2W_MODULE_SOURCE_FILES) $(BMIN_MODULE_SOURCE_FILES)
	$(MAKE) -f $(_SDL2W_MAKE_DIR)/build-bmi-em.mk \
		CXX=$(CXX) \
		SDL2W_MOD=$(SDL2W_MODULES_DIR) \
		BMIN_MOD=$(BMIN_MODULES_DIR)
	@touch $(SDL2W_BMI_STAMP)

else

SDL2W_CXXFLAGS ?= $(SDL2W_MODULE_CXXFLAGS) -I$(SDL2W_MODULES_DIR) -I$(BMIN_MODULES_DIR)
SDL2W_LDLIBS ?= -L$(SDL2W_LIBDIR) -L$(BMIN_LIBDIR) -lsdl2w_modules -lbmin_modules
SDL2W_CACHE_KEY := $(shell sh $(BMIN_MODULES_DIR)/make/cache-key.sh "$(CXX)" "$(subst ",\",$(SDL2W_MODULE_CXXFLAGS) $(SDL2W_MODULE_INTERFACE_FLAGS) -I$(SDL2W_MODULES_DIR) -I$(BMIN_MODULES_DIR))")
BMIN_CACHE_KEY := $(shell sh $(BMIN_MODULES_DIR)/make/cache-key.sh "$(CXX)" "$(subst ",\",$(SDL2W_MODULE_CXXFLAGS) $(SDL2W_MODULE_INTERFACE_FLAGS))")
SDL2W_BMI_STAMP = gcm.cache/.sdl2w-ready-$(SDL2W_CACHE_KEY)
BMIN_BMI_STAMP = gcm.cache/.bmin-ready-$(BMIN_CACHE_KEY)

ifeq ($(OS),Windows_NT)
  SDL2W_LDLIBS += -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Darwin)
    SDL2W_LDLIBS += -L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
  else
    SDL2W_LDLIBS += -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
  endif
endif

sdl2w-ensure-lib: $(SDL2W_LIB)

$(SDL2W_LIB):
	@echo "Building sdl2w modules library at $(SDL2W_LIB)"
	$(MAKE) -C $(SDL2W_MODULES_DIR) lib \
		CXX="$(CXX)" \
		SDL2W_MODULE_CXXFLAGS="$(SDL2W_MODULE_CXXFLAGS)" \
		SDL2W_MODULE_INTERFACE_FLAGS="$(SDL2W_MODULE_INTERFACE_FLAGS)"

$(BMIN_LIB):
	@echo "Building bmin modules library at $(BMIN_LIB)"
	$(MAKE) -C $(SDL2W_MODULES_DIR) bmin-bmi

bmin-bmi: $(BMIN_BMI_STAMP)

$(BMIN_BMI_STAMP): $(BMIN_LIB) $(BMIN_MODULE_SOURCE_FILES)
	$(MAKE) -f $(BMIN_MODULES_DIR)/make/build-bmi.mk \
		BMIN_MOD=$(BMIN_MODULES_DIR) \
		CXX="$(CXX)" \
		BMIN_MODULE_CXXFLAGS="$(SDL2W_MODULE_CXXFLAGS)" \
		BMIN_MODULE_INTERFACE_FLAGS="$(SDL2W_MODULE_INTERFACE_FLAGS)"
	@mkdir -p gcm.cache
	@touch $(BMIN_BMI_STAMP)

sdl2w-bmi: $(SDL2W_BMI_STAMP)

$(SDL2W_BMI_STAMP): $(SDL2W_LIB) $(BMIN_BMI_STAMP) $(SDL2W_MODULE_SOURCE_FILES)
	$(MAKE) -f $(_SDL2W_MAKE_DIR)/build-bmi.mk \
		SDL2W_MOD=$(SDL2W_MODULES_DIR) \
		BMIN_MOD=$(BMIN_MODULES_DIR) \
		CXX="$(CXX)" \
		SDL2W_MODULE_CXXFLAGS="$(SDL2W_MODULE_CXXFLAGS)" \
		SDL2W_MODULE_INTERFACE_FLAGS="$(SDL2W_MODULE_INTERFACE_FLAGS)"
	@touch $(SDL2W_BMI_STAMP)

endif
