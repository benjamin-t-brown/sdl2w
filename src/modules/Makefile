# sdl2w C++ modules (native GCC, or em++ when TARGET=wasm).
# Native consumers import sdl2w and link libsdl2w_modules.a.
# Wasm consumers rebuild Clang PCMs with em++ (see make/build-bmi-em.mk).

include make/config.mk

AR ?= ar
INCLUDES = -I. -I$(BMIN_MOD)

BUILD = build
INSTALL_DIR = ../../sdl2w
BMIN_REPO ?= ../../bmin
BMIN_MOD ?= $(BMIN_REPO)/src/modules
BMIN_LIB ?= $(BMIN_REPO)/bmin/lib/libbmin_modules.a
LIB_MODULES = $(BUILD)/libsdl2w_modules.a

BMIN_MAKE_DIR = $(BMIN_MOD)/make
BMIN_BMI_MK = $(BMIN_MAKE_DIR)/build-bmi.mk
include $(BMIN_MAKE_DIR)/module-graph.mk
BMIN_MODULE_SOURCES = \
	$(addprefix $(BMIN_MOD)/,$(addsuffix .cppm,$(BMIN_INTERFACE_MODULES))) \
	$(addprefix $(BMIN_MOD)/,$(addsuffix .cpp,$(BMIN_IMPLEMENTATION_MODULES)))

ifeq ($(OS),Windows_NT)
  SDL_LIBS = -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Darwin)
    SDL_LIBS = -L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
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

.PHONY: all check modules smoke public-import-check internal-module-checks \
	direct-imports validate-module-graph clean install dist lib bmin-bmi wasm-lib install-files

all: smoke

check: validate-module-graph smoke public-import-check internal-module-checks

modules: smoke

