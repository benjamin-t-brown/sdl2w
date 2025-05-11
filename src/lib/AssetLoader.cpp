#include "AssetLoader.h"
#include "Defines.h"
#include "Draw.h"
#include "Logger.h"
#include <filesystem>
#include <fstream> // Added fstream include for std::ifstream

#include <map>     // Added map include
#include <sstream> // Added sstream include for std::stringstream
#include <stdexcept>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if defined(MIYOOA30) || defined(MIYOOMINI)
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#endif

namespace sdl2w {

bool AssetLoader::fsReady = false;

std::string slice(const std::string& str, int start, int end) {
  const int len = static_cast<int>(str.length());

  // Normalize start index to be like JavaScript's slice
  int actualStart = start < 0 ? std::max(len + start, 0) : std::min(start, len);

  // Normalize end index to be like JavaScript's slice
  int actualEnd;
  if (end < 0) {
    actualEnd = std::max(len + end, 0);
  } else {
    actualEnd = std::min(end, len);
  }

  if (actualStart >= actualEnd) {
    return std::string(); // Return empty string if range is invalid or empty
  }

  return str.substr(actualStart, actualEnd - actualStart);
}

std::string trim(const std::string& str) {
  const char* whitespace = " \n\r\t"; // Define whitespace characters

  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos) {
    return ""; // String contains only whitespace or is empty
  }

  const auto strEnd = str.find_last_not_of(whitespace);
  // No need to check for npos for strEnd, as strBegin found something

  return str.substr(strBegin, strEnd - strBegin + 1);
}

void split(
    const std::string& str,
    const std::string& delimiter, // Renamed 'token' to 'delimiter' for clarity
    std::vector<std::string>& out) {
  // Note: JS split returns a new array. This function appends to 'out'.
  // If 'out' should be cleared first, add out.clear(); here.

  if (delimiter.empty()) {
    // JS behavior: "abc".split("") -> ["a", "b", "c"]
    // JS behavior: "".split("") -> []
    // This implementation will produce [""] for "".split(""), and ["a","b","c"]
    // for "abc".split("") To strictly match JS "".split("") -> [], handle
    // str.empty() case here:
    if (str.empty()) {
      out.clear();
      return;
    }
    for (char c : str) {
      out.emplace_back(1, c);
    }
    return;
  }

  size_t lastPos = 0;
  size_t findPos = 0;

  while ((findPos = str.find(delimiter, lastPos)) != std::string::npos) {
    out.push_back(str.substr(lastPos, findPos - lastPos));
    lastPos = findPos + delimiter.length();
  }
  // Add the last part of the string (or the whole string if delimiter not
  // found)
  out.push_back(str.substr(lastPos));
}

SDL_Surface* flipSurfaceHorizontal(SDL_Surface* surface) {
  if (!surface)
    return nullptr;

  SDL_Surface* flipped = SDL_CreateRGBSurface(surface->flags,
                                              surface->w,
                                              surface->h,
                                              surface->format->BitsPerPixel,
                                              surface->format->Rmask,
                                              surface->format->Gmask,
                                              surface->format->Bmask,
                                              surface->format->Amask);
  if (!flipped) {
    return nullptr;
  }

  SDL_LockSurface(surface);
  SDL_LockSurface(flipped);

  for (int y = 0; y < surface->h; y++) {
    uint8_t* srcRow =
        static_cast<uint8_t*>(surface->pixels) + y * surface->pitch;
    uint8_t* dstRow =
        static_cast<uint8_t*>(flipped->pixels) + y * flipped->pitch;
    for (int x = 0; x < surface->w; x++) {
      uint8_t* srcPixel = srcRow + x * surface->format->BytesPerPixel;
      uint8_t* dstPixel =
          dstRow + (surface->w - 1 - x) * surface->format->BytesPerPixel;
      memcpy(dstPixel, srcPixel, surface->format->BytesPerPixel);
    }
  }

  SDL_UnlockSurface(surface);
  SDL_UnlockSurface(flipped);

  return flipped;
}

