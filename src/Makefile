CLIENT_LIB_CODE=\

SHARED_LIB_CODE=\
lib/Window.cpp\
lib/Draw.cpp\
lib/Logger.cpp\
lib/Store.cpp\
lib/AssetLoader.cpp\
lib/Events.cpp\
lib/Animation.cpp\
lib/L10n.cpp\
lib/Init.cpp\
lib/EmscriptenHelpers.cpp

CLIENT_NONLIB_CODE=\

CLIENT_CODE=\
$(CLIENT_LIB_CODE)\
$(CLIENT_NONLIB_CODE)

SHARED_CODE=\
$(SHARED_LIB_CODE)\
$(SHARED_NONLIB_CODE)

TEST_CODE=\

TEST_CODE_UI=\

# Target can be 'native' or 'wasm'. Default to native.
TARGET ?= native
MIYOOA30 ?= false
MIYOOMINI ?= false

# Base output directories
BASE_BUILD_DIR = build
NATIVE_DIR_SUFFIX = native
WASM_DIR_SUFFIX = wasm

# Original EMCC_EXPORTED variable (used for wasm linking)
EMCC_EXPORTED=\
-s EXPORTED_FUNCTIONS='[\
 "_main",\
 "_enableSound",\
 "_disableSound",\
 "_setVolume",\
 "_setKeyDown",\
 "_setKeyUp",\
 "_setKeyStatus"\
]'\
-s EXPORTED_RUNTIME_METHODS=['ccall']

DEBUG=true # Keep original DEBUG flag setting

# Conditional settings based on TARGET
ifeq ($(TARGET),wasm)
    CXX = em++
    TARGET_SUFFIX = $(WASM_DIR_SUFFIX)

    # WASM specific flags
    EMCC_BASE_OPTS = -Oz -flto # Optimizations and LTO
    EMCC_SDL_SETTINGS = -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 -s USE_SDL_GFX=2

    # Flags for compiling .o files and also for linking
    EMCC_COMP_LINK_COMMON = $(EMCC_BASE_OPTS) $(EMCC_SDL_SETTINGS) $(EMCC_ENV_SETTINGS)

    CURRENT_COMPILE_FLAGS_BASE = -Wall -std=c++17 -I.
    ifeq ($(DEBUG),true)
        CURRENT_COMPILE_FLAGS_BASE += -g
    endif
    
    # Flags for compiling .o files
    CURRENT_FLAGS = $(CURRENT_COMPILE_FLAGS_BASE) $(EMCC_COMP_LINK_COMMON)
    
    # Flags for final linking stage (e.g. creating the .wasm file for an executable/tool)
    CURRENT_LINK_FLAGS = $(EMCC_COMP_LINK_COMMON) $(EMCC_EXPORTED) -lidbfs.js
    CURRENT_LIBS = # SDL libs are handled by -s flags in EMCC_COMP_LINK_COMMON for emcc

    CURRENT_LIB_SDL2W_NAME = libsdl2w.a
    CURRENT_INSTALL_DIR = ../sdl2w

else # native (default)
    CXX = g++
    TARGET_SUFFIX = $(NATIVE_DIR_SUFFIX)

    CURRENT_COMPILE_FLAGS = -O1 -Wall -std=c++17 -I.
    ifeq ($(DEBUG),true)
        CURRENT_COMPILE_FLAGS += -g
    endif
    CURRENT_FLAGS = $(CURRENT_COMPILE_FLAGS) # For .o compilation

    ifeq ($(MIYOOA30),true)
        CURRENT_FLAGS += -DMIYOOA30
    endif
    ifeq ($(MIYOOMINI),true)
        CURRENT_FLAGS += -DMIYOOA30
    endif

    CURRENT_LINK_FLAGS =
    ifeq ($(OS),Windows_NT)
        # CURRENT_LINK_FLAGS += -fuse-ld=lld # can uncomment if needed
    endif

    ifeq ($(OS),Windows_NT)
        CURRENT_LIBS = -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
    else
        UNAME_S := $(shell uname -s)
        ifeq ($(UNAME_S),Darwin)
            CURRENT_LIBS = -L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
            CURRENT_FLAGS += -I/opt/homebrew/include -I/usr/local/include # Add include paths to compile flags for native
        else
            CURRENT_LIBS = -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
        endif
    endif
    CURRENT_LIB_SDL2W_NAME = libsdl2w.a
    CURRENT_INSTALL_DIR = ../sdl2w
endif

CURRENT_OBJ_DIR = $(BASE_BUILD_DIR)/obj/$(TARGET_SUFFIX)
CURRENT_LIB_OUTPUT_DIR = $(BASE_BUILD_DIR)/lib/$(TARGET_SUFFIX)
CURRENT_BIN_OUTPUT_DIR = $(BASE_BUILD_DIR)/bin/$(TARGET_SUFFIX)

