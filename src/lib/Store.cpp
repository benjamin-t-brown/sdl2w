#include "Store.h"
#include "Animation.h"
#include "Defines.h"
#include "Draw.h"
#include "Logger.h"
#include <algorithm>
#include <stdexcept>

#if defined(MIYOOA30) || defined(MIYOOMINI)
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

void Store::storeTexture(const std::string& name, SDL_Texture* tex) {
  // LOG_LINE(DEBUG) << "[sdl2w] Store texture: " << name << Logger::endl;
  if (textures.find(name) != textures.end()) {
    LOG(WARN) << "[sdl2w] WARNING Texture with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }
  textures[name] = std::unique_ptr<SDL_Texture, SDL_Deleter>(tex);
}

void Store::storeDynamicTexture(const std::string& name, SDL_Texture* tex) {
  // LOG_LINE(DEBUG) << "[sdl2w] Store dynamic texture: " << name <<
  // Logger::endl;
  dynamicTextures[name] = std::unique_ptr<SDL_Texture, SDL_Deleter>(tex);
}

void Store::storeSurface(const std::string& name, SDL_Surface* surf) {
  // LOG_LINE(DEBUG) << "[sdl2w] Store surface: " << name << Logger::endl;
  if (surfaces.find(name) != surfaces.end()) {
    LOG(WARN) << "[sdl2w] WARNING Surface with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }
  surfaces[name] = std::unique_ptr<SDL_Surface, SDL_Deleter>(surf);
}

void Store::storeDynamicSurface(const std::string& name, SDL_Surface* surf) {
  // LOG_LINE(DEBUG) << "[sdl2w] Store dynamic surface: " << name <<
  // Logger::endl;
  dynamicSurfaces[name] = std::unique_ptr<SDL_Surface, SDL_Deleter>(surf);
}

void Store::storeSprite(const std::string& name, Sprite* sprite) {
  if (sprites.find(name) != sprites.end()) {
    LOG(WARN) << "[sdl2w] WARNING Sprite with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }
  sprites[name] = std::unique_ptr<Sprite>(sprite);
}

AnimationDefinition& Store::storeAnimationDefinition(const std::string& name,
                                                     const bool loop) {
  if (anims.find(name) == anims.end()) {
    anims[name] = std::make_unique<AnimationDefinition>(name, loop);
  } else {
    LOG(WARN) << "[sdl2w] WARNING Cannot store new anim, it already "
                 "exists: '" +
                     name + "'"
              << Logger::endl;
  }
  return *anims[name];
}

void Store::loadAndStoreFont(const std::string& name, const std::string& path) {
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
    const std::string key = name + std::to_string(size);
    fonts[key] = std::unique_ptr<TTF_Font, SDL_Deleter>(
        TTF_OpenFont(path.c_str(), size));

    if (!fonts[key]) {
      throw std::string("[sdl2w] ERROR Failed to load font '" + path +
                        "': reason= " + std::string(SDL_GetError()));
    }

    fonts[key + "o"] = std::unique_ptr<TTF_Font, SDL_Deleter>(
        TTF_OpenFont(path.c_str(), size));
    TTF_SetFontOutline(fonts[key + "o"].get(), 1);
  }
}

void Store::createFontAlias(const std::string& aliasName,
                            const std::string& loadedFontName) {
  if (fontAliases.find(aliasName) != fontAliases.end()) {
    LOG(WARN) << "[sdl2w] WARNING Font alias with name '" << aliasName
              << "' already exists to '" << loadedFontName << "'"
              << Logger::endl;
  }
  fontAliases[aliasName] = loadedFontName;
}

void Store::storeSound(const std::string& name, const std::string& path) {
  if (sounds.find(name) != sounds.end()) {
    LOG(WARN) << "[sdl2w] WARNING Sound with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }

  sounds[name] =
      std::unique_ptr<Mix_Chunk, SDL_Deleter>(Mix_LoadWAV(path.c_str()));
  if (!sounds[name]) {
    LOG_LINE(ERROR) << std::string("[sdl2w] ERROR Failed to load sound '" +
                                   path +
                                   "': reason= " + std::string(Mix_GetError()))
                    << Logger::endl;
    ;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}

void Store::storeMusic(const std::string& name, const std::string& path) {
  if (musics.find(name) != musics.end()) {
    LOG(WARN) << "[sdl2w] WARNING Music with name '" << name
              << "' already exists. '" << name << "'" << Logger::endl;
  }

  musics[name] = std::unique_ptr<Mix_Music, SDL_Deleter>(
      Mix_LoadMUS(path.c_str()), SDL_Deleter());
  if (!musics[name]) {
    LOG_LINE(ERROR) << std::string("[sdl2w] ERROR Failed to load music '" +
                                   path +
                                   "': reason= " + std::string(Mix_GetError()))
                    << Logger::endl;
    ;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}

SDL_Texture* Store::getTexture(const std::string& name) {
  auto pair = textures.find(name);
  if (pair != textures.end()) {
    return pair->second.get();
  } else {
    LOG_LINE(ERROR) << "[sdl2w] ERROR Cannot get Texture '" + name +
                           "' because it has not been loaded."
                    << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}
SDL_Texture* Store::getDynamicTexture(const std::string& name) {
  auto pair = dynamicTextures.find(name);
  if (pair != dynamicTextures.end()) {
    return pair->second.get();
  } else {
    LOG_LINE(ERROR) << "[sdl2w] ERROR Cannot get DynamicTexture '" + name +
                           "' because it has not been loaded."
                    << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}
SDL_Surface* Store::getSurface(const std::string& name) {
  auto pair = surfaces.find(name);
  if (pair != surfaces.end()) {
    return pair->second.get();
  } else {
    LOG_LINE(ERROR) << "[sdl2w] ERROR Cannot get Surface '" + name +
                           "' because it has not been loaded."
                    << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}
SDL_Surface* Store::getDynamicSurface(const std::string& name) {
  auto pair = dynamicSurfaces.find(name);
  if (pair != dynamicSurfaces.end()) {
    return pair->second.get();
  } else {
    LOG_LINE(ERROR) << "[sdl2w] ERROR Cannot get DynamicSurface '" + name +
                           "' because it has not been loaded."
                    << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}
Sprite& Store::getSprite(const std::string& name) {
  auto pair = sprites.find(name);
  if (pair != sprites.end()) {
    return *pair->second.get();
  } else {
    LOG_LINE(ERROR) << "[sdl2w] ERROR Cannot get Sprite '" + name +
                           "' because it has not been loaded."
                    << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}

AnimationDefinition& Store::getAnimationDefinition(const std::string& name) {
  auto pair = anims.find(name);
  if (pair != anims.end()) {
    return *pair->second;
  } else {
    LOG_LINE(ERROR) << "[sdl2w] ERROR Cannot get AnimationDefinition '" << name
                    << "' because it has not been created." << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}

TTF_Font*
Store::getFont(const std::string& name, const int sz, const bool isOutline) {
  std::string innerName = name;
  if (fontAliases.find(name) != fontAliases.end()) {
    innerName = fontAliases[name];
  }

  const std::string key =
      innerName + std::to_string(sz) + (isOutline ? "o" : "");
  auto pair = fonts.find(key);
  if (pair != fonts.end()) {
    return pair->second.get();
  } else {
    LOG_LINE(ERROR) << "[sdl2w] ERROR Cannot get Font '" << key
                    << "' because it has not been created." << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}

Mix_Chunk* Store::getSound(const std::string& name) {
  auto pair = sounds.find(name);
  if (pair != sounds.end()) {
    return pair->second.get();
  } else {
    LOG_LINE(ERROR) << "[sdl2w] ERROR Cannot get Sound '" + name +
                           "' because it has not been loaded."
                    << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}
Mix_Music* Store::getMusic(const std::string& name) {
  auto pair = musics.find(name);
  if (pair != musics.end()) {
    return pair->second.get();
  } else {
    LOG_LINE(ERROR) << "[sdl2w] ERROR Cannot get Music '" + name +
                           "' because it has not been loaded."
                    << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}

Animation Store::createAnimation(const std::string& name, bool flipped) {
  auto& def = getAnimationDefinition(name);
  Animation anim(def.name, def.loop);
  for (auto& spriteDef : def.sprites) {
    Sprite& sprite = getSprite(spriteDef.name +
                               (flipped ? std::string(SPRITE_FLIPPED) : ""));
    anim.addSprite(spriteDef, sprite);
  }
  return anim;
}

bool Store::hasDynamicTextureOrSurface(const std::string& name) {
  return (dynamicSurfaces.find(name) != dynamicSurfaces.end() ||
          dynamicTextures.find(name) != dynamicTextures.end());
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
  surfaces.clear();
  dynamicSurfaces.clear();
  sprites.clear();
  anims.clear();
  sounds.clear();
  musics.clear();
  fonts.clear();
}
} // namespace sdl2w