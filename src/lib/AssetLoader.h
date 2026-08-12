#pragma once

#include "Draw.h"
#include "Store.h"
#include "bmin/DynArray.h"
#include "bmin/Map.h"
#include "bmin/String.h"
#include <string_view>

namespace sdl2w {

enum AssetFileType {
  DEPRECATED_ASSET_TYPE_SPRITE,
  DEPRECATED_ASSET_TYPE_ANIMATION,
  DEPRECATED_ASSET_TYPE_SOUND,
  ASSET_FILE
};

// Preferred audio container when loading Sound/Music from asset files.
// Paths with an extension are rewritten to this mode's extension; paths
// without an extension get the extension appended.
enum SoundFileMode {
  SOUND_FILE_WAV = 0,
  SOUND_FILE_OGG = 1,
};

class AssetLoader {
  Draw& draw;
  Store& store;
  SoundFileMode soundFileMode = SOUND_FILE_WAV;

  void loadPicture(std::string_view name, std::string_view path);
  void loadSprite(std::string_view name, SDL_Texture* tex, bool flipped);
  void loadSprite(std::string_view name,
                  SDL_Texture* tex,
                  int spritesheetWidth,
                  int x,
                  int y,
                  int w,
                  int h,
                  bool flipped);
  void loadSpriteSheet(std::string_view pictureName,
                       std::string_view spriteName,
                       int lastSpriteInd,
                       int n,
                       int w,
                       int h);
  void loadAnimationDefinition(std::string_view name, bool loop);

  void loadSpriteAssetsFromFile(std::string_view path);
  void loadAnimationAssetsFromFile(std::string_view path);
  void loadSoundAssetsFromFile(std::string_view path);
  void loadAssetFile(std::string_view path);

  bmin::String resolveSoundPath(std::string_view path) const;
  // If token is a numeric volume in/near [0,1], write clamped value and return
  // true. Non-numeric tokens (e.g. attribution text) return false.
  static bool tryParseSoundVolume(const bmin::String& token, float& outVolume);

public:
  bmin::Map<bmin::String, bmin::String> picturePathToAlias;
  bmin::Map<bmin::String, bmin::String> spriteNameToPictureAlias;
  static bool fsReady;

  AssetLoader(Draw& drawA, Store& storeA) : draw(drawA), store(storeA) {}
  static void initFs();

  void setSoundFileMode(SoundFileMode mode) { soundFileMode = mode; }
  SoundFileMode getSoundFileMode() const { return soundFileMode; }

  void loadAssetsFromFile(AssetFileType type, std::string_view path);
};

bmin::String slice(std::string_view str, int start, int end);
bmin::String trim(std::string_view str);
void split(std::string_view str,
           std::string_view delimiter,
           bmin::DynArray<bmin::String>& out);
bmin::String loadFileAsString(std::string_view path);
void saveFileAsString(std::string_view path, std::string_view content);

} // namespace sdl2w
