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
lib/Revirtualis.cpp\
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

CLIENT_OBJECTS=$(CLIENT_CODE:.cpp=.o)
SHARED_OBJECTS=$(SHARED_CODE:.cpp=.o)
TEST_OBJECTS=$(TEST_CODE:.cpp=.o)
TEST_UI_OBJECTS=$(TEST_CODE_UI:.cpp=.o)

CLIENT_DEPENDS := $(patsubst %.cpp,%.d,$(CLIENT_CODE))
SHARED_DEPENDS := $(patsubst %.cpp,%.d,$(SHARED_CODE))
TEST_DEPENDS := $(patsubst %.cpp,%.d,$(TEST_CODE))
TEST_UI_DEPENDS := $(patsubst %.cpp,%.d,$(TEST_CODE_UI))

ALL_CLIENT_OBJECTS=$(CLIENT_OBJECTS) $(SHARED_OBJECTS)

ALL_CLIENT_DEPENDS=$(CLIENT_DEPENDS) $(SHARED_DEPENDS)

# Define the name of the static library
LIB_SDL2W = libsdl2w.a

# Installation directories
INSTALL_DIR = ../sdl2w
INSTALL_LIB_DIR = $(INSTALL_DIR)/lib
INSTALL_INCLUDE_DIR = $(INSTALL_DIR)/include

# Header source directories (relative to this Makefile in src/)
HEADER_SRC_DIRS = lib/sdl2w

FLAGS=-O1 -Wall -std=c++17 -I. -DSDL2W_ENABLE_L10N
LINK_FLAGS=

DEBUG=true

ifeq ($(DEBUG),true)
	FLAGS+=-g
endif

ifeq ($(OS),Windows_NT)
	LIBS=-mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
# If you get an 'lld error', remove the -fuse-ld=lld flag from the LINK_FLAGS variable
# NOTE it will likely link much slower on Windows.
# LINK_FLAGS=-fuse-ld=lld 
else
	UNAME_S=$(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
    LIBS=-L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
		FLAGS+= -I/opt/homebrew/include -I/usr/local/include
	else
		LIBS=-lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
    endif
endif

.PHONY: $(LIB_SDL2W) all tools/Anims.o tools install

$(LIB_SDL2W): $(ALL_CLIENT_OBJECTS)
	ar rcs $@ $(ALL_CLIENT_OBJECTS)

all: tools dist

dist: $(LIB_SDL2W)
	@echo "Creating library and headers at $(INSTALL_DIR)"
	@mkdir -p $(INSTALL_LIB_DIR)
	@mkdir -p $(INSTALL_INCLUDE_DIR)
	@cp $(LIB_SDL2W) $(INSTALL_LIB_DIR)/
	@echo "Copying header files from $(HEADER_SRC_DIRS) to $(INSTALL_INCLUDE_DIR)/"
	cp -f lib/*.h $(INSTALL_INCLUDE_DIR)/;
	cp -f Anims* $(INSTALL_DIR)
	cp -f L10nScanner $(INSTALL_DIR)/
	@echo "Created sdl2w folder at top level directory."

tools: $(ALL_CLIENT_OBJECTS)
	$(CXX) $(FLAGS) $(LINK_FLAGS) tools/Anims.cpp $(ALL_CLIENT_OBJECTS) -o Anims $(LIBS)
	$(CXX) $(FLAGS) $(LINK_FLAGS) tools/L10nScanner.cpp -o L10nScanner

-include $(ALL_CLIENT_DEPENDS)

%.o: %.cpp
	$(CXX) $(FLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(CLIENT_OBJECTS) $(SHARED_OBJECTS)
	rm -f $(CLIENT_DEPENDS) $(SHARED_DEPENDS)
	rm -f precompiled.h.gch
	rm -f $(LIB_SDL2W)
	rm -f Anims*
	rm -rf main.d
	rm -rf $(INSTALL_DIR)
