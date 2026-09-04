module;
#include <string_view>

#include "impl_headers.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif


export module sdl2w.emscripten;
import sdl2w.logger;
import bmin.string;
import bmin.stringstream;
import bmin.string_interop;

export namespace emshelpers {
void setEmscriptenWindow(void* window);
void notifyTargetWindowSize(int width, int height);
bool isEmscriptenEnv();
void notifyGameStarted();
void notifyGameReady();
void notifyGameCompleted(std::string_view result);
void notifyGameGeneric(std::string_view payload);
}
