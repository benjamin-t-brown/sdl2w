#pragma once

#include <string_view>
namespace sdl2w {
class Window;
}

namespace emshelpers {
void setEmscriptenWindow(sdl2w::Window* window);
bool isEmscriptenEnv();
void notifyGameStarted();
void notifyGameReady();
void notifyGameCompleted(std::string_view result);
void notifyGameGeneric(std::string_view payload);
} // namespace emshelpers
