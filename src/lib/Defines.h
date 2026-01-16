#pragma once

#include <string_view>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;
struct Mix_Chunk;
#ifdef __APPLE__
typedef struct TTF_Font TTF_Font;
typedef struct Mix_Music Mix_Music;
#else
typedef struct TTF_Font TTF_Font;
typedef struct Mix_Music Mix_Music;
#endif
typedef struct _SDL_Joystick SDL_Joystick;

namespace sdl2w {
const std::string_view SPRITE_FLIPPED{"_f"};
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
} // namespace sdl2w
