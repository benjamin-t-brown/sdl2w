EXE=SDL2W_EXAMPLE

MAIN_ALL=main.cpp
MAIN_ALL_OBJECT=$(MAIN_ALL:.cpp=.o)

FLAGS=-O1 -Wall -std=c++17 -I.
LINK_FLAGS=

EMCC_LIBS_CLIENT=\
-Oz\
-flto\
-s USE_SDL=2\
-s USE_SDL_IMAGE=2\
-s USE_SDL_MIXER=2\
-s SDL2_IMAGE_FORMATS='["png"]'\
-s USE_SDL_TTF=2\
-s ALLOW_MEMORY_GROWTH=1\
-s SAFE_HEAP=0\
-s ASSERTIONS=1\
-s INITIAL_MEMORY=326565888\
-s ENVIRONMENT=web\
-s JS_MATH=0\
-lidbfs.js\
-sNO_DISABLE_EXCEPTION_CATCHING\
-L../sdl2w/lib -lsdl2w

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

.PHONY: $(EXE) all
$(EXE): $(MAIN_ALL_OBJECT)
	$(CXX) $(FLAGS) $(LINK_FLAGS) $(MAIN_ALL_OBJECT) -o $(EXE) -L../sdl2w/lib -lsdl2w $(LIBS)

all: $(EXE)

lib:
	cd ../src && make -j8 all

clean_lib:
	cd ../src && make clean
	rm -rf ../sdl2w

%.o: %.cpp
	$(CXX) $(FLAGS) -MMD -MP -c $< -o $@

run:
	./$(EXE)

temp:
	mkdir -p .build

js: temp
	rm -rf dist
	em++ -I. -std=c++17 $(MAIN_ALL) $(EMCC_LIBS_CLIENT) $(EMCC_EXPORTED) --preload-file assets -o .build/$(EXE).js
	mkdir -p dist
	mv -v .build/* ../web
	rm -fd .build

files:
	@echo $(MAIN_ALL)

clean:
	rm -f $(EXE) $(EXE).exe
	rm -fd .build
	rm -rf main.o
	rm -rf main.d