# Object and dependency file definitions using CURRENT_OBJ_DIR
CLIENT_OBJECTS = $(patsubst %.cpp,$(CURRENT_OBJ_DIR)/%.o,$(CLIENT_CODE))
SHARED_OBJECTS = $(patsubst %.cpp,$(CURRENT_OBJ_DIR)/%.o,$(SHARED_CODE))
TEST_OBJECTS = $(patsubst %.cpp,$(CURRENT_OBJ_DIR)/%.o,$(TEST_CODE))
TEST_UI_OBJECTS = $(patsubst %.cpp,$(CURRENT_OBJ_DIR)/%.o,$(TEST_CODE_UI))

CLIENT_DEPENDS := $(patsubst %.cpp,$(CURRENT_OBJ_DIR)/%.d,$(CLIENT_CODE))
SHARED_DEPENDS := $(patsubst %.cpp,$(CURRENT_OBJ_DIR)/%.d,$(SHARED_CODE))
TEST_DEPENDS := $(patsubst %.cpp,$(CURRENT_OBJ_DIR)/%.d,$(TEST_CODE))
TEST_UI_DEPENDS := $(patsubst %.cpp,$(CURRENT_OBJ_DIR)/%.d,$(TEST_CODE_UI))

ALL_CLIENT_OBJECTS=$(CLIENT_OBJECTS) $(SHARED_OBJECTS)
ALL_CLIENT_DEPENDS=$(CLIENT_DEPENDS) $(SHARED_DEPENDS)

# Define the name of the static library for the current target
LIB_SDL2W = $(CURRENT_LIB_OUTPUT_DIR)/$(CURRENT_LIB_SDL2W_NAME)

# Installation directories for the current target
INSTALL_DIR = $(CURRENT_INSTALL_DIR)
INSTALL_LIB_DIR = $(INSTALL_DIR)/lib
INSTALL_INCLUDE_DIR = $(INSTALL_DIR)/include

# Header source directories (relative to this Makefile in src/)
HEADER_SRC_DIRS = lib

.PHONY: tools clean $(DIRS_TO_CREATE)

# Top-level targets for specific builds
native:
	@$(MAKE) all TARGET=native

wasm:
	@$(MAKE) all TARGET=wasm

miyooa30:
	@$(MAKE) all TARGET=native MIYOOA30=true

miyoomini:
	@$(MAKE) all TARGET=native MIYOOMINI=true

all: dist

# Ensure output directories exist
DIRS_TO_CREATE = $(CURRENT_OBJ_DIR)/lib $(CURRENT_OBJ_DIR)/tools $(CURRENT_LIB_OUTPUT_DIR) $(CURRENT_BIN_OUTPUT_DIR)
$(DIRS_TO_CREATE):
	@mkdir -p $@

# Rule for the static library
$(LIB_SDL2W): $(ALL_CLIENT_OBJECTS) | $(CURRENT_LIB_OUTPUT_DIR)
	ar rcs $@ $(ALL_CLIENT_OBJECTS)

# 'dist' target: installs library, headers, and tools for the current target
dist: $(LIB_SDL2W)
	@echo "Creating library and headers for $(TARGET) at $(INSTALL_DIR)"
	@mkdir -p $(INSTALL_LIB_DIR)
	@mkdir -p $(INSTALL_INCLUDE_DIR)
	@cp $(LIB_SDL2W) $(INSTALL_LIB_DIR)/
	@echo "Copying header files from $(HEADER_SRC_DIRS) to $(INSTALL_INCLUDE_DIR)/"
	cp -f lib/*.h $(INSTALL_INCLUDE_DIR)/
	@echo "Created $(TARGET) sdl2w folder at top level directory."

tools: L10nScanner Anims

Anims: tools/Anims.cpp $(ALL_CLIENT_OBJECTS)
	$(CXX) $(CURRENT_FLAGS) $< $(filter %.o,$^) $(CURRENT_LINK_FLAGS) -o $@ $(CURRENT_LIBS)

L10nScanner: tools/L10nScanner.cpp 
	$(CXX) $(CURRENT_FLAGS) $< $(CURRENT_LINK_FLAGS) -o $@

# Include dependency files
-include $(ALL_CLIENT_DEPENDS)

# Compilation rule for .o files
$(CURRENT_OBJ_DIR)/%.o: %.cpp | $(DIRS_TO_CREATE)
	@mkdir -p $(@D)
	$(CXX) $(CURRENT_FLAGS) -MMD -MP -c $< -o $@

# 'clean' target: removes all build artifacts for both targets
clean:
	@echo "Cleaning for all targets..."
	rm -rf $(BASE_BUILD_DIR)
	# Remove old top-level artifacts if any from previous Makefile structure
	rm -f libsdl2w.a Anims Anims.exe L10nScanner L10nScanner.exe
	# Remove old installation directories
	rm -rf ../sdl2w_native ../sdl2w_wasm # New install dirs
	rm -rf ../sdl2w # Old install dir from previous Makefile structure
	rm -f precompiled.h.gch # If this is still relevant
	rm -rf main.d # If this was from example builds in src/