bool strEndsWith(const std::string& fullString, const std::string& ending) {
  if (fullString.length() >= ending.length()) {
    return (0 == fullString.compare(fullString.length() - ending.length(),
                                    ending.length(),
                                    ending));
  } else {
    return false;
  }
}

#ifdef __EMSCRIPTEN__
extern "C" {
EMSCRIPTEN_KEEPALIVE void fs_init_complete(int success) {
  AssetLoader::fsReady = success == 1;
}
}
#endif

// async function to initialize the filesystem
void AssetLoader::initFs() {
#ifdef __EMSCRIPTEN__
  EM_ASM({
    // create save dir
    try {
      FS.mkdir('/sdl2wdata');
    } catch (e) {
      // Directory might already exist
      if (e.code != 'EEXIST') {
        console.error("[sdl2w] Error creating directory:", e);
        _fs_init_complete(0);
        return;
      }
    }

    // mount save dir
    try {
      FS.mount(IDBFS, {}, '/sdl2wdata');
    } catch (e) {
      console.error("[sdl2w console] Error mounting filesystem:", e);
      _fs_init_complete(0);
      return;
    }

    // Load from IndexedDB (async operation)
    FS.syncfs(
        true, function(err) {
          if (err) {
            console.error("[sdl2w console] Error loading filesystem from IndexedDB:",
                          err);
            _fs_init_complete(0);
          } else {
            console.log("[sdl2w console] Filesystem loaded from IndexedDB");
            _fs_init_complete(1);
          }
        });
  });
#else
  // For non-Emscripten platforms, just complete immediately
  fsReady = true;
#endif
}

