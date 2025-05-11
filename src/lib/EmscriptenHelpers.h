#pragma once

namespace sdl2w {
class Window;
}

namespace emshelpers {
void setEmscriptenWindow(sdl2w::Window* window);
void notifyGameStarted();
void notifyGameReady();
void notifyGameCompleted(bool isVictory);
} // namespace emshelpers
