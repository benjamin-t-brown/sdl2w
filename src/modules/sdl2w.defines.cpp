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

module sdl2w.defines;

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
