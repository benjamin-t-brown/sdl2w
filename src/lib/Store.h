// A Store owns the pointers to various SDL and SDL2W resources, handling
// retrieval and automatic clean up with RAII.

#pragma once

#include "Animation.h"
#include "Defines.h"
#include "bmin/Map.h"
#include "bmin/String.h"
#include "bmin/UniquePtr.h"
#include <initializer_list>
#include <string_view>

namespace sdl2w {

struct StoredSound {
  bmin::UniquePtr<Mix_Chunk, SDL_Deleter> chunk;
  // Per-sound volume multiplier in [0, 1], applied when the sound is played.
  float volume = 1.0f;
};

class Store {
  bmin::Map<bmin::String, bmin::String> fontPaths;
  bmin::Map<bmin::String, bmin::UniquePtr<SDL_Texture, SDL_Deleter>> textures;
  bmin::Map<bmin::String, bmin::UniquePtr<SDL_Texture, SDL_Deleter>>
      dynamicTextures;
  bmin::Map<bmin::String, bmin::UniquePtr<Sprite>> sprites;
  bmin::Map<bmin::String, bmin::UniquePtr<AnimationDefinition>> anims;
  bmin::Map<bmin::String, bmin::UniquePtr<TTF_Font, SDL_Deleter>> fonts;
  bmin::Map<bmin::String, StoredSound> sounds;
  bmin::Map<bmin::String, bmin::UniquePtr<Mix_Music, SDL_Deleter>> musics;
  bmin::Map<bmin::String, bmin::String> fontAliases;

public:
  using TexturePtr = bmin::UniquePtr<SDL_Texture, SDL_Deleter>;

  Store() = default;
  Store(const Store&) = delete;
  Store& operator=(const Store&) = delete;
  Store(Store&&) = delete;
  Store& operator=(Store&&) = delete;

  void storeTexture(std::string_view name, TexturePtr tex);
  void storeTexture(std::string_view name, SDL_Texture* tex);
  void storeDynamicTexture(std::string_view name, TexturePtr tex);
  void storeDynamicTexture(std::string_view name, SDL_Texture* tex);
  void storeSprite(std::string_view name, Sprite sprite);
  void storeSprite(std::string_view name, Sprite* sprite);
  AnimationDefinition& storeAnimationDefinition(std::string_view name,
                                                const bool loop);
  void loadAndStoreFont(std::string_view name, std::string_view path);
  void preloadFontSizes(std::string_view name,
                        std::initializer_list<TextSize> sizes,
                        bool includeOutlines = false);
  void createFontAlias(std::string_view aliasName,
                       std::string_view loadedFontName);
  void
  storeSound(std::string_view name, std::string_view path, float volume = 1.0f);
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

  bool hasTexture(std::string_view name) const;
  bool hasDynamicTexture(std::string_view name) const;
  bool hasSprite(std::string_view name) const;
  bool hasAnimationDefinition(std::string_view name) const;
  bool hasFont(std::string_view name) const;
  bool hasFont(std::string_view name, int size, bool isOutline = false) const;
  bool hasSound(std::string_view name) const;
  bool hasMusic(std::string_view name) const;

  void logAllSprites();
  void logAllAnimationDefinitions();

  void clear();
};

} // namespace sdl2w
