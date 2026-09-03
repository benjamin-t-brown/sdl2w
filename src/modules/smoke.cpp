#include "macros.h"

import sdl2w;

int main() {
  static_assert(sizeof(sdl2w::TextSize) >= 1);

  bmin::String title("modules");
  sdl2w::Store store;
  (void)store;
  (void)title;

  sdl2w::Logger::setLogLevel(sdl2w::INFO);
  LOG(INFO) << "sdl2w modules smoke ok" << LOG_ENDL;
  return 0;
}
