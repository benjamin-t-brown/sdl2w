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
BASE_BUILD_DIR = build

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

LIBS_PLUS=
DEBUG=true
ifeq ($(TARGET),wasm)
    INCLUDES= -I.
    FLAGS = -Wall -std=c++17 -Oz -flto
    LIBS_PLUS = -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 -s USE_SDL_GFX=2
    LIBS = $(EMCC_EXPORTED) -lidbfs.js
    CXX = em++
    AR = emar
else
    INCLUDES= -I.
    FLAGS= -Wall -std=c++17
    AR = ar
    ifeq ($(OS),Windows_NT)
        LIBS = -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
    else
        UNAME_S := $(shell uname -s)
        ifeq ($(UNAME_S),Darwin)
            LIBS = -L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
            INCLUDES += -I/opt/homebrew/include -I/usr/local/include
        else
            LIBS = -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
        endif
    endif
    ifeq ($(DEBUG),true)
        FLAGS += -g
    endif
    ifeq ($(TARGET),miyoo)
        AS=arm-linux-gnueabihf-as
        CC=arm-linux-gnueabihf-gcc
        CXX=arm-linux-gnueabihf-g++
        LD=arm-linux-gnueabihf-ld
    else
        CXX = g++
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

.PHONY: tools clean $(DIRS_TO_CREATE)

native:
	@$(MAKE) all TARGET=native

wasm:
	@$(MAKE) all TARGET=wasm

miyoo:
	@$(MAKE) all TARGET=miyoo

files:
	@echo $(CODE)

all: dist

$(DIRS_TO_CREATE):
	@mkdir -p $@

$(LIB_SDL2W): $(OBJECTS) | $(LIB_OUTPUT_DIR)
	$(AR) rcs $@ $(OBJECTS)

dist: $(LIB_SDL2W)
	@echo "Creating library and headers for $(TARGET) at $(INSTALL_DIR)"
	@mkdir -p $(INSTALL_DIR)/lib
	@mkdir -p $(INSTALL_DIR)/include
	@cp $(LIB_SDL2W) $(INSTALL_DIR)/lib
	@echo "Copying header files from $(HEADER_SRC_DIR) to $(INSTALL_DIR)/include"
	cp -f $(HEADER_SRC_DIR)/*.h $(INSTALL_DIR)/include
	@echo "Created $(TARGET) sdl2w folder at top level directory."

tools: $(LIB_SDL2W) L10nScanner Anims
	mv Anims* build/tools/
	mv L10nScanner* build/tools/

Anims: tools/Anims.cpp $(ALL_CLIENT_OBJECTS)
	$(CXX) $(FLAGS) $(INCLUDES) $(OBJECTS) $< $(filter %.o,$^) -o $@ $(LIBS_PLUS) $(LIBS) 

L10nScanner: tools/L10nScanner.cpp 
	$(CXX) $(FLAGS) $(INCLUDES) $< -o $@

-include $(DEPENDS)

$(OBJ_OUTPUT_DIR)/%.o: %.cpp | $(DIRS_TO_CREATE)
	@mkdir -p $(@D)
	$(CXX) $(FLAGS) $(INCLUDES) -MMD -MP -c $< -o $@ $(LIBS_PLUS)

clean:
	@echo "Cleaning for all targets..."
	rm -rf $(BASE_BUILD_DIR)
	rm -f libsdl2w.a Anims Anims.exe L10nScanner L10nScanner.exe
	rm -rf ../sdl2w
	rm -rf main.d
