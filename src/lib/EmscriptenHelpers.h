#pragma once

#include <string_view>
namespace emshelpers {
void setEmscriptenWindow(void* window);
void notifyTargetWindowSize(int width, int height);
bool isEmscriptenEnv();
void notifyGameStarted();
void notifyGameReady();
void notifyGameCompleted(std::string_view result);
void notifyGameGeneric(std::string_view payload);
} // namespace emshelpers
