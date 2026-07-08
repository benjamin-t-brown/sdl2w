CODE=\
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

TARGET ?= native
.DEFAULT_GOAL := native
BASE_BUILD_DIR = build

# bmin: clone to ../bmin, build to ../bmin/bmin/, copy into deps/ for -Ideps + <bmin/...>
BMIN_URL ?= https://github.com/benjamin-t-brown/bmin.git
BMIN_REF ?= main
BMIN_REPO ?= ../bmin
BMIN_SRC = $(BMIN_REPO)/src
BMIN_BUILD_OUT = $(BMIN_REPO)/bmin
BMIN_BUILT_LIB = $(BMIN_BUILD_OUT)/lib/libbmin.a
BMIN_BUILT_INCLUDE = $(BMIN_BUILD_OUT)/include
DEPS_DIR = deps
DEPS_BMIN_LIB = $(DEPS_DIR)/lib/libbmin.a
DEPS_BMIN_INCLUDE_MARKER = $(DEPS_DIR)/bmin/String.h

EMCC_SDL_PORTS=\
-s USE_SDL=2\
-s USE_SDL_IMAGE=2\
-s USE_SDL_MIXER=2\
-s SDL2_IMAGE_FORMATS='["png"]'\
-s USE_SDL_TTF=2\
-s USE_SDL_GFX=2

# On Windows, parallel make + g++ can emit \r and interleave output, showing as garbage (♪◙). Use V=1 to see full commands.
ifeq ($(V),1)
  Q =
else
  Q = @
endif

DEBUG=true
ifeq ($(TARGET),wasm)
    INCLUDES= -I. -I$(DEPS_DIR)
    FLAGS = -Wall -std=c++23 -Oz -flto  $(EMCC_SDL_PORTS)
    LIBS = $(DEPS_BMIN_LIB)
    CXX = em++
    AR = emar
else
    INCLUDES += -I. -I$(DEPS_DIR)
    FLAGS += -Wall -std=c++23
    LIBS += -L$(DEPS_DIR)/lib -lbmin
    ifeq ($(OS),Windows_NT)
        LIBS += -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
    else
        UNAME_S := $(shell uname -s)
        ifeq ($(UNAME_S),Darwin)
            LIBS += -L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
            INCLUDES += -I/opt/homebrew/include -I/usr/local/include
        else
            LIBS += -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
        endif
    endif
    ifeq ($(DEBUG),true)
        FLAGS += -g
    endif
endif
TARGET_SUFFIX = $(TARGET)

OBJ_OUTPUT_DIR = $(BASE_BUILD_DIR)/obj
LIB_OUTPUT_DIR = $(BASE_BUILD_DIR)/lib
BIN_OUTPUT_DIR = $(BASE_BUILD_DIR)/bin
TOOLS_OUTPUT_DIR = $(BASE_BUILD_DIR)/tools
INSTALL_DIR = ../sdl2w

DIRS_TO_CREATE = $(OBJ_OUTPUT_DIR) $(TOOLS_OUTPUT_DIR) $(LIB_OUTPUT_DIR) $(BIN_OUTPUT_DIR)

OBJECTS = $(patsubst %.cpp,$(OBJ_OUTPUT_DIR)/%.o,$(CODE))
DEPENDS = $(patsubst %.cpp,$(OBJ_OUTPUT_DIR)/%.d,$(CODE))

LIB_SDL2W = $(LIB_OUTPUT_DIR)/libsdl2w.a

HEADER_SRC_DIR = lib

# Per-TU compile timing: make ftime-report
# Windows: C:/progs/msys2/msys2_shell.cmd -defterm -here -no-start -ucrt64 -use-full-path -c "cd .../sdl2w/src && make ftime-report"
FTIME_REPORT ?= 0
ifeq ($(FTIME_REPORT),1)
  FLAGS += -ftime-report
endif

.PHONY: all native wasm dist tools clean clean-deps ftime-report bmin $(DIRS_TO_CREATE)

