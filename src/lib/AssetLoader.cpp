#include "AssetLoader.h"
#include "Defines.h"
#include "Draw.h"
#include "Logger.h"
#include <fstream>
#include <istream>
#include <stdexcept>
#include <string_view>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <cstring> // memcpy
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#endif

namespace sdl2w {

namespace {

bmin::String toKey(std::string_view sv) {
  return bmin::String(sv.data(), sv.size());
}

bmin::String assetsPath(std::string_view path) {
  return bmin::String(ASSETS_PREFIX.data(), ASSETS_PREFIX.size()) + toKey(path);
}

bool readLine(std::istream& in, bmin::String& line) {
  line.clear();
  int c = in.get();
  while (c != std::char_traits<char>::eof()) {
    const char ch = static_cast<char>(c);
    if (ch == '\n') {
      if (!line.empty() && line[line.size() - 1] == '\r') {
        line.erase(line.size() - 1, 1);
      }
      return true;
    }
    line.pushBack(ch);
    c = in.get();
  }
  return !line.empty();
}

bmin::String readStreamAsString(std::istream& in) {
  bmin::String result;
  char buf[4096];
  while (true) {
    in.read(buf, sizeof(buf));
    const std::streamsize count = in.gcount();
    if (count <= 0) {
      break;
    }
    result.append(buf, static_cast<size_t>(count));
  }
  return result;
}

} // namespace

bool AssetLoader::fsReady = false;

bmin::String slice(std::string_view str, int start, int end) {
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
    return bmin::String(); // Return empty string if range is invalid or empty
  }

  const std::string_view sub =
      str.substr(static_cast<size_t>(actualStart),
                 static_cast<size_t>(actualEnd - actualStart));
  return bmin::String(sub.data(), sub.size());
}

bmin::String trim(std::string_view str) {
  const char* whitespace = " \n\r\t"; // Define whitespace characters

  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string_view::npos) {
    return ""; // String contains only whitespace or is empty
  }

  const auto strEnd = str.find_last_not_of(whitespace);
  // No need to check for npos for strEnd, as strBegin found something

  const std::string_view sub = str.substr(strBegin, strEnd - strBegin + 1);
  return bmin::String(sub.data(), sub.size());
}

void split(std::string_view str,
           std::string_view delimiter,
           bmin::DynArray<bmin::String>& out) {
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
      out.pushBack(bmin::String(&c, 1));
    }
    return;
  }

  size_t lastPos = 0;
  size_t findPos = 0;

  while ((findPos = str.find(delimiter, lastPos)) != std::string_view::npos) {
    const std::string_view part = str.substr(lastPos, findPos - lastPos);
    out.pushBack(bmin::String(part.data(), part.size()));
    lastPos = findPos + delimiter.length();
  }
  // Add the last part of the string (or the whole string if delimiter not
  // found)
  const std::string_view part = str.substr(lastPos);
  out.pushBack(bmin::String(part.data(), part.size()));
}

