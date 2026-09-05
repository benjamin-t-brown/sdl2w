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

module sdl2w.store;
import sdl2w.animation;
import sdl2w.defines;
import sdl2w.types;
import bmin.containers;
import sdl2w.logger;
import bmin.string_interop;

namespace sdl2w {

static bmin::String toKey(std::string_view sv) {
  return bmin::String(sv.data(), sv.size());
}

static bmin::String fontKey(std::string_view name, int size, bool outline) {
  return bmin::String(name.data(), name.size()) + ":" + bmin::toString(size) +
         (outline ? ":outline" : ":regular");
}

void Store::storeTexture(std::string_view name, TexturePtr tex) {
  if (!tex) {
    THROW_RUNTIME_ERROR("[sdl2w] Cannot store a null texture.");
  }
  const bmin::String nameStr = toKey(name);
  if (textures.contains(nameStr)) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Texture already exists: '") +
                        nameStr + "'.");
  }
  textures.insert(nameStr, std::move(tex));
}

void Store::storeTexture(std::string_view name, SDL_Texture* tex) {
  storeTexture(name, TexturePtr(tex));
}

void Store::storeDynamicTexture(std::string_view name, TexturePtr tex) {
  if (!tex) {
    THROW_RUNTIME_ERROR("[sdl2w] Cannot store a null dynamic texture.");
  }
  const bmin::String nameStr = toKey(name);
  if (dynamicTextures.contains(nameStr)) {
    THROW_RUNTIME_ERROR(
        bmin::String("[sdl2w] Dynamic texture already exists: '") + nameStr +
        "'.");
  }
  dynamicTextures.insert(nameStr, std::move(tex));
}

void Store::storeDynamicTexture(std::string_view name, SDL_Texture* tex) {
  storeDynamicTexture(name, TexturePtr(tex));
}

void Store::storeSprite(std::string_view name, Sprite sprite) {
  const bmin::String nameStr = toKey(name);
  if (sprites.contains(nameStr)) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Sprite already exists: '") +
                        nameStr + "'.");
  }
  sprites.insert(nameStr, bmin::makeUnique<Sprite>(std::move(sprite)));
}

void Store::storeSprite(std::string_view name, Sprite* sprite) {
  if (sprite == nullptr) {
    THROW_RUNTIME_ERROR("[sdl2w] Cannot store a null sprite.");
  }
  bmin::UniquePtr<Sprite> owned(sprite);
  const bmin::String nameStr = toKey(name);
  if (sprites.contains(nameStr)) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Sprite already exists: '") +
                        nameStr + "'.");
  }
  sprites.insert(nameStr, std::move(owned));
}

AnimationDefinition& Store::storeAnimationDefinition(std::string_view name,
                                                     const bool loop) {
  const bmin::String nameStr = toKey(name);
  if (!anims.contains(nameStr)) {
    anims.insert(nameStr, bmin::makeUnique<AnimationDefinition>(name, loop));
  } else {
    THROW_RUNTIME_ERROR(
        bmin::String("[sdl2w] Animation definition already exists: '") +
        nameStr + "'.");
  }
  return *anims[nameStr];
}

void Store::loadAndStoreFont(std::string_view name, std::string_view path) {
  const bmin::String nameStr = toKey(name);
  const bmin::String pathStr(path.data(), path.size());
  if (fontPaths.contains(nameStr)) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Font already exists: '") +
                        nameStr + "'.");
  }

  bmin::UniquePtr<TTF_Font, SDL_Deleter> defaultFont(
      TTF_OpenFont(pathStr.cStr(), TEXT_SIZE_16));
  if (!defaultFont) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Failed to load font '") +
                        pathStr + "': " + TTF_GetError());
  }
  fontPaths.insert(nameStr, pathStr);
  fonts.insert(fontKey(name, TEXT_SIZE_16, false), std::move(defaultFont));
}

void Store::preloadFontSizes(std::string_view name,
                             std::initializer_list<TextSize> sizes,
                             bool includeOutlines) {
  for (TextSize size : sizes) {
    getFont(name, static_cast<int>(size), false);
    if (includeOutlines) {
      getFont(name, static_cast<int>(size), true);
    }
  }
}

