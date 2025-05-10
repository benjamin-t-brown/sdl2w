#pragma once

#include "Draw.h"
#include "Store.h"
#include <string>

namespace sdl2w {

enum AssetFileType {
  DEPRECATED_ASSET_TYPE_SPRITE,
  DEPRECATED_ASSET_TYPE_ANIMATION,
  DEPRECATED_ASSET_TYPE_SOUND,
  ASSET_FILE
};

class AssetLoader {
  Draw& draw;
  Store& store;

  void loadPicture(const std::string& name, const std::string& path);
  void loadSprite(const std::string& name,
                  SDL_Texture* tex,
                  SDL_Surface* surf,
                  bool flipped);
  void loadSprite(const std::string& name,
                  SDL_Texture* tex,
                  SDL_Surface* surf,
                  int spritesheetWidth,
                  int x,
                  int y,
                  int w,
                  int h,
                  bool flipped);
  void loadSpriteSheet(const std::string& pictureName,
                       const std::string& spriteName,
                       int lastSpriteInd,
                       int n,
                       int w,
                       int h);
  void loadAnimationDefinition(const std::string& name, bool loop);

  void loadSpriteAssetsFromFile(const std::string& path);
  void loadAnimationAssetsFromFile(const std::string& path);
  void loadSoundAssetsFromFile(const std::string& path);
  void loadAssetFile(const std::string& path);

public:
  std::unordered_map<std::string, std::string> picturePathToAlias;
  std::unordered_map<std::string, std::string> spriteNameToPictureAlias;

  AssetLoader(Draw& drawA, Store& storeA) : draw(drawA), store(storeA) {}

  void loadAssetsFromFile(AssetFileType type, const std::string& path);
};

std::string loadFileAsString(const std::string& path);
void saveFileAsString(const std::string& path, const std::string& content);

} // namespace sdl2w