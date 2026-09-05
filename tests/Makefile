EXE = sdl2w-tests-modules
HEADER_EXE = sdl2w-tests-headers

.PHONY: all modules headers run clean
.DEFAULT_GOAL := all
.NOTPARALLEL:

include ../sdl2w/modules/make/use.mk

SDL2W_HEADER_ROOT ?= $(abspath ../sdl2w)
SDL2W_HEADER_CXXFLAGS ?= -Wall -Wextra -std=c++23 -g -I$(SDL2W_HEADER_ROOT)/include
SDL2W_HEADER_LDLIBS = -L$(SDL2W_HEADER_ROOT)/lib -lsdl2w -lbmin

ifeq ($(OS),Windows_NT)
  SDL2W_HEADER_LDLIBS += -mconsole -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
else ifeq ($(SDL2W_HOST_OS),Darwin)
  SDL2W_HEADER_CXXFLAGS += -I/opt/homebrew/include -I/usr/local/include
  SDL2W_HEADER_LDLIBS += -L/opt/homebrew/lib -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
else
  SDL2W_HEADER_LDLIBS += -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
endif

all: modules headers
modules: $(EXE)
headers: $(HEADER_EXE)

test-modules.o: test_main.cpp | sdl2w-bmi
	$(CXX) $(SDL2W_CXXFLAGS) -DSDL2W_USE_MODULES=1 -c $< -o $@

$(EXE): test-modules.o | sdl2w-bmi
	$(CXX) $(SDL2W_CXXFLAGS) -o $@ $< $(SDL2W_LDLIBS)

test-headers.o: test_main.cpp
	$(CXX) $(SDL2W_HEADER_CXXFLAGS) -c $< -o $@

$(HEADER_EXE): test-headers.o
	$(CXX) $(SDL2W_HEADER_CXXFLAGS) -o $@ $< $(SDL2W_HEADER_LDLIBS)

run: all
	SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./$(HEADER_EXE)
	SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./$(EXE)

clean:
	rm -f test-modules.o test-headers.o $(EXE) $(HEADER_EXE) *.log
	rm -rf gcm.cache pcm.cache .bmin-bmi .sdl2w-bmi
