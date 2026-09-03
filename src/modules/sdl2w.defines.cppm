module;
#include <string_view>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#endif

export module sdl2w.defines;

export namespace sdl2w {
const std::string_view FAIL_ERROR_TEXT{"sdl2w fail"};
const std::string_view INDEXDB_PREFIX{"sdl2wdata"};
const std::string_view ASSETS_PREFIX{""};

enum TextSize {
  TEXT_SIZE_10 = 10,
  TEXT_SIZE_12 = 12,
  TEXT_SIZE_14 = 14,
  TEXT_SIZE_15 = 15,
  TEXT_SIZE_16 = 16,
  TEXT_SIZE_18 = 18,
  TEXT_SIZE_20 = 20,
  TEXT_SIZE_22 = 22,
  TEXT_SIZE_24 = 24,
  TEXT_SIZE_28 = 28,
  TEXT_SIZE_32 = 32,
  TEXT_SIZE_36 = 36,
  TEXT_SIZE_48 = 48,
  TEXT_SIZE_60 = 60,
  TEXT_SIZE_72 = 72
};

struct SDL_Deleter {
  void operator()(SDL_Window* p) const;
  void operator()(SDL_Renderer* p) const;
  void operator()(SDL_Texture* p) const;
  void operator()(SDL_Surface* p) const;
  void operator()(Mix_Chunk* p) const;
  void operator()(TTF_Font* p) const;
  void operator()(Mix_Music* p) const;
  void operator()(SDL_Joystick* p) const;
};
}

namespace sdl2w {

void SDL_Deleter::operator()(SDL_Window* p) const {
  if (p != nullptr) {
    SDL_DestroyWindow(p);
  }
}
void SDL_Deleter::operator()(SDL_Renderer* p) const {
  if (p != nullptr) {
    SDL_DestroyRenderer(p);
  }
}
void SDL_Deleter::operator()(SDL_Texture* p) const {
  if (p != nullptr) {
    SDL_DestroyTexture(p);
  }
}
void SDL_Deleter::operator()(SDL_Surface* p) const {
  if (p != nullptr) {
    SDL_FreeSurface(p);
  }
}
void SDL_Deleter::operator()(TTF_Font* p) const {
  if (p != nullptr) {
    TTF_CloseFont(p);
  }
}
void SDL_Deleter::operator()(Mix_Chunk* p) const {
  if (p != nullptr) {
    Mix_FreeChunk(p);
  }
}
void SDL_Deleter::operator()(Mix_Music* p) const {
  if (p != nullptr) {
    Mix_FreeMusic(p);
  }
}
void SDL_Deleter::operator()(SDL_Joystick* p) const {
  (void)p;
}

}