bool strEndsWith(std::string_view fullString, std::string_view ending) {
  if (fullString.length() >= ending.length()) {
    return fullString.compare(fullString.length() - ending.length(),
                              ending.length(),
                              ending) == 0;
  }
  return false;
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
            console.error(
                "[sdl2w console] Error loading filesystem from IndexedDB:",
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

void AssetLoader::loadPicture(std::string_view name, std::string_view path) {
  const bmin::String pathStr = toKey(path);
  SDL_Surface* loadedImage = IMG_Load(pathStr.cStr());

  if (loadedImage == nullptr) {
    LOG_LINE(ERROR) << "[sdl2w] ERROR Failed to load image: " << name << " ("
                    << path << ")" << Logger::endl;
    THROW_RUNTIME_ERROR(
        bmin::String(FAIL_ERROR_TEXT.data(), FAIL_ERROR_TEXT.size()));
  }
  picturePathToAlias[pathStr] = toKey(name);

  SDL_Texture* tex = draw.createTexture(loadedImage);
  store.storeTexture(name, tex);
  SDL_FreeSurface(loadedImage);
  loadSprite(name, tex, false);
}

void AssetLoader::loadSprite(std::string_view name,
                             SDL_Texture* tex,
                             bool flipped) {
  int width;
  int height;
  SDL_QueryTexture(tex, nullptr, nullptr, &width, &height);
  const bmin::String nameStr = toKey(name);
  store.storeSprite(
      name,
      new Sprite{nameStr,
                 Renderable{tex, nullptr},
                 0,
                 0,
                 width,
                 height,
                 width,
                 flipped});
}

void AssetLoader::loadSprite(std::string_view name,
                             SDL_Texture* tex,
                             int spritesheetWidth,
                             int x,
                             int y,
                             int w,
                             int h,
                             bool flipped) {
  const bmin::String nameStr = toKey(name);
  store.storeSprite(
      name,
      new Sprite{nameStr,
                 Renderable{tex, nullptr},
                 x,
                 y,
                 w,
                 h,
                 spritesheetWidth,
                 flipped});
}

void AssetLoader::loadSpriteSheet(std::string_view pictureName,
                                  std::string_view spriteName,
                                  int lastSpriteInd,
                                  int n,
                                  int w,
                                  int h) {
  const bmin::String pictureStr = toKey(pictureName);
  Sprite& sprite = store.getSprite(pictureStr.sliceView());

  int num_x = sprite.w / w;
  int ctr = 0;

  for (int i = lastSpriteInd; i < n; i++) {
    const bmin::String sprName =
        toKey(spriteName) + "_" + bmin::toString(ctr);

    spriteNameToPictureAlias[sprName] = pictureStr;
    loadSprite(sprName.cStr(),
               sprite.renderable.tex,
               sprite.w,
               (i % num_x) * w,
               (i / num_x) * h,
               w,
               h,
               false);
    ctr++;
  }
}

void AssetLoader::loadAnimationDefinition(std::string_view name, bool loop) {
  (void)name;
  (void)loop;
}

void AssetLoader::loadSpriteAssetsFromFile(std::string_view path) {
  const bmin::String fullPath = assetsPath(path);
  LOG(DEBUG) << "[sdl2w] Loading sprite assets from file " << fullPath
             << Logger::endl;
  std::ifstream file(fullPath.cStr());
  if (!file.is_open()) {
    LOG_LINE(ERROR) << "[sdl2w] Failed to open file: " << fullPath
                    << Logger::endl;
    return;
  }
  bmin::String lastPicture;
  int lastSpriteInd = 0;
  bmin::String line;
  try {
    while (readLine(file, line)) {
      const bmin::String trimmed = trim(line.sliceView());
      if (trimmed.empty()) {
        continue;
      }
      bmin::DynArray<bmin::String> arr;
      split(std::string_view(trimmed.cStr(), trimmed.size()), ",", arr);
      if (arr.empty()) {
        continue;
      }
      if (arr[0] == "-") {
        lastSpriteInd = 0;
        continue;
      }
      if (arr[0] == "Picture") {
        // LOG_LINE(DEBUG) << "Loading picture: " << arr[1] << Logger::endl;
        lastPicture = arr[1];
        lastSpriteInd = 0;
        loadPicture(arr[1].cStr(), arr[2].cStr());
      } else if (arr[0] == "SpriteList") {
        // LOG_LINE(DEBUG) << "Loading spriteList: " << arr[1] <<
        // Logger::endl;
        const bmin::String& name = arr[1];
        int nVal = 0;
        int wVal = 0;
        int hVal = 0;
        arr[2].parseInt(nVal);
        arr[3].parseInt(wVal);
        arr[4].parseInt(hVal);
        const int n = nVal + lastSpriteInd;
        loadSpriteSheet(lastPicture.cStr(), name.cStr(), lastSpriteInd, n,
                        wVal, hVal);
        lastSpriteInd = n;
      } else if (arr[0] == "Sprite") {
        // NOTE: Deprecated single sprites do not support flipping
        // LOG_LINE(DEBUG)
        //     << "Loading single sprite: " << arr[1] << Logger::endl;
        Sprite& spriteImage = store.getSprite(lastPicture.sliceView());

        const bmin::String& name = arr[1];
        int x = 0;
        int y = 0;
        int wVal = 0;
        int hVal = 0;
        arr[2].parseInt(x);
        arr[3].parseInt(y);
        arr[4].parseInt(wVal);
        arr[5].parseInt(hVal);
        spriteNameToPictureAlias[name] = lastPicture;
        loadSprite(name.cStr(),
                   spriteImage.renderable.tex,
                   spriteImage.w,
                   x,
                   y,
                   wVal,
                   hVal,
                   false);
      }
    }
    file.close();
  } catch (std::exception& e) {
    LOG_LINE(ERROR) << "[sdl2w] Failed to parse sprites res file: " << e.what()
                    << Logger::endl;
  }
}

void AssetLoader::loadAnimationAssetsFromFile(std::string_view path) {
  const bmin::String fullPath = assetsPath(path);
  LOG(DEBUG) << "[sdl2w] Loading anim assets from file " << fullPath
             << Logger::endl;
  try {
    bmin::String animName;
    bmin::String line;

    std::ifstream file(fullPath.cStr());

    int lineOffset = 0;

    while (readLine(file, line)) {
      if (line[0] == '#') {
        lineOffset = 1;
      } else if (line.size() > 1) {
        if (lineOffset == 1) {
          animName = line;
          lineOffset = 2;
        } else if (lineOffset == 2) {
          const bmin::String loop = line;
          store.storeAnimationDefinition(animName.cStr(),
                                         (loop == "loop" ? true : false));
          lineOffset = 3;
        } else {
          AnimationDefinition& anim =
              store.getAnimationDefinition(animName.sliceView());
          const bmin::String trimmed = trim(line.sliceView());
          const size_t sp = trimmed.find(" ");
          bmin::String strName;
          bmin::String strFrames;
          if (sp != bmin::String::npos) {
            strName = trimmed.substr(0, sp);
            strFrames = trim(trimmed.substr(sp + 1).sliceView());
          }
          int frames = 0;

          try {
            if (!strFrames.parseInt(frames)) {
              throw std::invalid_argument("invalid frame count");
            }
          } catch (std::exception&) {
            LOG_LINE(ERROR)
                << "[sdl2w] Failed to load anim sprite for: " << animName
                << Logger::endl;
            LOG_LINE(ERROR) << " FROM: '" << line << "'" << Logger::endl;
          }

          anim.addSprite(strName.cStr(), frames);
        }
      }
    }
    file.close();
  } catch (std::exception& e) {
    LOG_LINE(ERROR) << "[sdl2w] Failed to parse anim res file: " << e.what()
                    << Logger::endl;
  }
}

void AssetLoader::loadSoundAssetsFromFile(std::string_view path) {
  const bmin::String fullPath = assetsPath(path);
  LOG(DEBUG) << "[sdl2w] Loading sound assets from file " << fullPath
             << Logger::endl;
  std::ifstream file(fullPath.cStr());
  if (!file.is_open()) {
    LOG_LINE(ERROR) << "[sdl2w] Failed to open file: " << fullPath
                    << Logger::endl;
    return;
  }
  bmin::String line;
  try {
    while (readLine(file, line)) {
      const bmin::String trimmed = trim(line.sliceView());
      if (trimmed.size()) {
        bmin::DynArray<bmin::String> arr;
        split(std::string_view(trimmed.cStr(), trimmed.size()), ",", arr);
        if (arr.size() >= 3) {
          if (arr[0] == "Sound") {
            store.storeSound(arr[1].cStr(), arr[2].cStr());
          } else if (arr[0] == "Music") {
            store.storeMusic(arr[1].cStr(), arr[2].cStr());
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

void AssetLoader::loadAssetFile(std::string_view path) {
  const bmin::String fullPath = assetsPath(path);
  LOG(DEBUG) << "[sdl2w] Loading asset file " << fullPath << Logger::endl;
  std::ifstream file(fullPath.cStr());
  if (!file.is_open()) {
    LOG_LINE(ERROR) << "[sdl2w] Failed to open file: " << fullPath
                    << Logger::endl;
    return;
  }

  bmin::String line;
  bmin::Map<bmin::String, int>
      nextSpriteIndexForPicture; // Tracks the next sprite index for a given
                                 // picture alias

  bmin::String currentAnimationName;
  bool parsingAnimationFrames = false;

  try {
    while (readLine(file, line)) {
      const bmin::String trimmed = trim(line.sliceView());
      if (trimmed.empty() || trimmed[0] == '#') { // Skip comments and empty lines
        continue;
      }

      if (parsingAnimationFrames) {
        if (trimmed == "EndAnim") {
          parsingAnimationFrames = false;
          currentAnimationName = "";
          continue;
        }
        // Parse animation frame line: <sprite name> <ms>
        const size_t sp = trimmed.find(" ");
        bmin::String spriteNameStr;
        bmin::String framesStr;
        if (sp != bmin::String::npos) {
          spriteNameStr = trimmed.substr(0, sp);
          framesStr = trim(trimmed.substr(sp + 1).sliceView());
        }

        if (!spriteNameStr.empty() && !framesStr.empty() &&
            !currentAnimationName.empty()) {
          try {
            int frames = 0;
            if (!framesStr.parseInt(frames)) {
              throw std::invalid_argument("invalid frame count");
            }
            AnimationDefinition& animDef =
                store.getAnimationDefinition(currentAnimationName.sliceView());
            animDef.addSprite(spriteNameStr.cStr(), frames);
          } catch (const std::exception& e) {
            LOG_LINE(ERROR) << "[sdl2w] Failed to parse animation frame for "
                            << currentAnimationName << ": '" << trimmed << "' - "
                            << e.what() << Logger::endl;
          }
        } else {
          LOG(WARN) << "[sdl2w] Malformed or incomplete animation frame line: '"
                    << trimmed << "' for animation '" << currentAnimationName
                    << "'" << Logger::endl;
        }
        continue;
      }

      bmin::DynArray<bmin::String> tokens;
      split(std::string_view(trimmed.cStr(), trimmed.size()), ",", tokens);

      if (tokens.empty()) {
        continue;
      }

      const bmin::String command = trim(
          std::string_view(tokens[0].cStr(), tokens[0].size())); // Trim the command itself

      if (command == "Pic") {
        if (tokens.size() >= 3) {
          const bmin::String alias = trim(
              std::string_view(tokens[1].cStr(), tokens[1].size()));
          const bmin::String picPath = trim(
              std::string_view(tokens[2].cStr(), tokens[2].size()));
          loadPicture(
              alias.cStr(), picPath.cStr()); // Path is from executable as per assets.txt spec
          nextSpriteIndexForPicture[alias] =
              0; // Initialize sprite counter for this picture
        } else {
          LOG(WARN) << "[sdl2w] Malformed Pic asset specified: " << trimmed
                    << Logger::endl;
        }
      } else if (command == "Sprites") {
        if (tokens.size() >= 5) {
          const bmin::String picName = trim(
              std::string_view(tokens[1].cStr(), tokens[1].size()));
          try {
            int numSprites = 0;
            int spriteWidth = 0;
            int spriteHeight = 0;
            const bmin::String numStr = trim(
                std::string_view(tokens[2].cStr(), tokens[2].size()));
            const bmin::String widthStr = trim(
                std::string_view(tokens[3].cStr(), tokens[3].size()));
            const bmin::String heightStr = trim(
                std::string_view(tokens[4].cStr(), tokens[4].size()));
            if (!numStr.parseInt(numSprites) ||
                !widthStr.parseInt(spriteWidth) ||
                !heightStr.parseInt(spriteHeight)) {
              throw std::invalid_argument("invalid number in Sprites line");
            }

            if (!nextSpriteIndexForPicture.contains(picName)) {
              LOG(WARN)
                  << "[sdl2w] Sprites command for picture '" << picName
                  << "' encountered without a preceding 'Pic' command for it. "
                     "Assuming sprite index starts at 0."
                  << Logger::endl;
              nextSpriteIndexForPicture[picName] = 0;
            }

            const int startIndex = nextSpriteIndexForPicture[picName];
            // The spriteBaseName for loadSpriteSheet should be picName, so
            // sprites are picName_0, picName_1 etc.
            loadSpriteSheet(picName.cStr(),
                            picName.cStr(),
                            startIndex,
                            startIndex + numSprites,
                            spriteWidth,
                            spriteHeight);
            nextSpriteIndexForPicture[picName] = startIndex + numSprites;
          } catch (const std::invalid_argument& ia) {
            LOG_LINE(ERROR)
                << "[sdl2w] Invalid number in Sprites asset specified: "
                << trimmed << " - " << ia.what() << Logger::endl;
          } catch (const std::out_of_range& oor) {
            LOG_LINE(ERROR)
                << "[sdl2w] Number out of range in Sprites asset specified: "
                << trimmed << " - " << oor.what() << Logger::endl;
          }
        } else {
          LOG(WARN) << "[sdl2w] Malformed Sprites asset specified: " << trimmed
                    << Logger::endl;
        }
      } else if (command == "Anim") {
        if (tokens.size() >= 3) {
          currentAnimationName = trim(
              std::string_view(tokens[1].cStr(), tokens[1].size()));
          const bmin::String loopStr = trim(
              std::string_view(tokens[2].cStr(), tokens[2].size()));
          const bool loop = (loopStr == "loop");
          store.storeAnimationDefinition(currentAnimationName.cStr(), loop);
          parsingAnimationFrames = true;
        } else {
          LOG(WARN) << "[sdl2w] Malformed Anim asset specified: " << trimmed
                    << Logger::endl;
        }
      } else if (command == "Sound") {
        if (tokens.size() >= 3) {
          const bmin::String alias = trim(
              std::string_view(tokens[1].cStr(), tokens[1].size()));
          const bmin::String soundPath = trim(
              std::string_view(tokens[2].cStr(), tokens[2].size()));
          store.storeSound(alias.cStr(), soundPath.cStr());
        } else {
          LOG(WARN) << "[sdl2w] Malformed Sound asset specified: " << trimmed
                    << Logger::endl;
        }
      } else if (command == "Music") {
        if (tokens.size() >= 3) {
          const bmin::String alias = trim(
              std::string_view(tokens[1].cStr(), tokens[1].size()));
          const bmin::String musicPath = trim(
              std::string_view(tokens[2].cStr(), tokens[2].size()));
          store.storeMusic(alias.cStr(), musicPath.cStr());
        } else {
          LOG(WARN) << "[sdl2w] Malformed Music asset specified: " << trimmed
                    << Logger::endl;
        }
      } else {
        LOG(WARN) << "[sdl2w] Unknown command in asset file: '" << command
                  << "' in line: '" << trimmed << "'" << Logger::endl;
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
                                     std::string_view path) {
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

bmin::String loadFileAsString(std::string_view path) {
  const bmin::String pathStr = toKey(path);
#ifdef __EMSCRIPTEN__
  // Use a path relative to the mounted filesystem
  const bmin::String fullPath = bmin::String("/sdl2wdata/") + pathStr;

  // Check if file exists
  if (EM_ASM_INT(
          {
            try {
              return FS.stat(UTF8ToString($0)) != undefined;
            } catch (e) {
              return 0;
            }
          },
          fullPath.cStr())) {
    // File exists, read it
    std::ifstream file(fullPath.cStr());
    if (!file) {
      LOG_LINE(ERROR) << "[sdl2w] Error opening file: " << fullPath
                      << Logger::endl;
      return "";
    }
    return readStreamAsString(file);
  } else {
    LOG_LINE(ERROR) << "[sdl2w] File not found: " << fullPath << Logger::endl;
    return "";
  }
#else
  const bmin::String fullPath = assetsPath(path);
  LOG(DEBUG) << "[sdl2w] Loading file " << fullPath << Logger::endl;
  std::ifstream file(fullPath.cStr());

  if (!file) {
    LOG_LINE(ERROR) << "[sdl2w] Error opening file: " << pathStr << Logger::endl;
    THROW_RUNTIME_ERROR(
        bmin::String(FAIL_ERROR_TEXT.data(), FAIL_ERROR_TEXT.size()));
  }
  return readStreamAsString(file);
#endif
}

void saveFileAsString(std::string_view path, std::string_view content) {
#ifdef __EMSCRIPTEN__
  bmin::StringStream transformPath;
  transformPath << "/" << "sdl2wdata" << "/" << path;
  const bmin::String savePath = transformPath.str();
  LOG(DEBUG) << "[sdl2w] Saving file " << savePath << Logger::endl;
  std::ofstream file(savePath.cStr());
#else
  const bmin::String fullPath = assetsPath(path);
  LOG(DEBUG) << "[sdl2w] Saving file " << fullPath << Logger::endl;
  std::ofstream file(fullPath.cStr());
#endif

  if (!file) {

#ifdef __EMSCRIPTEN__
    LOG_LINE(ERROR) << "[sdl2w] Error opening file for save: " << savePath
                    << Logger::endl;
    // HACK emscripten is set to not catch errors
    return;
#else
    LOG_LINE(ERROR) << "[sdl2w] Error opening file for save: " << toKey(path)
                    << Logger::endl;
    // HACK emscripten is set to not catch errors
    THROW_RUNTIME_ERROR(
        bmin::String(FAIL_ERROR_TEXT.data(), FAIL_ERROR_TEXT.size()));
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
