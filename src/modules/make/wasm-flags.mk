# Emscripten compile/link flags shared by use.mk and build-bmi-em.mk.

EMCC_SDL_PORTS ?= \
	-sUSE_SDL=2 \
	-sUSE_SDL_IMAGE=2 \
	-sUSE_SDL_MIXER=2 \
	-sSDL2_IMAGE_FORMATS='["png"]' \
	-sUSE_SDL_TTF=2 \
	-sUSE_SDL_GFX=2

EMCC_LIBS ?= \
	$(EMCC_SDL_PORTS) \
	-sALLOW_MEMORY_GROWTH=1 \
	-sSAFE_HEAP=0 \
	-sASSERTIONS=1 \
	-sINITIAL_MEMORY=326565888 \
	-sENVIRONMENT=web \
	-sDISABLE_EXCEPTION_CATCHING=0 \
	-lidbfs.js

EMCC_EXPORTED ?= \
	-sEXPORTED_FUNCTIONS='["_main","_enableSound","_disableSound","_setVolume","_setKeyDown","_setKeyUp","_setKeyStatus","_sendEvent"]' \
	-sEXPORTED_RUNTIME_METHODS='["ccall"]'

EMCC_CXXFLAGS ?= -Wall -std=c++23 -Oz $(EMCC_SDL_PORTS)
