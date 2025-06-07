#pragma once

#include <string>
namespace sdl2w {
class Window;
}

namespace emshelpers {
void setEmscriptenWindow(sdl2w::Window* window);
void notifyGameStarted();
void notifyGameReady();
void notifyGameCompleted(const std::string& result);
} // namespace emshelpers
