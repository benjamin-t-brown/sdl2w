# sdl2w C++ modules (native GCC, or em++ when TARGET=wasm).
# Native consumers import sdl2w and link libsdl2w_modules.a.
# Wasm consumers rebuild Clang PCMs with em++ (see make/build-bmi-em.mk).

CXX ?= g++
AR ?= ar
CXXFLAGS = -Wall -std=c++23 -g -fmodules-ts
INCLUDES = -I. -I$(BMIN_MOD)

BUILD = build
INSTALL_DIR = ../../sdl2w
BMIN_MOD = ../bmin/modules
BMIN_LIB = ../bmin/libbmin_modules.a
LIB_MODULES = $(BUILD)/libsdl2w_modules.a

BMIN_BMI_MK = $(BMIN_MOD)/make/build-bmi.mk

ifeq ($(OS),Windows_NT)
  SDL_LIBS = -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Darwin)
    SDL_LIBS = -L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
    INCLUDES += -I/opt/homebrew/include -I/usr/local/include
  else
    SDL_LIBS = -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
  endif
endif

ifeq ($(TARGET),wasm)
  include make/emsdk.mk
  BUILD = build-wasm
  LIB_MODULES = $(BUILD)/libsdl2w_modules.a
  BMIN_WASM_LIB = $(BUILD)/libbmin_modules.a
endif

.PHONY: all modules smoke clean install dist lib bmin-bmi wasm-lib install-files

all: smoke

modules: smoke

$(BUILD):
	@mkdir -p $@

$(BMIN_LIB):
	@$(MAKE) -C .. bmin TARGET=native

BMIN_BMI_STAMP = gcm.cache/.bmin-ready

$(BMIN_BMI_STAMP): $(BMIN_LIB) $(BMIN_BMI_MK)
	$(MAKE) -f $(BMIN_BMI_MK) BMIN_MOD=$(abspath $(BMIN_MOD))
	@mkdir -p gcm.cache
	@touch $@

bmin-bmi: $(BMIN_BMI_STAMP)

$(BUILD)/sdl2w.defines.o: sdl2w.defines.cppm $(BMIN_BMI_STAMP) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.logger.o: sdl2w.logger.cppm $(BUILD)/sdl2w.defines.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.types.o: sdl2w.types.cppm $(BUILD)/sdl2w.defines.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.events.o: sdl2w.events.cppm $(BUILD)/sdl2w.logger.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.animation.o: sdl2w.animation.cppm $(BUILD)/sdl2w.types.o $(BUILD)/sdl2w.logger.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.store.o: sdl2w.store.cppm $(BUILD)/sdl2w.animation.o $(BUILD)/sdl2w.logger.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.draw.o: sdl2w.draw.cppm $(BUILD)/sdl2w.animation.o $(BUILD)/sdl2w.store.o $(BUILD)/sdl2w.logger.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.assets.o: sdl2w.assets.cppm $(BUILD)/sdl2w.draw.o $(BUILD)/sdl2w.store.o $(BUILD)/sdl2w.logger.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.l10n.o: sdl2w.l10n.cppm $(BUILD)/sdl2w.assets.o $(BUILD)/sdl2w.logger.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.emscripten.o: sdl2w.emscripten.cppm $(BUILD)/sdl2w.logger.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.window.o: sdl2w.window.cppm $(BUILD)/sdl2w.draw.o $(BUILD)/sdl2w.events.o $(BUILD)/sdl2w.store.o $(BUILD)/sdl2w.assets.o $(BUILD)/sdl2w.emscripten.o $(BUILD)/sdl2w.logger.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.init.o: sdl2w.init.cppm $(BUILD)/sdl2w.window.o $(BUILD)/sdl2w.l10n.o $(BUILD)/sdl2w.logger.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/sdl2w.o: sdl2w.cppm \
		$(BUILD)/sdl2w.defines.o $(BUILD)/sdl2w.logger.o $(BUILD)/sdl2w.types.o \
		$(BUILD)/sdl2w.events.o $(BUILD)/sdl2w.animation.o $(BUILD)/sdl2w.store.o \
		$(BUILD)/sdl2w.draw.o $(BUILD)/sdl2w.assets.o $(BUILD)/sdl2w.l10n.o \
		$(BUILD)/sdl2w.window.o $(BUILD)/sdl2w.init.o $(BUILD)/sdl2w.emscripten.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