$(BMIN_REPO)/.git:
	@mkdir -p $(dir $(BMIN_REPO))
	@if [ ! -d "$(BMIN_REPO)/.git" ]; then \
		echo "Cloning bmin from $(BMIN_URL) ($(BMIN_REF))..."; \
		git clone --depth 1 --branch $(BMIN_REF) $(BMIN_URL) $(BMIN_REPO); \
	fi

$(BMIN_BUILT_LIB): $(BMIN_REPO)/.git
	@echo "Building bmin for $(TARGET)..."
	@$(MAKE) -C $(BMIN_SRC) all TARGET=$(TARGET)

$(DEPS_BMIN_INCLUDE_MARKER): $(BMIN_BUILT_LIB) $(wildcard $(BMIN_BUILT_INCLUDE)/* $(BMIN_BUILT_INCLUDE)/internal/*)
	@mkdir -p $(DEPS_DIR)/bmin $(DEPS_DIR)/lib
	@cp -R $(BMIN_BUILT_INCLUDE)/* $(DEPS_DIR)/bmin/
	@cp $(BMIN_BUILT_LIB) $(DEPS_DIR)/lib/

bmin: $(DEPS_BMIN_INCLUDE_MARKER)

$(OBJECTS): | $(DEPS_BMIN_INCLUDE_MARKER)

native:
	@$(MAKE) all TARGET=native

wasm:
	@$(MAKE) all TARGET=wasm

files:
	@echo $(CODE)

ftime-report:
	@bash ftime-report.sh

all: dist

$(DIRS_TO_CREATE):
	@mkdir -p $@

$(LIB_SDL2W): $(OBJECTS) | $(LIB_OUTPUT_DIR)
	$(AR) rcs $@ $(OBJECTS)

dist: $(LIB_SDL2W) $(DEPS_BMIN_INCLUDE_MARKER)
	@echo "Creating library and headers for $(TARGET) at $(INSTALL_DIR)"
	@mkdir -p $(INSTALL_DIR)/lib
	@mkdir -p $(INSTALL_DIR)/include
	@mkdir -p $(INSTALL_DIR)/include/bmin
	@cp $(LIB_SDL2W) $(INSTALL_DIR)/lib
	@cp $(DEPS_BMIN_LIB) $(INSTALL_DIR)/lib/libbmin.a
	@echo "Copying header files from $(HEADER_SRC_DIR) to $(INSTALL_DIR)/include"
	cp -f $(HEADER_SRC_DIR)/*.h $(INSTALL_DIR)/include
	@cp -R $(DEPS_DIR)/bmin/* $(INSTALL_DIR)/include/bmin/
	@echo "Created $(TARGET) sdl2w folder at top level directory."

tools: $(LIB_SDL2W) L10nScanner Anims
	mv Anims* build/tools/
	mv L10nScanner* build/tools/

Anims: tools/Anims.cpp $(ALL_CLIENT_OBJECTS)
	$(CXX) $(FLAGS) $(INCLUDES) $(OBJECTS) $< $(filter %.o,$^) -o $@ $(LIBS) 

L10nScanner: tools/L10nScanner.cpp | $(DEPS_BMIN_INCLUDE_MARKER)
	$(CXX) $(FLAGS) $(INCLUDES) $< -o $@ $(LIBS)

-include $(DEPENDS)

$(OBJ_OUTPUT_DIR)/%.o: %.cpp | $(DIRS_TO_CREATE)
	@mkdir -p $(@D)
	$(CXX) $(FLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

clean:
	@echo "Cleaning for all targets..."
	rm -rf $(BASE_BUILD_DIR)
	rm -rf $(DEPS_DIR)
	rm -f libsdl2w.a Anims Anims.exe L10nScanner L10nScanner.exe
	rm -rf ../sdl2w
	rm -rf main.d
	@if [ -f "$(BMIN_SRC)/Makefile" ]; then $(MAKE) -C $(BMIN_SRC) clean; fi

clean-deps: clean
	@echo "Removing bmin clone at $(BMIN_REPO)..."
	rm -rf $(BMIN_REPO)
