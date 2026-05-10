// A Store owns the pointers to various SDL and SDL2W resources, handling
// retrieval and automatic clean up with RAII.

#pragma once

#include "Animation.h"
#include "Defines.h"
#include <memory>
#include <string_view>
#include <unordered_map>

namespace sdl2w {

class Store {
public:
  std::unordered_map<std::string, std::unique_ptr<SDL_Texture, SDL_Deleter>>
      textures;
  std::unordered_map<std::string, std::unique_ptr<SDL_Texture, SDL_Deleter>>
      dynamicTextures;
  std::unordered_map<std::string, std::unique_ptr<SDL_Surface, SDL_Deleter>>
      surfaces;
  std::unordered_map<std::string, std::unique_ptr<SDL_Surface, SDL_Deleter>>
      dynamicSurfaces;
  std::unordered_map<std::string, std::unique_ptr<Sprite>> sprites;
  std::unordered_map<std::string, std::unique_ptr<AnimationDefinition>> anims;
  std::unordered_map<std::string, std::unique_ptr<TTF_Font, SDL_Deleter>> fonts;
  std::unordered_map<std::string, std::unique_ptr<Mix_Chunk, SDL_Deleter>>
      sounds;
  std::unordered_map<std::string, std::unique_ptr<Mix_Music, SDL_Deleter>>
      musics;

  std::unordered_map<std::string, std::string> fontAliases;
  AnimationDefinition defaultAnimDef = AnimationDefinition("default", false);

  Store() {}

  void storeTexture(std::string_view name, SDL_Texture* tex);
  void storeDynamicTexture(std::string_view name, SDL_Texture* tex);
  void storeSurface(std::string_view name, SDL_Surface* surf);
  void storeDynamicSurface(std::string_view name, SDL_Surface* surf);
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
  SDL_Surface* getSurface(std::string_view name);
  SDL_Surface* getDynamicSurface(std::string_view name);
  Sprite& getSprite(std::string_view name);
  AnimationDefinition& getAnimationDefinition(std::string_view name);
  TTF_Font*
  getFont(std::string_view name, const int sz, const bool isOutline = false);
  Mix_Chunk* getSound(std::string_view name);
  Mix_Music* getMusic(std::string_view name);
  Animation createAnimation(std::string_view name, bool flipped = false);

  bool hasDynamicTextureOrSurface(std::string_view name);

  void logAllSprites();
  void logAllAnimationDefinitions();

  void clear();
};

} // namespace sdl2w