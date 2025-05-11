#pragma once

#include "Window.h"
#include <string_view>

namespace sdl2w {

constexpr std::string_view SPLASH_FONT_NAME = "monofonto";

void setupStartupArgs(int argc, char* argv[], sdl2w::Window& window);
void renderSplash(sdl2w::Window& window);

} // namespace sdl2w