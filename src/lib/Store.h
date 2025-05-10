// A Store owns the pointers to various SDL and SDL2W resources, handling
// retrieval and automatic clean up with RAII.

#pragma once

#include "Animation.h"
#include "Defines.h"
#include <memory>
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

  Store() {}

  void storeTexture(const std::string& name, SDL_Texture* tex);
  void storeDynamicTexture(const std::string& name, SDL_Texture* tex);
  void storeSurface(const std::string& name, SDL_Surface* surf);
  void storeDynamicSurface(const std::string& name, SDL_Surface* surf);
  void storeSprite(const std::string& name, Sprite* sprite);
  AnimationDefinition& storeAnimationDefinition(const std::string& name,
                                                const bool loop);
  void loadAndStoreFont(const std::string& name, const std::string& path);
  void createFontAlias(const std::string& aliasName,
                       const std::string& loadedFontName);
  void storeSound(const std::string& name, const std::string& path);
  void storeMusic(const std::string& name, const std::string& path);

  SDL_Texture* getTexture(const std::string& name);
  SDL_Texture* getDynamicTexture(const std::string& name);
  SDL_Texture* getTextTexture(const std::string& name);
  SDL_Surface* getSurface(const std::string& name);
  SDL_Surface* getDynamicSurface(const std::string& name);
  Sprite& getSprite(const std::string& name);
  AnimationDefinition& getAnimationDefinition(const std::string& name);
  TTF_Font*
  getFont(const std::string& name, const int sz, const bool isOutline = false);
  Mix_Chunk* getSound(const std::string& name);
  Mix_Music* getMusic(const std::string& name);
  Animation createAnimation(const std::string& name, bool flipped = false);

  bool hasDynamicTextureOrSurface(const std::string& name);

  void logAllSprites();
  void logAllAnimationDefinitions();

  void clear();
};

} // namespace sdl2w