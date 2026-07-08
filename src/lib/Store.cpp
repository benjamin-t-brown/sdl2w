#include "Store.h"
#include "Animation.h"
#include "Defines.h"
#include "Draw.h"
#include "Logger.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include <algorithm>
#include <string_view>

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

namespace sdl2w {

static bmin::String toKey(std::string_view sv) {
  return bmin::String(sv.data(), sv.size());
}

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

void Store::storeTexture(std::string_view name, SDL_Texture* tex) {
  const bmin::String nameStr = toKey(name);
  if (textures.contains(nameStr)) {
    LOG(WARN) << "[sdl2w] WARNING Texture with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }
  textures[nameStr] = bmin::UniquePtr<SDL_Texture, SDL_Deleter>(tex);
}

void Store::storeDynamicTexture(std::string_view name, SDL_Texture* tex) {
  dynamicTextures[toKey(name)] =
      bmin::UniquePtr<SDL_Texture, SDL_Deleter>(tex);
}

void Store::storeSprite(std::string_view name, Sprite* sprite) {
  const bmin::String nameStr = toKey(name);
  if (sprites.contains(nameStr)) {
    LOG(WARN) << "[sdl2w] WARNING Sprite with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }
  sprites[nameStr] = bmin::UniquePtr<Sprite>(sprite);
}

AnimationDefinition& Store::storeAnimationDefinition(std::string_view name,
                                                     const bool loop) {
  const bmin::String nameStr = toKey(name);
  if (!anims.contains(nameStr)) {
    anims[nameStr] = bmin::makeUnique<AnimationDefinition>(name, loop);
  } else {
    LOG(WARN) << "[sdl2w] WARNING Cannot store new anim, it already exists: '"
              << nameStr << "'" << Logger::endl;
  }
  return *anims[nameStr];
}

void Store::loadAndStoreFont(std::string_view name, std::string_view path) {
  const bmin::String pathStr(path.data(), path.size());
  static const int sizes[] = {TEXT_SIZE_10, TEXT_SIZE_12, TEXT_SIZE_14,
                              TEXT_SIZE_15, TEXT_SIZE_16, TEXT_SIZE_18,
                              TEXT_SIZE_20, TEXT_SIZE_22, TEXT_SIZE_24,
                              TEXT_SIZE_28, TEXT_SIZE_32, TEXT_SIZE_36,
                              TEXT_SIZE_48, TEXT_SIZE_60, TEXT_SIZE_72};

  for (const int size : sizes) {
    bmin::String key = toKey(name);
    key.append(bmin::toString(size));
    fonts[key] = bmin::UniquePtr<TTF_Font, SDL_Deleter>(
        TTF_OpenFont(pathStr.cStr(), size));

    if (!fonts[key]) {
      const bmin::String error(SDL_GetError());
      LOG(ERROR) << "[sdl2w] ERROR Failed to load font '" << pathStr
                 << "': reason= " << error << LOG_ENDL;
      throw bmin::String("[sdl2w] ERROR Failed to load font '") + pathStr +
            "': reason= " + error;
    }

    bmin::String outlineKey = key + "o";
    fonts[outlineKey] = bmin::UniquePtr<TTF_Font, SDL_Deleter>(
        TTF_OpenFont(pathStr.cStr(), size));
    TTF_SetFontOutline(fonts[outlineKey].get(), 1);
  }
}

void Store::createFontAlias(std::string_view aliasName,
                            std::string_view loadedFontName) {
  const bmin::String aliasStr = toKey(aliasName);
  if (fontAliases.contains(aliasStr)) {
    LOG(WARN) << "[sdl2w] WARNING Font alias with name '" << aliasName
              << "' already exists to '" << loadedFontName << "'"
              << Logger::endl;
  }
  fontAliases[aliasStr] = toKey(loadedFontName);
}

void Store::storeSound(std::string_view name, std::string_view path) {
  const bmin::String nameStr = toKey(name);
  const bmin::String pathStr(path.data(), path.size());
  if (sounds.contains(nameStr)) {
    LOG(WARN) << "[sdl2w] WARNING Sound with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }

  sounds[nameStr] = bmin::UniquePtr<Mix_Chunk, SDL_Deleter>(
      Mix_LoadWAV(pathStr.cStr()));
  if (!sounds[nameStr]) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Failed to load sound '") +
                        pathStr + "': reason= " + Mix_GetError());
  }
}

void Store::storeMusic(std::string_view name, std::string_view path) {
  const bmin::String nameStr = toKey(name);
  const bmin::String pathStr(path.data(), path.size());
  if (musics.contains(nameStr)) {
    LOG(WARN) << "[sdl2w] WARNING Music with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }

  musics[nameStr] = bmin::UniquePtr<Mix_Music, SDL_Deleter>(
      Mix_LoadMUS(pathStr.cStr()));
  if (!musics[nameStr]) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Failed to load music '") +
                        pathStr + "': reason= " + Mix_GetError());
  }
}

SDL_Texture* Store::getTexture(std::string_view name) {
  const bmin::String nameStr = toKey(name);
  auto it = textures.find(nameStr);
  if (it != textures.end()) {
    return (*it).value.get();
  }
  THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Cannot get Texture '") +
                      nameStr + "' because it has not been loaded.");
}

