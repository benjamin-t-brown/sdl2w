CODE = \
lib/Window.cpp \
lib/Draw.cpp \
lib/Logger.cpp \
lib/Store.cpp \
lib/AssetLoader.cpp \
lib/Events.cpp \
lib/Animation.cpp \
lib/L10n.cpp \
lib/Init.cpp \
lib/EmscriptenHelpers.cpp

TARGET ?= native
.DEFAULT_GOAL := native
BASE_BUILD_DIR = build

include modules/make/config.mk

BMIN_URL ?= https://github.com/benjamin-t-brown/bmin.git
BMIN_REF ?= experiment/cpp-modules
BMIN_REPO ?= ../bmin
BMIN_SRC = $(BMIN_REPO)/src
BMIN_BUILD_OUT = $(BMIN_REPO)/bmin
BMIN_BUILT_HEADER_LIB = $(BMIN_BUILD_OUT)/lib/libbmin.a
BMIN_BUILT_MODULE_LIB = $(BMIN_BUILD_OUT)/lib/libbmin_modules.a
BMIN_BUILT_INCLUDE = $(BMIN_BUILD_OUT)/include
BMIN_BUILT_MODULES = $(BMIN_BUILD_OUT)/modules

DEPS_DIR = .
DEPS_BMIN_DIR = $(DEPS_DIR)/bmin
DEPS_BMIN_HEADER_LIB = $(DEPS_BMIN_DIR)/libbmin.a
DEPS_BMIN_MODULE_LIB = $(DEPS_BMIN_DIR)/libbmin_modules.a
DEPS_BMIN_MODULES = $(DEPS_BMIN_DIR)/modules
DEPS_BMIN_STAMP = $(DEPS_BMIN_DIR)/.artifacts-ready

INSTALL_DIR = ../sdl2w

EMCC_SDL_PORTS = \
-s USE_SDL=2 \
-s USE_SDL_IMAGE=2 \
-s USE_SDL_MIXER=2 \
-s SDL2_IMAGE_FORMATS='["png"]' \
-s USE_SDL_TTF=2 \
-s USE_SDL_GFX=2

DEBUG = true
ifeq ($(TARGET),wasm)
  CLASSIC_INCLUDES = -iquote .
  CLASSIC_FLAGS = -Wall -std=c++23 -Oz -flto $(EMCC_SDL_PORTS)
  CLASSIC_LIBS = $(DEPS_BMIN_HEADER_LIB)
  CXX = em++
  AR = emar
else
  CLASSIC_INCLUDES = -iquote .
  CLASSIC_FLAGS = -Wall -std=c++23
  CLASSIC_LIBS = -L$(DEPS_BMIN_DIR) -lbmin
  AR ?= ar
  ifeq ($(OS),Windows_NT)
    CLASSIC_LIBS += -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
  else
    ifeq ($(SDL2W_HOST_OS),Darwin)
      CLASSIC_LIBS += -L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
      CLASSIC_INCLUDES += -I/opt/homebrew/include -I/usr/local/include
    else
      CLASSIC_LIBS += -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
    endif
  endif
  ifeq ($(DEBUG),true)
    CLASSIC_FLAGS += -g
  endif
endif

OBJ_OUTPUT_DIR = $(BASE_BUILD_DIR)/obj
LIB_OUTPUT_DIR = $(BASE_BUILD_DIR)/lib
TOOLS_OUTPUT_DIR = $(BASE_BUILD_DIR)/tools
OBJECTS = $(patsubst %.cpp,$(OBJ_OUTPUT_DIR)/%.o,$(CODE))
DEPENDS = $(patsubst %.cpp,$(OBJ_OUTPUT_DIR)/%.d,$(CODE))
LIB_SDL2W = $(LIB_OUTPUT_DIR)/libsdl2w.a
HEADER_SRC_DIR = lib

.PHONY: all native wasm test dist install-headers install-modules clean clean-deps \
  bmin modules tools $(OBJ_OUTPUT_DIR) $(LIB_OUTPUT_DIR) $(TOOLS_OUTPUT_DIR)

$(BMIN_REPO)/.git:
	@mkdir -p $(dir $(BMIN_REPO))
	@if [ ! -d "$(BMIN_REPO)/.git" ]; then \
		echo "Cloning bmin from $(BMIN_URL) ($(BMIN_REF))..."; \
		git clone --depth 1 --branch $(BMIN_REF) $(BMIN_URL) $(BMIN_REPO); \
	fi

$(BMIN_BUILT_HEADER_LIB): $(BMIN_REPO)/.git
	@echo "Building both bmin APIs for the SDL2W bundle..."
	@$(MAKE) -C $(BMIN_SRC) native \
		CXX="$(CXX)" \
		BMIN_MODULE_CXXFLAGS="$(SDL2W_MODULE_CXXFLAGS)" \
		BMIN_MODULE_INTERFACE_FLAGS="$(SDL2W_MODULE_INTERFACE_FLAGS)"

$(BMIN_BUILT_MODULE_LIB): $(BMIN_BUILT_HEADER_LIB)
	@test -f $@

