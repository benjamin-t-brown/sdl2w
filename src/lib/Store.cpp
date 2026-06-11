#include "Store.h"
#include "Animation.h"
#include "Defines.h"
#include "Draw.h"
#include "Logger.h"
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
  // This segfaults for some reason on machines without a joystick attached
  // if (p != nullptr && p != NULL) {
  //   SDL_JoystickClose(p);
  // }
}

void Store::storeTexture(std::string_view name, SDL_Texture* tex) {
  const std::string nameStr(name);
  // LOG_LINE(DEBUG) << "[sdl2w] Store texture: " << name << Logger::endl;
  if (textures.find(nameStr) != textures.end()) {
    LOG(WARN) << "[sdl2w] WARNING Texture with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }
  textures[nameStr] = std::unique_ptr<SDL_Texture, SDL_Deleter>(tex);
}

void Store::storeDynamicTexture(std::string_view name, SDL_Texture* tex) {
  // LOG_LINE(DEBUG) << "[sdl2w] Store dynamic texture: " << name <<
  // Logger::endl;
  dynamicTextures[std::string(name)] =
      std::unique_ptr<SDL_Texture, SDL_Deleter>(tex);
}

void Store::storeSprite(std::string_view name, Sprite* sprite) {
  const std::string nameStr(name);
  if (sprites.find(nameStr) != sprites.end()) {
    LOG(WARN) << "[sdl2w] WARNING Sprite with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }
  sprites[nameStr] = std::unique_ptr<Sprite>(sprite);
}

AnimationDefinition& Store::storeAnimationDefinition(std::string_view name,
                                                       const bool loop) {
  const std::string nameStr(name);
  if (anims.find(nameStr) == anims.end()) {
    anims[nameStr] = std::make_unique<AnimationDefinition>(nameStr, loop);
  } else {
    LOG(WARN) << "[sdl2w] WARNING Cannot store new anim, it already "
                 "exists: '" +
                     nameStr + "'"
              << Logger::endl;
  }
  return *anims[nameStr];
}

void Store::loadAndStoreFont(std::string_view name, std::string_view path) {
  const std::string pathStr(path);
  static const std::vector<int> sizes = {TEXT_SIZE_10,
                                         TEXT_SIZE_12,
                                         TEXT_SIZE_14,
                                         TEXT_SIZE_15,
                                         TEXT_SIZE_16,
                                         TEXT_SIZE_18,
                                         TEXT_SIZE_20,
                                         TEXT_SIZE_22,
                                         TEXT_SIZE_24,
                                         TEXT_SIZE_28,
                                         TEXT_SIZE_32,
                                         TEXT_SIZE_36,
                                         TEXT_SIZE_48,
                                         TEXT_SIZE_60,
                                         TEXT_SIZE_72};

  for (const int& size : sizes) {
    const std::string key = std::string(name) + std::to_string(size);
    fonts[key] = std::unique_ptr<TTF_Font, SDL_Deleter>(
        TTF_OpenFont(pathStr.c_str(), size));

    if (!fonts[key]) {
      auto error = std::string(SDL_GetError());
      LOG(ERROR) << "[sdl2w] ERROR Failed to load font '" << pathStr << "': reason= " << error << LOG_ENDL;
      throw std::string("[sdl2w] ERROR Failed to load font '" + pathStr +
                        "': reason= " + error);
    }

    fonts[key + "o"] = std::unique_ptr<TTF_Font, SDL_Deleter>(
        TTF_OpenFont(pathStr.c_str(), size));
    TTF_SetFontOutline(fonts[key + "o"].get(), 1);
  }
}

void Store::createFontAlias(std::string_view aliasName,
                            std::string_view loadedFontName) {
  const std::string aliasStr(aliasName);
  if (fontAliases.find(aliasStr) != fontAliases.end()) {
    LOG(WARN) << "[sdl2w] WARNING Font alias with name '" << aliasName
              << "' already exists to '" << loadedFontName << "'"
              << Logger::endl;
  }
  fontAliases[aliasStr] = std::string(loadedFontName);
}

void Store::storeSound(std::string_view name, std::string_view path) {
  const std::string nameStr(name);
  const std::string pathStr(path);
  if (sounds.find(nameStr) != sounds.end()) {
    LOG(WARN) << "[sdl2w] WARNING Sound with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }

  sounds[nameStr] = std::unique_ptr<Mix_Chunk, SDL_Deleter>(
      Mix_LoadWAV(pathStr.c_str()));
  if (!sounds[nameStr]) {
    THROW_RUNTIME_ERROR(
        std::string("[sdl2w] ERROR Failed to load sound '" + pathStr +
                    "': reason= " + std::string(Mix_GetError())));
  }
}

void Store::storeMusic(std::string_view name, std::string_view path) {
  const std::string nameStr(name);
  const std::string pathStr(path);
  if (musics.find(nameStr) != musics.end()) {
    LOG(WARN) << "[sdl2w] WARNING Music with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }

  musics[nameStr] = std::unique_ptr<Mix_Music, SDL_Deleter>(
      Mix_LoadMUS(pathStr.c_str()), SDL_Deleter());
  if (!musics[nameStr]) {
    THROW_RUNTIME_ERROR(
        std::string("[sdl2w] ERROR Failed to load music '" + pathStr +
                    "': reason= " + std::string(Mix_GetError())));
  }
}

SDL_Texture* Store::getTexture(std::string_view name) {
  const std::string nameStr(name);
  auto pair = textures.find(nameStr);
  if (pair != textures.end()) {
    return pair->second.get();
  } else {
    THROW_RUNTIME_ERROR("[sdl2w] ERROR Cannot get Texture '" + nameStr +
                        "' because it has not been loaded.");
  }
}
SDL_Texture* Store::getDynamicTexture(std::string_view name) {
  const std::string nameStr(name);
  auto pair = dynamicTextures.find(nameStr);
  if (pair != dynamicTextures.end()) {
    return pair->second.get();
  } else {
    THROW_RUNTIME_ERROR("[sdl2w] ERROR Cannot get DynamicTexture '" + nameStr +
                        "' because it has not been loaded.");
  }
}

SDL_Texture* Store::getTextTexture(std::string_view name) {
  return getDynamicTexture(name);
}
Sprite& Store::getSprite(std::string_view name) {
  const std::string nameStr(name);
  auto pair = sprites.find(nameStr);
  if (pair != sprites.end()) {
    return *pair->second.get();
  } else {
    THROW_RUNTIME_ERROR("[sdl2w] ERROR Cannot get Sprite '" + nameStr +
                        "' because it has not been loaded.");
  }
}

AnimationDefinition& Store::getAnimationDefinition(std::string_view name) {
  const std::string nameStr(name);
  auto pair = anims.find(nameStr);
  if (pair != anims.end()) {
    return *pair->second;
  } else {
    // TODO inconsistent api
    // LOG_LINE(ERROR) << "[sdl2w] ERROR Cannot get AnimationDefinition '" +
    // name +
    //                        "' because it has not been loaded. "
    //                 << Logger::getStackTrace() << Logger::endl;
    // return defaultAnimDef;
    THROW_RUNTIME_ERROR("[sdl2w] ERROR Cannot get AnimationDefinition '" +
                        nameStr + "' because it has not been loaded.");
  }
}

TTF_Font*
Store::getFont(std::string_view name, const int sz, const bool isOutline) {
  std::string innerName(name);
  if (fontAliases.find(innerName) != fontAliases.end()) {
    innerName = fontAliases[innerName];
  }

  const std::string key =
      innerName + std::to_string(sz) + (isOutline ? "o" : "");
  auto pair = fonts.find(key);
  if (pair != fonts.end()) {
    return pair->second.get();
  } else {
    THROW_RUNTIME_ERROR("[sdl2w] ERROR Cannot get Font '" + key +
                        "' because it has not been created.");
  }
}

Mix_Chunk* Store::getSound(std::string_view name) {
  const std::string nameStr(name);
  auto pair = sounds.find(nameStr);
  if (pair != sounds.end()) {
    return pair->second.get();
  } else {
    THROW_RUNTIME_ERROR("[sdl2w] ERROR Cannot get Sound '" + nameStr +
                        "' because it has not been loaded.");
  }
}
Mix_Music* Store::getMusic(std::string_view name) {
  const std::string nameStr(name);
  auto pair = musics.find(nameStr);
  if (pair != musics.end()) {
    return pair->second.get();
  } else {
    THROW_RUNTIME_ERROR("[sdl2w] ERROR Cannot get Music '" + nameStr +
                        "' because it has not been loaded.");
  }
}

Animation Store::createAnimation(std::string_view name, bool flipped) {
  auto& def = getAnimationDefinition(name);
  Animation anim(def.name, def.loop);
  anim.flipped = flipped;
  for (auto& spriteDef : def.sprites) {
    Sprite& sprite = getSprite(spriteDef.name);
    anim.addSprite(spriteDef, sprite);
  }
  return anim;
}

bool Store::hasDynamicTexture(std::string_view name) {
  const std::string nameStr(name);
  return dynamicTextures.find(nameStr) != dynamicTextures.end();
}

void Store::logAllSprites() {
  std::vector<std::string> keys;
  for (const auto& pair : sprites) {
    keys.push_back(pair.first);
  }
  std::sort(keys.begin(), keys.end());

  Logger().get(INFO) << "[sdl2w] All sprites:" << Logger::endl;
  for (const auto& key : keys) {
    Logger().get(INFO) << "  " << key << Logger::endl;
  }
}

void Store::logAllAnimationDefinitions() {
  std::vector<std::string> keys;
  for (const auto& pair : anims) {
    keys.push_back(pair.first);
  }
  std::sort(keys.begin(), keys.end());

  Logger().get(INFO) << "[sdl2w] All animation definitions:" << Logger::endl;
  for (const auto& key : keys) {
    Logger().get(INFO) << "  " << key << Logger::endl;
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
}
} // namespace sdl2w