void Store::createFontAlias(std::string_view aliasName,
                            std::string_view loadedFontName) {
  const bmin::String aliasStr = toKey(aliasName);
  if (fontAliases.contains(aliasStr)) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Font alias already exists: '") +
                        aliasStr + "'.");
  }
  if (!fontPaths.contains(loadedFontName)) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Cannot alias unknown font '") +
                        toKey(loadedFontName) + "'.");
  }
  fontAliases.insert(aliasStr, toKey(loadedFontName));
}

void Store::storeSound(std::string_view name,
                       std::string_view path,
                       float volume) {
  const bmin::String nameStr = toKey(name);
  const bmin::String pathStr(path.data(), path.size());
  if (sounds.contains(nameStr)) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Sound already exists: '") +
                        nameStr + "'.");
  }

  float clampedVolume = volume;
  if (clampedVolume < 0.0f) {
    clampedVolume = 0.0f;
  } else if (clampedVolume > 1.0f) {
    clampedVolume = 1.0f;
  }
  if (clampedVolume != volume) {
    LOG(WARN) << "[sdl2w] WARNING Sound volume for '" << name
              << "' clamped from " << volume << " to " << clampedVolume
              << Logger::endl;
  }

  StoredSound stored;
  stored.chunk =
      bmin::UniquePtr<Mix_Chunk, SDL_Deleter>(Mix_LoadWAV(pathStr.cStr()));
  stored.volume = clampedVolume;
  if (!stored.chunk) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Failed to load sound '") +
                        pathStr + "': reason= " + Mix_GetError());
  }
  sounds.insert(nameStr, std::move(stored));
}

void Store::storeMusic(std::string_view name, std::string_view path) {
  const bmin::String nameStr = toKey(name);
  const bmin::String pathStr(path.data(), path.size());
  if (musics.contains(nameStr)) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Music already exists: '") +
                        nameStr + "'.");
  }

  bmin::UniquePtr<Mix_Music, SDL_Deleter> music(Mix_LoadMUS(pathStr.cStr()));
  if (!music) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Failed to load music '") +
                        pathStr + "': reason= " + Mix_GetError());
  }
  musics.insert(nameStr, std::move(music));
}

SDL_Texture* Store::getTexture(std::string_view name) {
  auto it = textures.find(name);
  if (it != textures.end()) {
    return (*it).value.get();
  }
  const bmin::String nameStr = toKey(name);
  THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Cannot get Texture '") +
                      nameStr + "' because it has not been loaded.");
}

SDL_Texture* Store::getDynamicTexture(std::string_view name) {
  auto it = dynamicTextures.find(name);
  if (it != dynamicTextures.end()) {
    return (*it).value.get();
  }
  const bmin::String nameStr = toKey(name);
  THROW_RUNTIME_ERROR(
      bmin::String("[sdl2w] ERROR Cannot get DynamicTexture '") + nameStr +
      "' because it has not been loaded.");
}

SDL_Texture* Store::getTextTexture(std::string_view name) {
  return getDynamicTexture(name);
}

Sprite& Store::getSprite(std::string_view name) {
  auto it = sprites.find(name);
  if (it != sprites.end()) {
    return *(*it).value.get();
  }
  const bmin::String nameStr = toKey(name);
  THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Cannot get Sprite '") +
                      nameStr + "' because it has not been loaded.");
}

AnimationDefinition& Store::getAnimationDefinition(std::string_view name) {
  auto it = anims.find(name);
  if (it != anims.end()) {
    return *(*it).value;
  }
  const bmin::String nameStr = toKey(name);
  THROW_RUNTIME_ERROR(
      bmin::String("[sdl2w] ERROR Cannot get AnimationDefinition '") + nameStr +
      "' because it has not been loaded.");
}

TTF_Font*
Store::getFont(std::string_view name, const int sz, const bool isOutline) {
  if (sz <= 0) {
    THROW_RUNTIME_ERROR("[sdl2w] Font size must be positive.");
  }
  bmin::String innerName(name.data(), name.size());
  auto aliasIt = fontAliases.find(innerName);
  if (aliasIt != fontAliases.end()) {
    innerName = (*aliasIt).value;
  }

  const bmin::String key = fontKey(innerName.sliceView(), sz, isOutline);
  auto it = fonts.find(key);
  if (it != fonts.end()) {
    return (*it).value.get();
  }
  auto pathIt = fontPaths.find(innerName);
  if (pathIt == fontPaths.end()) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Cannot get unknown font '") +
                        innerName + "'.");
  }
  bmin::UniquePtr<TTF_Font, SDL_Deleter> font(
      TTF_OpenFont((*pathIt).value.cStr(), sz));
  if (!font) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Failed to load font '") +
                        innerName + "' at size " + bmin::toString(sz) + ": " +
                        TTF_GetError());
  }
  if (isOutline) {
    TTF_SetFontOutline(font.get(), 1);
  }
  TTF_Font* result = font.get();
  fonts.insert(key, std::move(font));
  return result;
}

