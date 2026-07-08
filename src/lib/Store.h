// A Store owns the pointers to various SDL and SDL2W resources, handling
// retrieval and automatic clean up with RAII.

#pragma once

#include "Animation.h"
#include "Defines.h"
#include "bmin/Map.h"
#include "bmin/String.h"
#include "bmin/UniquePtr.h"
#include <string_view>

namespace sdl2w {

class Store {
public:
  bmin::Map<bmin::String, bmin::UniquePtr<SDL_Texture, SDL_Deleter>> textures;
  bmin::Map<bmin::String, bmin::UniquePtr<SDL_Texture, SDL_Deleter>>
      dynamicTextures;
  bmin::Map<bmin::String, bmin::UniquePtr<Sprite>> sprites;
  bmin::Map<bmin::String, bmin::UniquePtr<AnimationDefinition>> anims;
  bmin::Map<bmin::String, bmin::UniquePtr<TTF_Font, SDL_Deleter>> fonts;
  bmin::Map<bmin::String, bmin::UniquePtr<Mix_Chunk, SDL_Deleter>> sounds;
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
  void storeSound(std::string_view name, std::string_view path);
  void storeMusic(std::string_view name, std::string_view path);

  SDL_Texture* getTexture(std::string_view name);
  SDL_Texture* getDynamicTexture(std::string_view name);
  SDL_Texture* getTextTexture(std::string_view name);
  Sprite& getSprite(std::string_view name);
  AnimationDefinition& getAnimationDefinition(std::string_view name);
  TTF_Font*
  getFont(std::string_view name, const int sz, const bool isOutline = false);
  Mix_Chunk* getSound(std::string_view name);
  Mix_Music* getMusic(std::string_view name);
  Animation createAnimation(std::string_view name, bool flipped = false);

  bool hasDynamicTexture(std::string_view name);

  void logAllSprites();
  void logAllAnimationDefinitions();

  void clear();
};

} // namespace sdl2w
