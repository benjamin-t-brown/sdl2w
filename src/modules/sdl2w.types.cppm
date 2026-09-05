module;
#include <string_view>
#include <utility>

#if __has_include(<SDL2/SDL_pixels.h>) && __has_include(<SDL2/SDL_stdinc.h>)
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_stdinc.h>
#elif __has_include(<SDL_pixels.h>) && __has_include(<SDL_stdinc.h>)
#include <SDL_pixels.h>
#include <SDL_stdinc.h>
#else
#error                                                                         \
    "Could not find SDL pixel/stdinc headers in either SDL2/ or root include paths"
#endif

#include "sdl_fwd.h"

export module sdl2w.types;
export import sdl2w.defines;
export import bmin.string;

export namespace sdl2w {

struct RenderableParamsEx {
  std::pair<double, double> scale = {0., 0.};
  double angleDeg = 0.;
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  int clipX = 0;
  int clipY = 0;
  int clipW = 0;
  int clipH = 0;
  bool centered = true;
  bool flipped = false;
};

struct RenderableParams {
  std::pair<double, double> scale = {0., 0.};
  int x = 0;
  int y = 0;
  bool centered = true;
  bool flipped = false;
};

struct RenderTextParams {
  bmin::String fontName = "default";
  TextSize fontSize = TextSize::TEXT_SIZE_16;
  int x = 0;
  int y = 0;
  SDL_Color color = {0, 0, 0, 255};
  bool centered = false;
  double angleDeg = 0.;
  std::pair<double, double> scale = {1., 1.};
  bool outlined = false;
};

struct Renderable {
  SDL_Texture* tex = nullptr;
  SDL_Surface* surf = nullptr;
};

struct Sprite {
  bmin::String name;
  Renderable renderable;
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  int spritesheetWidth = 0;
  bool flipped = false;
};

enum DrawMode {
  CPU,
  GPU,
};

} // namespace sdl2w