MODULE_OBJS = \
	$(BUILD)/sdl2w.defines.o \
	$(BUILD)/sdl2w.logger.o \
	$(BUILD)/sdl2w.types.o \
	$(BUILD)/sdl2w.events.o \
	$(BUILD)/sdl2w.animation.o \
	$(BUILD)/sdl2w.store.o \
	$(BUILD)/sdl2w.draw.o \
	$(BUILD)/sdl2w.assets.o \
	$(BUILD)/sdl2w.l10n.o \
	$(BUILD)/sdl2w.emscripten.o \
	$(BUILD)/sdl2w.window.o \
	$(BUILD)/sdl2w.init.o \
	$(BUILD)/sdl2w.o

ifeq ($(TARGET),wasm)
$(LIB_MODULES): wasm-lib
$(BMIN_WASM_LIB): wasm-lib

wasm-lib: | $(BUILD)
	$(MAKE) -f make/build-bmi-em.mk \
		CXX=$(CXX) \
		SDL2W_MOD=$(abspath .) \
		BMIN_MOD=$(abspath $(BMIN_MOD))
	$(AR) rcs $(LIB_MODULES) \
		.sdl2w-bmi/sdl2w.defines.o .sdl2w-bmi/sdl2w.logger.o .sdl2w-bmi/sdl2w.types.o \
		.sdl2w-bmi/sdl2w.events.o .sdl2w-bmi/sdl2w.animation.o .sdl2w-bmi/sdl2w.store.o \
		.sdl2w-bmi/sdl2w.draw.o .sdl2w-bmi/sdl2w.assets.o .sdl2w-bmi/sdl2w.l10n.o \
		.sdl2w-bmi/sdl2w.emscripten.o .sdl2w-bmi/sdl2w.window.o .sdl2w-bmi/sdl2w.init.o \
		.sdl2w-bmi/sdl2w.o
	$(AR) rcs $(BMIN_WASM_LIB) \
		.bmin-bmi/bmin.types.o .bmin-bmi/bmin.detail.o .bmin-bmi/bmin.dynarray.o \
		.bmin-bmi/bmin.unique_ptr.o .bmin-bmi/bmin.string.o .bmin-bmi/bmin.list.o \
		.bmin-bmi/bmin.queue.o .bmin-bmi/bmin.hash.o .bmin-bmi/bmin.map.o \
		.bmin-bmi/bmin.stringstream.o .bmin-bmi/bmin.string_interop.o \
		.bmin-bmi/bmin.containers.o \
		.bmin-bmi/bmin.detail-impl.o .bmin-bmi/bmin.string-impl.o \
		.bmin-bmi/bmin.stringstream-impl.o .bmin-bmi/bmin.string_interop-impl.o

lib: $(LIB_MODULES)
else
$(LIB_MODULES): $(MODULE_OBJS) | $(BUILD)
	$(AR) rcs $@ $(MODULE_OBJS)

lib: $(LIB_MODULES)
endif

$(BUILD)/smoke.o: smoke.cpp $(MODULE_OBJS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/smoke: $(BUILD)/smoke.o $(LIB_MODULES) $(BMIN_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $(BUILD)/smoke.o $(LIB_MODULES) $(BMIN_LIB) $(SDL_LIBS)

smoke: $(BUILD)/smoke
	./$(BUILD)/smoke

MODULE_SOURCES = \
	macros.h \
	sdl_fwd.h \
	impl_headers.h \
	sdl2w.defines.cppm \
	sdl2w.logger.cppm \
	sdl2w.types.cppm \
	sdl2w.events.cppm \
	sdl2w.animation.cppm \
	sdl2w.store.cppm \
	sdl2w.draw.cppm \
	sdl2w.assets.cppm \
	sdl2w.l10n.cppm \
	sdl2w.window.cppm \
	sdl2w.init.cppm \
	sdl2w.emscripten.cppm \
	sdl2w.cppm

install-files:
	@mkdir -p $(INSTALL_DIR)/lib
	@mkdir -p $(INSTALL_DIR)/modules
	@mkdir -p $(INSTALL_DIR)/modules/make
	cp -f $(MODULE_SOURCES) $(INSTALL_DIR)/modules/
	cp -f make/use.mk make/build-bmi.mk make/build-bmi-em.mk make/emsdk.mk make/wasm-flags.mk $(INSTALL_DIR)/modules/make/
	cp -f MODULES.md $(INSTALL_DIR)/modules/README.md

install dist: $(LIB_MODULES) install-files
	@echo "Installing sdl2w modules into $(INSTALL_DIR)"
	cp -f $(LIB_MODULES) $(INSTALL_DIR)/lib/
	@echo "Installed libsdl2w_modules.a, module sources, and make/ helpers."

clean:
	rm -rf $(BUILD) build-wasm
	rm -rf gcm.cache pcm.cache .bmin-bmi .sdl2w-bmi
