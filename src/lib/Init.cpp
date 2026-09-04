#include "Init.h"
#include "L10n.h"
#include "Logger.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace sdl2w {

void setLanguage(int argc, char* argv[]) {
  bmin::String langArg = "en";
  for (int i = 1; i < argc; i++) {
    bmin::String arg = argv[i];
    if (arg == "--language" && (i + 1) < argc) {
      langArg = argv[++i];
    }
  }
  sdl2w::L10n::setLanguage(langArg.sliceView());
}

void setupStartupArgs(int argc, char* argv[], sdl2w::Window& window) {
  window.getStore().loadAndStoreFont(
      SPLASH_FONT_NAME,
      (bmin::String("assets/") +
       bmin::String(SPLASH_FONT_NAME.data(), SPLASH_FONT_NAME.size()) + ".ttf")
          .sliceView());
#ifndef __EMSCRIPTEN__
  sdl2w::Logger::setLogToFile(true);
#endif
  setLanguage(argc, argv);
}

void renderSplash(sdl2w::Window& window) {
  bmin::DynArray<bmin::String> lines;
  lines.pushBack("                                   _(_)_ ");
  lines.pushBack("                                  (_)@(_)");
  lines.pushBack("                                    (_)  ");
  lines.pushBack("_                                    |   ");
  lines.pushBack("|_)  _     o ._ _|_      _. | o _  (\\|/) ");
  lines.pushBack("| \\ (/_ \\/ | |   |_ |_| (_| | | _>  \\|/  ");
  lines.pushBack("^^^ ^^^^^^ ^^ ^^^^ ^^^  ^^^^^^ ^^^ ^^^^^^ ");

  auto& d = window.getDraw();
  auto [windowWidth, windowHeight] = d.getRenderSize();
  const int x = (windowWidth / 2.f);
  const int y = (windowHeight / 2.f);
  d.setBackgroundColor({16, 30, 41});
  for (size_t i = 0; i < lines.size(); i++) {
    d.drawText(lines[i].sliceView(),
               {
                   .fontName =
                       bmin::String(SPLASH_FONT_NAME.data(),
                                    SPLASH_FONT_NAME.size()),
                   .fontSize = sdl2w::TextSize::TEXT_SIZE_16,
                   .x = x,
                   .y = y + static_cast<int>(i) * 20 - 100,
                   .color = {244, 126, 27},
                   .centered = true,
               });
  }
  d.drawText("Have fun!",
             {
                 .fontName =
                     bmin::String(SPLASH_FONT_NAME.data(),
                                  SPLASH_FONT_NAME.size()),
                 .fontSize = sdl2w::TextSize::TEXT_SIZE_24,
                 .x = x,
                 .y = y + 50,
                 .color = {255, 255, 255},
                 .centered = true,
             });
}

void renderRevirtualisSplash(sdl2w::Window& window) {
  window.getDraw().setBackgroundColor({16, 30, 41});
  renderSplash(window);
}

} // namespace sdl2w