$(BMIN_LIB): $(BMIN_MODULE_SOURCES) $(wildcard $(BMIN_MAKE_DIR)/*)
	@$(MAKE) -C $(BMIN_REPO)/src native

BMIN_CACHE_KEY := $(shell sh $(BMIN_MAKE_DIR)/cache-key.sh "$(CXX)" "$(subst ",\",$(SDL2W_MODULE_CXXFLAGS) $(SDL2W_MODULE_INTERFACE_FLAGS))")
BMIN_BMI_STAMP = gcm.cache/.bmin-ready-$(BMIN_CACHE_KEY)
SDL2W_CACHE_KEY := $(shell sh $(BMIN_MAKE_DIR)/cache-key.sh "$(CXX)" "$(subst ",\",$(SDL2W_MODULE_CXXFLAGS) $(SDL2W_MODULE_INTERFACE_FLAGS) $(INCLUDES))")
SDL2W_CACHE_STAMP = gcm.cache/.sdl2w-config-$(SDL2W_CACHE_KEY)

$(BMIN_BMI_STAMP): $(BMIN_LIB) $(BMIN_BMI_MK) $(BMIN_MODULE_SOURCES)
	$(MAKE) -f $(BMIN_BMI_MK) \
		BMIN_MOD=$(abspath $(BMIN_MOD)) \
		CXX="$(CXX)" \
		BMIN_MODULE_CXXFLAGS="$(SDL2W_MODULE_CXXFLAGS)" \
		BMIN_MODULE_INTERFACE_FLAGS="$(SDL2W_MODULE_INTERFACE_FLAGS)"
	@mkdir -p gcm.cache
	@touch $@

bmin-bmi: $(BMIN_BMI_STAMP)

$(SDL2W_CACHE_STAMP): $(BMIN_BMI_STAMP)
	@mkdir -p gcm.cache
	@touch $@

SDL2W_MODULE_ROOT := .
SDL2W_MODULE_OBJDIR := $(BUILD)
SDL2W_MODULE_INCLUDES := $(INCLUDES)
SDL2W_MODULE_EXTRA_PREREQS := $(BMIN_BMI_STAMP) $(SDL2W_CACHE_STAMP)
include make/native-rules.mk

MODULE_INTERFACE_OBJS = $(SDL2W_INTERFACE_OBJECTS)
MODULE_IMPL_NAMES = $(SDL2W_IMPLEMENTATION_MODULES)

MODULE_IMPL_OBJS = $(patsubst %,$(BUILD)/%-impl.o,$(MODULE_IMPL_NAMES))
MODULE_OBJS = $(MODULE_INTERFACE_OBJS) $(MODULE_IMPL_OBJS)
PUBLIC_IMPORT_SOURCE = direct_import/sdl2w.cpp
INTERNAL_IMPORT_SOURCES = $(filter-out $(PUBLIC_IMPORT_SOURCE),$(wildcard direct_import/*.cpp))
PUBLIC_IMPORT_OBJ = $(patsubst direct_import/%.cpp,$(BUILD)/direct-imports/%.o,$(PUBLIC_IMPORT_SOURCE))
INTERNAL_IMPORT_OBJS = $(patsubst direct_import/%.cpp,$(BUILD)/direct-imports/%.o,$(INTERNAL_IMPORT_SOURCES))

$(BUILD)/%-impl.o: %.cpp $(BUILD)/%.o | $(BUILD)
	$(CXX) $(SDL2W_MODULE_CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/direct-imports:
	@mkdir -p $@

$(BUILD)/direct-imports/%.o: direct_import/%.cpp $(MODULE_INTERFACE_OBJS) | $(BUILD)/direct-imports
	$(CXX) $(SDL2W_MODULE_CXXFLAGS) $(INCLUDES) -c $< -o $@

public-import-check: $(PUBLIC_IMPORT_OBJ)
	@echo "public import: OK"

internal-module-checks: $(INTERNAL_IMPORT_OBJS)
	@echo "internal modules: OK"

direct-imports: public-import-check internal-module-checks

validate-module-graph:
	@set -e; \
	actual='$(sort $(basename $(notdir $(wildcard sdl2w*.cppm))))'; \
	expected='$(sort $(SDL2W_INTERFACE_MODULES))'; \
	if [ "$$actual" != "$$expected" ]; then \
		echo "SDL2W interface graph mismatch"; echo "actual:   $$actual"; echo "declared: $$expected"; exit 1; \
	fi; \
	actual='$(sort $(basename $(notdir $(wildcard sdl2w*.cpp))))'; \
	expected='$(sort $(SDL2W_IMPLEMENTATION_MODULES))'; \
	if [ "$$actual" != "$$expected" ]; then \
		echo "SDL2W implementation graph mismatch"; echo "actual:   $$actual"; echo "declared: $$expected"; exit 1; \
	fi; \
	$(foreach module,$(SDL2W_INTERFACE_MODULES),actual=$$(sed -nE 's/^(export[[:space:]]+)?import[[:space:]]+((sdl2w|bmin)\.[[:alnum:]_]+);/\2/p' $(module).cppm | LC_ALL=C sort | awk 'BEGIN { sep="" } { printf "%s%s", sep, $$0; sep=" " } END { print "" }'); expected='$(sort $(SDL2W_DEPS_$(module)) $(SDL2W_BMIN_DEPS_$(module)))'; if [ "$$actual" != "$$expected" ]; then echo "$(module) import mismatch"; echo "actual:   $$actual"; echo "declared: $$expected"; exit 1; fi; ) \
	echo "SDL2W module graph: OK"

ifeq ($(TARGET),wasm)
$(LIB_MODULES): wasm-lib
$(BMIN_WASM_LIB): wasm-lib

wasm-lib: | $(BUILD)
	$(MAKE) -f make/build-bmi-em.mk \
		CXX=$(CXX) \
		SDL2W_MOD=$(abspath .) \
		BMIN_MOD=$(abspath $(BMIN_MOD))
	$(AR) rcs $(LIB_MODULES) \
		$(patsubst %,.sdl2w-bmi/%.o,$(SDL2W_INTERFACE_MODULES)) \
		$(patsubst %,.sdl2w-bmi/%-impl.o,$(MODULE_IMPL_NAMES))
	$(AR) rcs $(BMIN_WASM_LIB) \
		$(patsubst %,.bmin-bmi/%.o,$(BMIN_INTERFACE_MODULES)) \
		$(patsubst %,.bmin-bmi/%-impl.o,$(BMIN_IMPLEMENTATION_MODULES))

lib: $(LIB_MODULES)
else
$(LIB_MODULES): $(MODULE_OBJS) | $(BUILD)
	$(AR) rcs $@ $(MODULE_OBJS)

lib: $(LIB_MODULES)
endif

$(BUILD)/smoke.o: smoke.cpp $(MODULE_OBJS) | $(BUILD)
	$(CXX) $(SDL2W_MODULE_CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD)/smoke: $(BUILD)/smoke.o $(LIB_MODULES) $(BMIN_LIB)
	$(CXX) $(SDL2W_MODULE_CXXFLAGS) -o $@ $(BUILD)/smoke.o $(LIB_MODULES) $(BMIN_LIB) $(SDL_LIBS)

smoke: $(BUILD)/smoke
	./$(BUILD)/smoke

MODULE_SOURCES = \
	sdl_fwd.h \
	impl_headers.h \
	$(addsuffix .cppm,$(SDL2W_INTERFACE_MODULES)) \
	$(addsuffix .cpp,$(MODULE_IMPL_NAMES))

install-files:
	@mkdir -p $(INSTALL_DIR)/lib
	@mkdir -p $(INSTALL_DIR)/modules
	@mkdir -p $(INSTALL_DIR)/modules/make
	cp -f $(MODULE_SOURCES) $(INSTALL_DIR)/modules/
	cp -f make/config.mk make/module-graph.mk make/native-rules.mk make/use.mk make/build-bmi.mk make/build-bmi-em.mk make/emsdk.mk make/wasm-flags.mk $(INSTALL_DIR)/modules/make/
	cp -f MODULES.md $(INSTALL_DIR)/modules/README.md

install dist: $(LIB_MODULES) install-files
	@echo "Installing sdl2w modules into $(INSTALL_DIR)"
	cp -f $(LIB_MODULES) $(INSTALL_DIR)/lib/
	@echo "Installed libsdl2w_modules.a, module sources, and make/ helpers."

clean:
	rm -rf $(BUILD) build-wasm
	rm -rf gcm.cache pcm.cache .bmin-bmi .sdl2w-bmi
