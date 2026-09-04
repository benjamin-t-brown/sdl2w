module;
#include <string_view>

#include "impl_headers.h"
#include "macros.h"

export module sdl2w.init;
export import sdl2w.window;
import sdl2w.l10n;
import sdl2w.logger;
import bmin.containers;
import bmin.string_interop;

export namespace sdl2w {

constexpr std::string_view SPLASH_FONT_NAME = "monofonto";

void setupStartupArgs(int argc, char* argv[], sdl2w::Window& window);
void renderSplash(sdl2w::Window& window);

}
