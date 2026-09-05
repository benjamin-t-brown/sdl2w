#include "EmscriptenHelpers.h"
#include "Logger.h"
#include "Window.h"
#include "bmin/StringStream.h"

#ifdef __EMSCRIPTEN__
#if __has_include(<SDL_mixer.h>)
#include <SDL_mixer.h>
#else
#include <SDL2/SDL_mixer.h>
#endif
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace emshelpers {

sdl2w::Window* emscriptenWindow = nullptr;

void setEmscriptenWindow(void* window) {
  emscriptenWindow = static_cast<sdl2w::Window*>(window);
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "[sdl2w] Set Emscripten window: " << emscriptenWindow
      << sdl2w::Logger::endl;
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

#ifdef __EMSCRIPTEN__
extern "C" {

EMSCRIPTEN_KEEPALIVE
void setVolume(int volumePct) {
  if (emshelpers::emscriptenWindow == nullptr)
    return;
  emshelpers::emscriptenWindow->setSoundPct(double(volumePct));
  emshelpers::emscriptenWindow->setMusicPct(double(volumePct));
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "Set master volume:" << volumePct << "%" << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void enableSound() {
  if (emshelpers::emscriptenWindow == nullptr)
    return;
  sdl2w::Window::_soundEnabled = true;
  int volumePct = emshelpers::emscriptenWindow->getSoundPct();
  int musicPct = emshelpers::emscriptenWindow->getMusicPct();
  Mix_Volume(-1, (double(volumePct) / 100.0) * double(MIX_MAX_VOLUME));
  Mix_VolumeMusic((double(musicPct) / 100.0) * double(MIX_MAX_VOLUME));
  sdl2w::Logger().get(sdl2w::DEBUG) << "Enable sound" << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void disableSound() {
  sdl2w::Window::_soundEnabled = false;

  Mix_VolumeMusic(0);
  Mix_Volume(-1, 0);
  sdl2w::Logger().get(sdl2w::DEBUG) << "Disable sound" << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void setKeyDown(int key) {
  if (emshelpers::emscriptenWindow == nullptr)
    return;
  emshelpers::emscriptenWindow->getEvents().keydown(key);
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "External set key down: " << key << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void setKeyUp(int key) {
  if (emshelpers::emscriptenWindow == nullptr)
    return;
  emshelpers::emscriptenWindow->getEvents().keyup(key);
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "External set key up: " << key << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void setKeyStatus(int status) {
  sdl2w::Window::_inputEnabled = !!status;
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "External set key status: " << sdl2w::Window::_inputEnabled
      << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void sendEvent(int event, int payload) {
  if (emshelpers::emscriptenWindow == nullptr)
    return;
  emshelpers::emscriptenWindow->pushExternalEvent(
      event, bmin::toString(payload).sliceView());
  sdl2w::Logger().get(sdl2w::DEBUG) << "External event received: " << event
                                    << ":" << payload << sdl2w::Logger::endl;
}
}
#endif
