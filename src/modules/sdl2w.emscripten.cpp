module;
#include <string_view>

#include "impl_headers.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

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
  EM_ASM(
      {
        if (window.Lib && window.Lib.notifyTargetWindowSize)
          window.Lib.notifyTargetWindowSize($0, $1);
      },
      width,
      height);
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
  EM_ASM({
    if (window.Lib && window.Lib.notifyGameStarted)
      window.Lib.notifyGameStarted();
  });
#endif
}
void notifyGameReady() {
#ifdef __EMSCRIPTEN__
  EM_ASM({
    if (window.Lib && window.Lib.notifyGameReady)
      window.Lib.notifyGameReady();
  });
#endif
}
void notifyGameCompleted(std::string_view result) {
#ifdef __EMSCRIPTEN__
  const bmin::String value(result.data(), result.size());
  EM_ASM(
      {
        if (window.Lib && window.Lib.notifyGameCompleted)
          window.Lib.notifyGameCompleted(UTF8ToString($0));
      },
      value.cStr());
#else
  (void)result;
#endif
}
void notifyGameGeneric(std::string_view payload) {
#ifdef __EMSCRIPTEN__
  const bmin::String value(payload.data(), payload.size());
  EM_ASM(
      {
        if (window.Lib && window.Lib.notifyGameGeneric)
          window.Lib.notifyGameGeneric(UTF8ToString($0));
      },
      value.cStr());
#else
  (void)payload;
#endif
}

} // namespace emshelpers