SDL_Texture* Store::getDynamicTexture(std::string_view name) {
  const bmin::String nameStr = toKey(name);
  auto it = dynamicTextures.find(nameStr);
  if (it != dynamicTextures.end()) {
    return (*it).value.get();
  }
  THROW_RUNTIME_ERROR(
      bmin::String("[sdl2w] ERROR Cannot get DynamicTexture '") + nameStr +
      "' because it has not been loaded.");
}

SDL_Texture* Store::getTextTexture(std::string_view name) {
  return getDynamicTexture(name);
}

Sprite& Store::getSprite(std::string_view name) {
  const bmin::String nameStr = toKey(name);
  auto it = sprites.find(nameStr);
  if (it != sprites.end()) {
    return *(*it).value.get();
  }
  THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Cannot get Sprite '") +
                      nameStr + "' because it has not been loaded.");
}

AnimationDefinition& Store::getAnimationDefinition(std::string_view name) {
  const bmin::String nameStr = toKey(name);
  auto it = anims.find(nameStr);
  if (it != anims.end()) {
    return *(*it).value;
  }
  THROW_RUNTIME_ERROR(
      bmin::String("[sdl2w] ERROR Cannot get AnimationDefinition '") +
      nameStr + "' because it has not been loaded.");
}

TTF_Font*
Store::getFont(std::string_view name, const int sz, const bool isOutline) {
  bmin::String innerName(name.data(), name.size());
  auto aliasIt = fontAliases.find(innerName);
  if (aliasIt != fontAliases.end()) {
    innerName = (*aliasIt).value;
  }

  bmin::String key = innerName + bmin::toString(sz) + (isOutline ? "o" : "");
  auto it = fonts.find(key);
  if (it != fonts.end()) {
    return (*it).value.get();
  }
  THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Cannot get Font '") + key +
                      "' because it has not been created.");
}

Mix_Chunk* Store::getSound(std::string_view name) {
  const bmin::String nameStr = toKey(name);
  auto it = sounds.find(nameStr);
  if (it != sounds.end()) {
    return (*it).value.get();
  }
  THROW_RUNTIME_ERROR(bmin::String("[sdl2w] ERROR Cannot get Sound '") +
                      nameStr + "' because it has not been loaded.");
}

Mix_Music* Store::getMusic(std::string_view name) {
  const bmin::String nameStr = toKey(name);
  auto it = musics.find(nameStr);
  if (it != musics.end()) {
    return (*it).value.get();
  }
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

bool Store::hasDynamicTexture(std::string_view name) {
  return dynamicTextures.contains(toKey(name));
}

void Store::logAllSprites() {
  bmin::DynArray<bmin::String> keys;
  for (auto it = sprites.begin(); it != sprites.end(); ++it) {
    keys.pushBack((*it).key);
  }
  std::sort(keys.begin(), keys.end(),
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
  std::sort(keys.begin(), keys.end(),
            [](const bmin::String& a, const bmin::String& b) { return a < b; });

  Logger().get(INFO) << "[sdl2w] All animation definitions:" << Logger::endl;
  for (size_t i = 0; i < keys.size(); ++i) {
    Logger().get(INFO) << "  " << keys[i] << Logger::endl;
  }
}

void Store::clear() {
  textures = bmin::Map<bmin::String, bmin::UniquePtr<SDL_Texture, SDL_Deleter>>();
  dynamicTextures =
      bmin::Map<bmin::String, bmin::UniquePtr<SDL_Texture, SDL_Deleter>>();
  sprites = bmin::Map<bmin::String, bmin::UniquePtr<Sprite>>();
  anims = bmin::Map<bmin::String, bmin::UniquePtr<AnimationDefinition>>();
  sounds = bmin::Map<bmin::String, bmin::UniquePtr<Mix_Chunk, SDL_Deleter>>();
  musics = bmin::Map<bmin::String, bmin::UniquePtr<Mix_Music, SDL_Deleter>>();
  fonts = bmin::Map<bmin::String, bmin::UniquePtr<TTF_Font, SDL_Deleter>>();
}
} // namespace sdl2w