void AssetLoader::loadPicture(const std::string& name,
                              const std::string& path) {
  SDL_Surface* loadedImage = IMG_Load(path.c_str());

  if (loadedImage == nullptr) {
    LOG_LINE(ERROR) << "[sdl2w] ERROR Failed to load image: " << name << " ("
                    << path << ")" << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
  std::string preferredPath =
      std::filesystem::path(path).make_preferred().string();
  picturePathToAlias[preferredPath] = name;

  SDL_Texture* tex = draw.createTexture(loadedImage);
  store.storeTexture(name, tex);
  store.storeSurface(name, loadedImage);
  loadSprite(name, tex, loadedImage, false);

  // surfaces need to be manually flipped
  SDL_Surface* flippedImage = flipSurfaceHorizontal(loadedImage);
  store.storeSurface(name + std::string(SPRITE_FLIPPED), flippedImage);
  loadSprite(name + std::string(SPRITE_FLIPPED), tex, flippedImage, true);
}

void AssetLoader::loadSprite(const std::string& name,
                             SDL_Texture* tex,
                             SDL_Surface* surf,
                             bool flipped) {
  int width;
  int height;
  SDL_QueryTexture(tex, nullptr, nullptr, &width, &height);
  store.storeSprite(
      name,
      new Sprite{
          name, Renderable{tex, surf}, 0, 0, width, height, width, flipped});
}

void AssetLoader::loadSprite(const std::string& name,
                             SDL_Texture* tex,
                             SDL_Surface* surf,
                             int spritesheetWidth,
                             int x,
                             int y,
                             int w,
                             int h,
                             bool flipped) {
  store.storeSprite(
      name,
      new Sprite{
          name, Renderable{tex, surf}, x, y, w, h, spritesheetWidth, flipped});
}

void AssetLoader::loadSpriteSheet(const std::string& pictureName,
                                  const std::string& spriteName,
                                  int lastSpriteInd,
                                  int n,
                                  int w,
                                  int h) {
  Sprite& sprite = store.getSprite(pictureName);
  Sprite& spriteFlipped =
      store.getSprite(pictureName + std::string(SPRITE_FLIPPED));

  int num_x = sprite.w / w;
  int ctr = 0;

  for (int i = lastSpriteInd; i < n; i++) {
    std::string sprName = spriteName + "_" + std::to_string(ctr);

    spriteNameToPictureAlias[sprName] = pictureName;
    loadSprite(sprName,
               sprite.renderable.tex,
               sprite.renderable.surf,
               sprite.w,
               (i % num_x) * w,
               (i / num_x) * h,
               w,
               h,
               false);

    spriteNameToPictureAlias[sprName + std::string(SPRITE_FLIPPED)] =
        pictureName;
    loadSprite(sprName + std::string(SPRITE_FLIPPED),
               spriteFlipped.renderable.tex,
               spriteFlipped.renderable.surf,
               sprite.w,
               (i % num_x) * w,
               (i / num_x) * h,
               w,
               h,
               true);
    ctr++;
  }
}

void AssetLoader::loadAnimationDefinition(const std::string& name, bool loop) {}

void AssetLoader::loadSpriteAssetsFromFile(const std::string& path) {
  LOG(DEBUG) << "[sdl2w] Loading sprite assets from file "
             << (std::string(ASSETS_PREFIX) + path) << Logger::endl;
  std::ifstream file(std::string(ASSETS_PREFIX) + path);
  if (!file.is_open()) {
    LOG_LINE(ERROR) << "[sdl2w] Failed to open file: "
                    << (std::string(ASSETS_PREFIX) + path) << Logger::endl;
    return;
  }
  std::string lastPicture;
  int lastSpriteInd = 0;
  std::string line;
  try {
    while (std::getline(file, line)) {
      line = trim(line);
      if (line.empty()) {
        continue;
      }
      std::vector<std::string> arr;
      split(line, ",", arr);
      if (arr.empty()) {
        continue;
      }
      if (arr[0] == "Picture") {
        // LOG_LINE(DEBUG) << "Loading picture: " << arr[1] << Logger::endl;
        lastPicture = arr[1];
        lastSpriteInd = 0;
        loadPicture(arr[1], arr[2]);
      } else if (arr[0] == "SpriteList") {
        // LOG_LINE(DEBUG) << "Loading spriteList: " << arr[1] <<
        // Logger::endl;
        std::string name = arr[1];
        int n = std::stoi(arr[2]) + lastSpriteInd;
        int w = std::stoi(arr[3]);
        int h = std::stoi(arr[4]);
        loadSpriteSheet(lastPicture, name, lastSpriteInd, n, w, h);
        lastSpriteInd = n;
      } else if (arr[0] == "Sprite") {
        // NOTE: Deprecated single sprites do not support flipping
        // LOG_LINE(DEBUG)
        //     << "Loading single sprite: " << arr[1] << Logger::endl;
        Sprite& spriteImage = store.getSprite(lastPicture);

        std::string name = arr[1];
        int x = std::stoi(arr[2]);
        int y = std::stoi(arr[3]);
        int w = std::stoi(arr[4]);
        int h = std::stoi(arr[5]);
        spriteNameToPictureAlias[name] = lastPicture;
        loadSprite(name,
                   spriteImage.renderable.tex,
                   spriteImage.renderable.surf,
                   spriteImage.w,
                   x,
                   y,
                   w,
                   h,
                   false);
      }
    }
    file.close();
  } catch (std::exception& e) {
    LOG_LINE(ERROR) << "[sdl2w] Failed to parse sprites res file: " << e.what()
                    << Logger::endl;
  }
}
void AssetLoader::loadAnimationAssetsFromFile(const std::string& path) {
  LOG(DEBUG) << "[sdl2w] Loading anim assets from file "
             << (std::string(ASSETS_PREFIX) + path) << Logger::endl;
  try {
    std::string animName = "";
    std::string line;

    std::ifstream file(std::string(ASSETS_PREFIX) + path);

    int lineOffset = 0;

    while (std::getline(file, line)) {
      if (line[0] == '#') {
        lineOffset = 1;
      } else if (line.size() > 1) {
        if (lineOffset == 1) {
          animName = std::string(line);
          lineOffset = 2;
        } else if (lineOffset == 2) {
          const std::string loop = std::string(line);
          store.storeAnimationDefinition(animName,
                                         (loop == "loop" ? true : false));
          lineOffset = 3;
        } else {
          AnimationDefinition& anim = store.getAnimationDefinition(animName);
          std::stringstream ss;
          ss << line;
          std::string strName;
          std::string strFrames;
          std::getline(ss, strName, ' ');
          std::getline(ss, strFrames, ' ');
          int frames = 0;

          try {
            frames = std::stoi(strFrames);
          } catch (std::exception& e) {
            LOG_LINE(ERROR)
                << "[sdl2w] Failed to load anim sprite for: " << animName
                << Logger::endl;
            LOG_LINE(ERROR) << " FROM: '" << line << "'" << Logger::endl;
          }

          anim.addSprite(strName, frames);
        }
      }
    }
    file.close();
  } catch (std::exception& e) {
    LOG_LINE(ERROR) << "[sdl2w] Failed to parse anim res file: " << e.what()
                    << Logger::endl;
  }
}

void AssetLoader::loadSoundAssetsFromFile(const std::string& path) {
  LOG(DEBUG) << "[sdl2w] Loading sound assets from file "
             << (std::string(ASSETS_PREFIX) + path) << Logger::endl;
  std::ifstream file(std::string(ASSETS_PREFIX) + path);
  if (!file.is_open()) {
    LOG_LINE(ERROR) << "[sdl2w] Failed to open file: "
                    << (std::string(ASSETS_PREFIX) + path) << Logger::endl;
    return;
  }
  std::string line;
  try {
    while (std::getline(file, line)) {
      line = trim(line);
      if (line.size()) {
        std::vector<std::string> arr;
        split(line, ",", arr);
        if (arr.size() >= 3) {
          if (arr[0] == "Sound") {
            store.storeSound(arr[1], arr[2]);
          } else if (arr[0] == "Music") {
            store.storeMusic(arr[1], arr[2]);
          }
        }
      }
    }
    file.close();
  } catch (std::exception& e) {
    LOG_LINE(ERROR) << "[sdl2w] Failed to parse sound/music list: " << e.what()
                    << Logger::endl;
  }
}

void AssetLoader::loadAssetFile(const std::string& path) {
  LOG(DEBUG) << "[sdl2w] Loading asset file "
             << (std::string(ASSETS_PREFIX) + path) << Logger::endl;
  std::ifstream file(std::string(ASSETS_PREFIX) + path);
  if (!file.is_open()) {
    LOG_LINE(ERROR) << "[sdl2w] Failed to open file: "
                    << (std::string(ASSETS_PREFIX) + path) << Logger::endl;
    return;
  }

  std::string line;
  std::map<std::string, int>
      nextSpriteIndexForPicture; // Tracks the next sprite index for a given
                                 // picture alias

  std::string currentAnimationName;
  bool parsingAnimationFrames = false;

  try {
    while (std::getline(file, line)) {
      line = trim(line);
      if (line.empty() || line[0] == '#') { // Skip comments and empty lines
        continue;
      }

      if (parsingAnimationFrames) {
        if (line == "EndAnim") {
          parsingAnimationFrames = false;
          currentAnimationName = "";
          continue;
        }
        // Parse animation frame line: <sprite name> <ms>
        std::stringstream ss(line);
        std::string spriteNameStr;
        std::string framesStr;
        // Use std::ws to consume leading whitespace before spriteNameStr
        ss >> std::ws >> spriteNameStr; // Read first word (sprite name)
        ss >> std::ws >> framesStr;     // Read second word (frames)

        if (!spriteNameStr.empty() && !framesStr.empty() &&
            !currentAnimationName.empty()) {
          try {
            int frames = std::stoi(framesStr);
            AnimationDefinition& animDef =
                store.getAnimationDefinition(currentAnimationName);
            animDef.addSprite(spriteNameStr, frames);
          } catch (const std::exception& e) {
            LOG_LINE(ERROR) << "[sdl2w] Failed to parse animation frame for "
                            << currentAnimationName << ": '" << line << "' - "
                            << e.what() << Logger::endl;
          }
        } else {
          LOG(WARN) << "[sdl2w] Malformed or incomplete animation frame line: '"
                    << line << "' for animation '" << currentAnimationName
                    << "'" << Logger::endl;
        }
        continue;
      }

      std::vector<std::string> tokens;
      split(line, ",", tokens);

      if (tokens.empty()) {
        continue;
      }

      const std::string& command = trim(tokens[0]); // Trim the command itself

      if (command == "Pic") {
        if (tokens.size() >= 3) {
          std::string alias = trim(tokens[1]);
          std::string picPath = trim(tokens[2]);
          loadPicture(
              alias, picPath); // Path is from executable as per assets.txt spec
          nextSpriteIndexForPicture[alias] =
              0; // Initialize sprite counter for this picture
        } else {
          LOG(WARN) << "[sdl2w] Malformed Pic asset specified: " << line
                    << Logger::endl;
        }
      } else if (command == "Sprites") {
        if (tokens.size() >= 5) {
          std::string picName = trim(tokens[1]);
          try {
            int numSprites = std::stoi(trim(tokens[2]));
            int spriteWidth = std::stoi(trim(tokens[3]));
            int spriteHeight = std::stoi(trim(tokens[4]));

            if (nextSpriteIndexForPicture.find(picName) ==
                nextSpriteIndexForPicture.end()) {
              LOG(WARN)
                  << "[sdl2w] Sprites command for picture '" << picName
                  << "' encountered without a preceding 'Pic' command for it. "
                     "Assuming sprite index starts at 0."
                  << Logger::endl;
              nextSpriteIndexForPicture[picName] = 0;
            }

            int startIndex = nextSpriteIndexForPicture[picName];
            // The spriteBaseName for loadSpriteSheet should be picName, so
            // sprites are picName_0, picName_1 etc.
            loadSpriteSheet(picName,
                            picName,
                            startIndex,
                            startIndex + numSprites,
                            spriteWidth,
                            spriteHeight);
            nextSpriteIndexForPicture[picName] = startIndex + numSprites;
          } catch (const std::invalid_argument& ia) {
            LOG_LINE(ERROR)
                << "[sdl2w] Invalid number in Sprites asset specified: " << line
                << " - " << ia.what() << Logger::endl;
          } catch (const std::out_of_range& oor) {
            LOG_LINE(ERROR)
                << "[sdl2w] Number out of range in Sprites asset specified: "
                << line << " - " << oor.what() << Logger::endl;
          }
        } else {
          LOG(WARN) << "[sdl2w] Malformed Sprites asset specified: " << line
                    << Logger::endl;
        }
      } else if (command == "Anim") {
        if (tokens.size() >= 3) {
          currentAnimationName = trim(tokens[1]);
          std::string loopStr = trim(tokens[2]);
          bool loop = (loopStr == "loop");
          store.storeAnimationDefinition(currentAnimationName, loop);
          parsingAnimationFrames = true;
        } else {
          LOG(WARN) << "[sdl2w] Malformed Anim asset specified: " << line
                    << Logger::endl;
        }
      } else if (command == "Sound") {
        if (tokens.size() >= 3) {
          std::string alias = trim(tokens[1]);
          std::string soundPath = trim(tokens[2]);
          store.storeSound(alias, soundPath);
        } else {
          LOG(WARN) << "[sdl2w] Malformed Sound asset specified: " << line
                    << Logger::endl;
        }
      } else if (command == "Music") {
        if (tokens.size() >= 3) {
          std::string alias = trim(tokens[1]);
          std::string musicPath = trim(tokens[2]);
          store.storeMusic(alias, musicPath);
        } else {
          LOG(WARN) << "[sdl2w] Malformed Music asset specified: " << line
                    << Logger::endl;
        }
      } else {
        LOG(WARN) << "[sdl2w] Unknown command in asset file: '" << command
                  << "' in line: '" << line << "'" << Logger::endl;
      }
    }
    file.close();
  } catch (const std::exception& e) {
    LOG_LINE(ERROR) << "[sdl2w] Exception while parsing asset file '" << path
                    << "': " << e.what() << Logger::endl;
    if (file.is_open()) {
      file.close();
    }
  }
}

void AssetLoader::loadAssetsFromFile(AssetFileType type,
                                     const std::string& path) {
  switch (type) {
  case DEPRECATED_ASSET_TYPE_SPRITE:
    loadSpriteAssetsFromFile(path);
    break;
  case DEPRECATED_ASSET_TYPE_ANIMATION:
    loadAnimationAssetsFromFile(path);
    break;
  case DEPRECATED_ASSET_TYPE_SOUND:
    loadSoundAssetsFromFile(path);
    break;
  case ASSET_FILE:
    loadAssetFile(path);
    break;
  }
}

std::string loadFileAsString(const std::string& path) {
#ifdef __EMSCRIPTEN__
  // Use a path relative to the mounted filesystem
  std::string fullPath = std::string("/sdl2wdata/") + path;

  // Check if file exists
  if (EM_ASM_INT(
          {
            try {
              return FS.stat(UTF8ToString($0)) != undefined;
            } catch (e) {
              return 0;
            }
          },
          fullPath.c_str())) {
    // File exists, read it
    std::ifstream file(fullPath);
    if (!file) {
      LOG_LINE(ERROR) << "[sdl2w] Error opening file: " << fullPath
                      << Logger::endl;
      return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  } else {
    LOG_LINE(ERROR) << "[sdl2w] File not found: " << fullPath << Logger::endl;
    return "";
  }
#endif
  LOG(DEBUG) << "[sdl2w] Loading file " << (std::string(ASSETS_PREFIX) + path)
             << Logger::endl;
  std::ifstream file(std::string(ASSETS_PREFIX) + path);

  if (!file) {
    LOG_LINE(ERROR) << "[sdl2w] Error opening file: " << path << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void saveFileAsString(const std::string& path, const std::string& content) {
#ifdef __EMSCRIPTEN__
  std::stringstream transformPath;
  transformPath << "/" << "sdl2wdata" << "/" << path;
  LOG(DEBUG) << "[sdl2w] Saving file " << transformPath.str() << Logger::endl;
  std::ofstream file(transformPath.str());
#else
  LOG(DEBUG) << "[sdl2w] Saving file " << (std::string(ASSETS_PREFIX) + path)
             << Logger::endl;
  std::ofstream file(std::string(ASSETS_PREFIX) + path);
#endif

  if (!file) {

#ifdef __EMSCRIPTEN__
    LOG_LINE(ERROR) << "[sdl2w] Error opening file for save: "
                    << transformPath.str() << Logger::endl;
    // HACK emscripten is set to not catch errors
    return;
#else
    LOG_LINE(ERROR) << "[sdl2w] Error opening file for save: " << path
                    << Logger::endl;
    // HACK emscripten is set to not catch errors
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
#endif
  }
  file << content;
  file.close();

#ifdef __EMSCRIPTEN__
  // write files to indexdb
  EM_ASM(FS.syncfs(
      false, function(err) {
        if (err)
          console.error("[sdl2w console] Error syncing to IndexedDB:", err);
        else
          console.log("[sdl2w console] Data synced to IndexedDB");
      }););
#endif
}

} // namespace sdl2w