$(DEPS_BMIN_STAMP): $(BMIN_BUILT_HEADER_LIB) $(BMIN_BUILT_MODULE_LIB)
	rm -rf $(DEPS_BMIN_DIR)
	@mkdir -p $(DEPS_BMIN_DIR)/modules
	cp -R $(BMIN_BUILT_INCLUDE)/* $(DEPS_BMIN_DIR)/
	cp -f $(BMIN_BUILT_HEADER_LIB) $(DEPS_BMIN_HEADER_LIB)
	cp -f $(BMIN_BUILT_MODULE_LIB) $(DEPS_BMIN_MODULE_LIB)
	cp -R $(BMIN_BUILT_MODULES)/* $(DEPS_BMIN_MODULES)/
	@touch $@

bmin: $(DEPS_BMIN_STAMP)

$(OBJECTS): | $(DEPS_BMIN_STAMP)

native:
	@$(MAKE) all TARGET=native

# The classic wasm API remains available through TARGET=wasm. The existing
# named-module wasm bundle is also produced from the same checked-out sources.
wasm: $(DEPS_BMIN_STAMP)
	@$(MAKE) $(LIB_SDL2W) TARGET=wasm
	@$(MAKE) -C modules lib TARGET=wasm
	@$(MAKE) -C $(BMIN_SRC) clean
	@$(MAKE) -C $(BMIN_SRC) all TARGET=wasm
	@mkdir -p $(INSTALL_DIR)/lib/wasm
	@cp -f $(LIB_SDL2W) $(INSTALL_DIR)/lib/wasm/
	@cp -f $(BMIN_BUILT_HEADER_LIB) $(INSTALL_DIR)/lib/wasm/libbmin.a
	@cp -f modules/build-wasm/libsdl2w_modules.a $(INSTALL_DIR)/lib/wasm/
	@cp -f modules/build-wasm/libbmin_modules.a $(INSTALL_DIR)/lib/wasm/
	@$(MAKE) install-headers TARGET=wasm
	@$(MAKE) -C modules install-files
	@echo "Created classic and module wasm archives at $(INSTALL_DIR)/lib/wasm"

all: dist

test: native
	@$(MAKE) -C modules check
	@$(MAKE) -C ../example clean all

$(OBJ_OUTPUT_DIR) $(LIB_OUTPUT_DIR) $(TOOLS_OUTPUT_DIR):
	@mkdir -p $@

$(LIB_SDL2W): $(OBJECTS) | $(LIB_OUTPUT_DIR)
	$(AR) rcs $@ $(OBJECTS)

install-headers: $(LIB_SDL2W) $(DEPS_BMIN_STAMP)
	@echo "Installing classic SDL2W headers and libraries at $(INSTALL_DIR)"
	@mkdir -p $(INSTALL_DIR)/lib $(INSTALL_DIR)/include/bmin
	cp -f $(LIB_SDL2W) $(INSTALL_DIR)/lib/
	cp -f $(DEPS_BMIN_HEADER_LIB) $(INSTALL_DIR)/lib/
	cp -f $(HEADER_SRC_DIR)/*.h $(INSTALL_DIR)/include/
	cp -R $(DEPS_BMIN_DIR)/internal $(INSTALL_DIR)/include/bmin/
	cp -f $(DEPS_BMIN_DIR)/*.h $(INSTALL_DIR)/include/bmin/

install-modules: $(DEPS_BMIN_STAMP)
	@$(MAKE) -C modules install TARGET=native \
		CXX="$(CXX)" \
		SDL2W_MODULE_CXXFLAGS="$(SDL2W_MODULE_CXXFLAGS)" \
		SDL2W_MODULE_INTERFACE_FLAGS="$(SDL2W_MODULE_INTERFACE_FLAGS)"
	@mkdir -p $(INSTALL_DIR)/lib $(INSTALL_DIR)/modules/bmin
	cp -f $(DEPS_BMIN_MODULE_LIB) $(INSTALL_DIR)/lib/
	cp -R $(DEPS_BMIN_MODULES)/* $(INSTALL_DIR)/modules/bmin/

dist: install-headers install-modules
	@echo "Created dual-ship SDL2W bundle at $(INSTALL_DIR)"

modules: $(DEPS_BMIN_STAMP)
	@$(MAKE) -C modules all \
		CXX="$(CXX)" \
		SDL2W_MODULE_CXXFLAGS="$(SDL2W_MODULE_CXXFLAGS)" \
		SDL2W_MODULE_INTERFACE_FLAGS="$(SDL2W_MODULE_INTERFACE_FLAGS)"

tools: $(DEPS_BMIN_STAMP)
	@$(MAKE) -C modules lib
	@$(MAKE) -C tools

-include $(DEPENDS)

$(OBJ_OUTPUT_DIR)/%.o: %.cpp | $(OBJ_OUTPUT_DIR)
	@mkdir -p $(@D)
	$(CXX) $(CLASSIC_FLAGS) $(CLASSIC_INCLUDES) -MMD -MP -c $< -o $@

clean:
	@echo "Cleaning SDL2W build outputs..."
	rm -rf $(BASE_BUILD_DIR)
	rm -rf $(DEPS_BMIN_DIR)
	rm -rf $(INSTALL_DIR)
	@if [ -f modules/Makefile ]; then $(MAKE) -C modules clean; fi
	@if [ -f "$(BMIN_SRC)/Makefile" ]; then $(MAKE) -C $(BMIN_SRC) clean; fi

clean-deps: clean
	@echo "Removing bmin clone at $(BMIN_REPO)..."
	rm -rf $(BMIN_REPO)
