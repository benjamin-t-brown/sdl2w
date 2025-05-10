#include "EmscriptenHelpers.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <string>
#endif

namespace emshelpers {

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
void notifyGameCompleted(bool isVictory) {
#ifdef __EMSCRIPTEN__
  const std::string script = std::string("window.Lib.notifyGameCompleted(" +
                                         std::to_string(isVictory) + ")");
  emscripten_run_script(script.c_str());
#endif
}
} // namespace emshelpers

#ifdef __EMSCRIPTEN__
extern "C" {
EMSCRIPTEN_KEEPALIVE
void enableSound() {
  sdl2w::Window::soundEnabled = true;
  int volumePct = sdl2w::Window::soundPercent;
  Mix_VolumeMusic((double(volumePct) / 100.0) * double(MIX_MAX_VOLUME));
  Mix_Volume(-1, (double(volumePct) / 100.0) * double(MIX_MAX_VOLUME));
  sdl2w::Logger().get(sdl2w::DEBUG) << "Enable sound" << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void disableSound() {
  sdl2w::Window::soundEnabled = false;

  Mix_VolumeMusic(0);
  Mix_Volume(-1, 0);
  sdl2w::Logger().get(sdl2w::DEBUG) << "Disable sound" << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void setVolume(int volumePct) {
  sdl2w::Window& window = sdl2w::Window::getGlobalWindow();
  window.setVolume(double(volumePct));
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "Set volume:" << volumePct << "%" << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void setKeyDown(int key) {
  sdl2w::Window& window = sdl2w::Window::getGlobalWindow();
  sdl2w::Events& events = window.getEvents();
  events.keydown(key);
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "External set key down: " << key << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void setKeyUp(int key) {
  sdl2w::Window& window = sdl2w::Window::getGlobalWindow();
  sdl2w::Events& events = window.getEvents();
  events.keyup(key);
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "External set key up: " << key << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void setKeyStatus(int status) {
  sdl2w::Window& window = sdl2w::Window::getGlobalWindow();
  window.isInputEnabled = !!status;
  sdl2w::Logger().get(sdl2w::DEBUG)
      << "External set key status: " << window.isInputEnabled
      << sdl2w::Logger::endl;
}
EMSCRIPTEN_KEEPALIVE
void sendEvent(int event, int payload) {
  sdl2w::Window& window = sdl2w::Window::getGlobalWindow();
  window.externalEvents.emplace_back(event, payload);
  sdl2w::Logger().get(sdl2w::DEBUG) << "External event received: " << event
                                    << ":" << payload << sdl2w::Logger::endl;
}
}
#endif