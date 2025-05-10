#pragma once

#include "Window.h"
#include <string_view>

namespace revirtualis {

constexpr std::string_view REVIRTUALIS_FONT_NAME = "monofonto";

void setupRevirtualis(int argc, char* argv[], sdl2w::Window& window);
void showRevirtualisSplash(sdl2w::Window& window, int duration = 1000);

} // namespace revirtualis