Mix_Chunk* Store::getSound(std::string_view name) {
  auto it = sounds.find(name);
  if (it != sounds.end()) {
    return (*it).value.chunk.get();
  }
  const bmin::String nameStr = toKey(name);
  THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Cannot get Sound '") +
                      nameStr + "' because it has not been loaded.");
}

float Store::getSoundVolume(std::string_view name) {
  auto it = sounds.find(name);
  if (it != sounds.end()) {
    return (*it).value.volume;
  }
  const bmin::String nameStr = toKey(name);
  THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Cannot get Sound '") +
                      nameStr + "' because it has not been loaded.");
}

Mix_Music* Store::getMusic(std::string_view name) {
  auto it = musics.find(name);
  if (it != musics.end()) {
    return (*it).value.get();
  }
  const bmin::String nameStr = toKey(name);
  THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Cannot get Music '") +
                      nameStr + "' because it has not been loaded.");
}

Animation Store::createAnimation(std::string_view name, bool flipped) {
  auto& def = getAnimationDefinition(name);
  Animation anim(def.name.sliceView(), def.loop);
  anim.flipped = flipped;
  for (auto it = def.sprites.begin(); it != def.sprites.end(); ++it) {
    Sprite& sprite = getSprite((*it).name.sliceView());
    anim.addSprite(*it, sprite);
  }
  return anim;
}

bool Store::hasTexture(std::string_view name) const {
  return textures.contains(name);
}

bool Store::hasDynamicTexture(std::string_view name) const {
  return dynamicTextures.contains(name);
}

bool Store::hasSprite(std::string_view name) const {
  return sprites.contains(name);
}

bool Store::hasAnimationDefinition(std::string_view name) const {
  return anims.contains(name);
}

bool Store::hasFont(std::string_view name) const {
  return fontPaths.contains(name) || fontAliases.contains(name);
}

bool Store::hasFont(std::string_view name, int size, bool isOutline) const {
  bmin::String innerName(name.data(), name.size());
  auto aliasIt = fontAliases.find(innerName);
  if (aliasIt != fontAliases.end()) {
    innerName = (*aliasIt).value;
  }
  return fonts.contains(fontKey(innerName.sliceView(), size, isOutline));
}

bool Store::hasSound(std::string_view name) const {
  return sounds.contains(name);
}

bool Store::hasMusic(std::string_view name) const {
  return musics.contains(name);
}

void Store::logAllSprites() {
  bmin::DynArray<bmin::String> keys;
  for (auto it = sprites.begin(); it != sprites.end(); ++it) {
    keys.pushBack((*it).key);
  }
  std::sort(keys.begin(),
            keys.end(),
            [](const bmin::String& a, const bmin::String& b) { return a < b; });

  Logger().get(INFO) << "[sdl2w] All sprites:" << Logger::endl;
  for (size_t i = 0; i < keys.size(); ++i) {
    Logger().get(INFO) << "  " << keys[i] << Logger::endl;
  }
}

void Store::logAllAnimationDefinitions() {
  bmin::DynArray<bmin::String> keys;
  for (auto it = anims.begin(); it != anims.end(); ++it) {
    keys.pushBack((*it).key);
  }
  std::sort(keys.begin(),
            keys.end(),
            [](const bmin::String& a, const bmin::String& b) { return a < b; });

  Logger().get(INFO) << "[sdl2w] All animation definitions:" << Logger::endl;
  for (size_t i = 0; i < keys.size(); ++i) {
    Logger().get(INFO) << "  " << keys[i] << Logger::endl;
  }
}

void Store::clear() {
  textures.clear();
  dynamicTextures.clear();
  sprites.clear();
  anims.clear();
  sounds.clear();
  musics.clear();
  fonts.clear();
  fontAliases.clear();
  fontPaths.clear();
}

} // namespace sdl2w
