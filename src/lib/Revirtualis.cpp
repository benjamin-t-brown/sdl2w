#include "revirtualis.h"
#include "L10n.h"
#include "Logger.h"
#include <string>
#include <vector>

namespace revirtualis {

void setLanguage(int argc, char* argv[]) {
  std::string langArg = "en";
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--language" && (i + 1) < argc) {
      langArg = argv[++i];
    }
  }
  sdl2w::L10n::setLanguage(langArg);
}

void setupRevirtualis(int argc, char* argv[], sdl2w::Window& window) {
  window.getStore().loadAndStoreFont(
      std::string(REVIRTUALIS_FONT_NAME),
      "assets/" + std::string(REVIRTUALIS_FONT_NAME) + ".ttf");
#ifndef __EMSCRIPTEN__
  sdl2w::Logger::setLogToFile(true);
#endif
  setLanguage(argc, argv);
}

void renderSplash(sdl2w::Window& window) {
  // asci "mini" font
  std::vector<std::string> lines = {
      "                                   _(_)_ ",
      "                                  (_)@(_)",
      "                                    (_)  ",
      "_                                    |   ",
      "|_)  _     o ._ _|_      _. | o _  (\\|/) ",
      "| \\ (/_ \\/ | |   |_ |_| (_| | | _>  \\|/  ",
      "^^^ ^^^^^^ ^^ ^^^^ ^^^  ^^^^^^ ^^^ ^^^^^^ ",
  };

  auto& d = window.getDraw();
  auto [windowWidth, windowHeight] = d.getRenderSize();
  const int x = (windowWidth / 2.f);
  const int y = (windowHeight / 2.f);
  d.setBackgroundColor({16, 30, 41});
  for (int i = 0; i < static_cast<int>(lines.size()); i++) {
    d.drawText(lines[i],
               {
                   .fontName = std::string(REVIRTUALIS_FONT_NAME),
                   .fontSize = sdl2w::TextSize::TEXT_SIZE_16,
                   .x = x,
                   .y = y + i * 20 - 100,
                   .color = {244, 126, 27},
                   .centered = true,
               });
  }
  d.drawText("Have fun!",
             {
                 .fontName = std::string(REVIRTUALIS_FONT_NAME),
                 .fontSize = sdl2w::TextSize::TEXT_SIZE_24,
                 .x = x,
                 .y = y + 50,
                 .color = {255, 255, 255},
                 .centered = true,
             });
}

void showRevirtualisSplash(sdl2w::Window& window, int duration) {
  window.getDraw().setBackgroundColor({16, 30, 41});
#ifdef __EMSCRIPTEN__
  renderLoadingScreen(window);
#else
  int t = 0;
  window.startRenderLoop([&]() {
    renderSplash(window);
    t += std::min(window.getDeltaTime(), 100);
    return !(t >= duration);
  });
#endif
}

} // namespace revirtualis