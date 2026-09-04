module;
#include <string_view>

#include "impl_headers.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "macros.h"

module sdl2w.emscripten;
import sdl2w.logger;
import bmin.string;
import bmin.stringstream;
import bmin.string_interop;

namespace emshelpers {

void* emscriptenWindow = nullptr;

void setEmscriptenWindow(void* window) {
  emscriptenWindow = window;
  sdl2w::log(sdl2w::DEBUG) << "[sdl2w] Set Emscripten window: "
                           << emscriptenWindow << sdl2w::endl;
}

void notifyTargetWindowSize(int width, int height) {
#ifdef __EMSCRIPTEN__
  bmin::StringStream ss;
  ss << "window.Lib.notifyTargetWindowSize(" << width << ", " << height << ")";
  emscripten_run_script(ss.str().cStr());
#else
  (void)width;
  (void)height;
#endif
}

bool isEmscriptenEnv() {
#ifdef __EMSCRIPTEN__
  return true;
#else
  return false;
#endif
}

void notifyGameStarted() {
#ifdef __EMSCRIPTEN__
  emscripten_run_script("window.Lib.notifyGameStarted()");
#endif
}
void notifyGameReady() {
#ifdef __EMSCRIPTEN__
  emscripten_run_script("window.Lib.notifyGameReady()");
#endif
}
void notifyGameCompleted(std::string_view result) {
#ifdef __EMSCRIPTEN__
  bmin::String script = bmin::String("window.Lib.notifyGameCompleted('") +
                         bmin::String(result.data(), result.size()) + "')";
  emscripten_run_script(script.cStr());
#endif
}
void notifyGameGeneric(std::string_view payload) {
#ifdef __EMSCRIPTEN__
  bmin::String script = bmin::String("window.Lib.notifyGameGeneric('") +
                         bmin::String(payload.data(), payload.size()) + "')";
  emscripten_run_script(script.cStr());
#endif
}

}
