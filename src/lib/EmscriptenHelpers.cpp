#include "EmscriptenHelpers.h"
#include "Logger.h"
#include "Window.h"

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL_mixer.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace emshelpers {

sdl2w::Window* emscriptenWindow = nullptr;

void setEmscriptenWindow(sdl2w::Window* window) {
  emscriptenWindow = window;
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "[sdl2w] Set Emscripten window: " << emscriptenWindow << sdl2w::Logger::endl;

#ifdef __EMSCRIPTEN__
  std::stringstream ss;
  auto [width, height] = window->getDraw().getRenderSize();
  ss << "window.Lib.notifyTargetWindowSize(" << width << ", "
     << height << ")";
  emscripten_run_script(ss.str().c_str());
#endif
}

void notifyGameStarted() {
#ifdef __EMSCRIPTEN__
  const std::string script = std::string("window.Lib.notifyGameStarted()");
  emscripten_run_script(script.c_str());
#endif
}
void notifyGameReady() {
#ifdef __EMSCRIPTEN__
  const std::string script = std::string("window.Lib.notifyGameReady()");
  emscripten_run_script(script.c_str());
#endif
}
void notifyGameCompleted(const std::string& result) {
#ifdef __EMSCRIPTEN__
  const std::string script = std::string("window.Lib.notifyGameCompleted('" +
                                         result + "')");
  emscripten_run_script(script.c_str());
#endif
}
void notifyGameGeneric(const std::string& payload)
{
 #ifdef __EMSCRIPTEN__
  const std::string script = std::string("window.Lib.notifyGameGeneric('" +
                                         payload + "')");
  emscripten_run_script(script.c_str());
#endif 
}
} // namespace emshelpers

#ifdef __EMSCRIPTEN__
extern "C" {
EMSCRIPTEN_KEEPALIVE
void enableSound() {
  sdl2w::Window::_soundEnabled = true;
  int volumePct = emshelpers::emscriptenWindow->getSoundPct();
  Mix_VolumeMusic((double(volumePct) / 100.0) * double(MIX_MAX_VOLUME));
  Mix_Volume(-1, (double(volumePct) / 100.0) * double(MIX_MAX_VOLUME));
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
void setVolume(int volumePct) {
  emshelpers::emscriptenWindow->setSoundPct(double(volumePct));
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "Set volume:" << volumePct << "%" << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void setKeyDown(int key) {
  emshelpers::emscriptenWindow->getEvents().keydown(key);
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "External set key down: " << key << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void setKeyUp(int key) {
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
  emshelpers::emscriptenWindow->pushExternalEvent(event);
  emshelpers::emscriptenWindow->pushExternalEvent(payload);
  sdl2w::Logger().get(sdl2w::DEBUG) << "External event received: " << event
                                    << ":" << payload << sdl2w::Logger::endl;
}
}
#endif