module;
#include <string_view>

#include "impl_headers.h"
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_surface.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#endif

#include "macros.h"

export module sdl2w.store;
export import sdl2w.animation;
export import sdl2w.defines;
export import sdl2w.types;
export import bmin.containers;
import sdl2w.logger;
import bmin.string_interop;

export namespace sdl2w {

struct StoredSound {
  bmin::UniquePtr<Mix_Chunk, SDL_Deleter> chunk;
  float volume = 1.0f;
};

class Store {
public:
  bmin::Map<bmin::String, bmin::UniquePtr<SDL_Texture, SDL_Deleter>> textures;
  bmin::Map<bmin::String, bmin::UniquePtr<SDL_Texture, SDL_Deleter>>
      dynamicTextures;
  bmin::Map<bmin::String, bmin::UniquePtr<Sprite>> sprites;
  bmin::Map<bmin::String, bmin::UniquePtr<AnimationDefinition>> anims;
  bmin::Map<bmin::String, bmin::UniquePtr<TTF_Font, SDL_Deleter>> fonts;
  bmin::Map<bmin::String, StoredSound> sounds;
  bmin::Map<bmin::String, bmin::UniquePtr<Mix_Music, SDL_Deleter>> musics;

  bmin::Map<bmin::String, bmin::String> fontAliases;
  AnimationDefinition defaultAnimDef = AnimationDefinition("default", false);

  Store() {}

  void storeTexture(std::string_view name, SDL_Texture* tex);
  void storeDynamicTexture(std::string_view name, SDL_Texture* tex);
  void storeSprite(std::string_view name, Sprite* sprite);
  AnimationDefinition& storeAnimationDefinition(std::string_view name,
                                                const bool loop);
  void loadAndStoreFont(std::string_view name, std::string_view path);
  void createFontAlias(std::string_view aliasName,
                       std::string_view loadedFontName);
  void storeSound(std::string_view name,
                  std::string_view path,
                  float volume = 1.0f);
  void storeMusic(std::string_view name, std::string_view path);

  SDL_Texture* getTexture(std::string_view name);
  SDL_Texture* getDynamicTexture(std::string_view name);
  SDL_Texture* getTextTexture(std::string_view name);
  Sprite& getSprite(std::string_view name);
  AnimationDefinition& getAnimationDefinition(std::string_view name);
  TTF_Font*
  getFont(std::string_view name, const int sz, const bool isOutline = false);
  Mix_Chunk* getSound(std::string_view name);
  float getSoundVolume(std::string_view name);
  Mix_Music* getMusic(std::string_view name);
  Animation createAnimation(std::string_view name, bool flipped = false);

  bool hasDynamicTexture(std::string_view name);

  void logAllSprites();
  void logAllAnimationDefinitions();

  void clear();
};

}
