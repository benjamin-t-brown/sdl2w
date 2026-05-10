// This is a helper program for managing Sprite and Animation assets.  It shows
// what SDL2W parsed from the asset file and shows every animation and sprite
// that was loaded.

#include "../lib/Animation.h"
#include "../lib/AssetLoader.h"
#include "../lib/Defines.h"
#include "../lib/Draw.h"
#include "../lib/Logger.h"
#include "../lib/Window.h"
#include <SDL2/SDL_rect.h>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

using namespace sdl2w;

enum UiState { UI_SELECT_ASSET, UI_SHOW_ANIMS };
enum AssetBrowserTab { TAB_PICTURES, TAB_SOUNDS };

struct State {
  std::vector<std::string> pictures;
  std::vector<std::string> filteredPictures;
  std::vector<std::string> sounds;
  std::vector<std::string> filteredSounds;
  std::string filter;
  std::string selectedPicturePath;
  std::string selectedSoundName;
  std::vector<std::string> selectedSpriteNames;
  std::vector<AnimationDefinition> selectedAnimDefinitions;
  std::vector<std::string> selectedAnimNames;
  std::optional<Animation> selectedAnim;
  std::string selectedSpriteName;
  double scale = 2.;
  AssetBrowserTab assetBrowserTab = TAB_PICTURES;
  UiState uiState = UI_SELECT_ASSET;
};

struct AssetLoadConfig {
  std::string assetFilePath;
  std::string deprecatedSpritesFilePath;
  std::string deprecatedAnimationsFilePath;
  std::string deprecatedSoundsFilePath;
};

void reloadAssets(AssetLoader& assetLoader,
                  Store& store,
                  const AssetLoadConfig& assetLoadConfig) {
  store.clear();
  store.loadAndStoreFont(
      "default",
      "assets/monofonto.ttf"); // Assuming monofonto.ttf is still at a fixed
                               // relative path or handled differently
  assetLoader.picturePathToAlias.clear();
  assetLoader.spriteNameToPictureAlias.clear();

  if (!assetLoadConfig.assetFilePath.empty()) {
    assetLoader.loadAssetsFromFile(sdl2w::ASSET_FILE,
                                   assetLoadConfig.assetFilePath);
    return;
  }

  bool loadedAnyDeprecatedFile = false;
  if (!assetLoadConfig.deprecatedSpritesFilePath.empty()) {
    assetLoader.loadAssetsFromFile(sdl2w::DEPRECATED_ASSET_TYPE_SPRITE,
                                   assetLoadConfig.deprecatedSpritesFilePath);
    loadedAnyDeprecatedFile = true;
  }
  if (!assetLoadConfig.deprecatedAnimationsFilePath.empty()) {
    assetLoader.loadAssetsFromFile(
        sdl2w::DEPRECATED_ASSET_TYPE_ANIMATION,
        assetLoadConfig.deprecatedAnimationsFilePath);
    loadedAnyDeprecatedFile = true;
  }
  if (!assetLoadConfig.deprecatedSoundsFilePath.empty()) {
    assetLoader.loadAssetsFromFile(sdl2w::DEPRECATED_ASSET_TYPE_SOUND,
                                   assetLoadConfig.deprecatedSoundsFilePath);
    loadedAnyDeprecatedFile = true;
  }

  if (!loadedAnyDeprecatedFile) {
    throw std::runtime_error(
        "No assets configured. Provide --asset-file <path> for unified asset "
        "format, or use deprecated file args like --sprites-file/--anims-file."
    );
  }
}

std::vector<std::string> findFilesRecursive(const std::string& path,
                                            const std::string& ext) {
  std::vector<std::string> files;
  const auto basePath = std::filesystem::path(path);
  const auto baseName = basePath.filename();
  for (const auto& entry :
       std::filesystem::recursive_directory_iterator(path)) {
    if (entry.is_regular_file() && entry.path().extension() == ext) {
      try {
        const auto rel = std::filesystem::relative(entry.path(), basePath);
        files.push_back((baseName / rel).generic_string());
      } catch (const std::exception&) {
        // Fallback: keep absolute path if relative conversion fails.
        files.push_back(entry.path().string());
      }
    }
  }
  return files;
}

std::string toLower(const std::string& s) {
  std::string lower = s;
  std::transform(lower.begin(),
                 lower.end(),
                 lower.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return lower;
}

int normalize(int x, int a, int b, int c, int d) {
  return c + ((x - a) * (d - c)) / (b - a);
}

int extractNumber(const std::string& s) {
  int i = static_cast<int>(s.size()) - 1;
  while (i >= 0 && std::isdigit(s[i])) {
    i--;
  }
  if (i != static_cast<int>(s.size()) - 1) {
    return std::stoi(s.substr(i + 1));
  }
  return 0;
}

bool naturalComparator(const std::string& a, const std::string& b) {
  size_t posA = a.find_last_not_of("0123456789");
  size_t posB = b.find_last_not_of("0123456789");
  std::string prefixA = (posA != std::string::npos) ? a.substr(0, posA + 1) : a;
  std::string prefixB = (posB != std::string::npos) ? b.substr(0, posB + 1) : b;
  if (prefixA == prefixB) {
    return extractNumber(a) < extractNumber(b);
  }
  return prefixA < prefixB;
}

void showAnimListScreen(State& state) {
  state.uiState = UI_SELECT_ASSET;
  state.selectedPicturePath.clear();
  state.selectedSpriteNames.clear();
  state.selectedAnimDefinitions.clear();
  state.selectedAnim.reset();
  state.selectedSpriteName.clear();
}

class Button {
public:
  SDL_Rect bounds;
  std::string text;
  SDL_Color bgColor = {50, 50, 50, 255};

  Button(int x, int y, int w, int h, const std::string& text)
      : bounds{x, y, w, h}, text(text) {}

  void
  handleMousedown(int x, int y, std::function<void(const std::string&)> cb) {
    SDL_Point p{x, y};
    if (SDL_PointInRect(&p, &bounds)) {
      if (cb) {
        cb(text);
      }
    }
  }

  void render(Draw& d) {
    d.drawRect(bounds.x, bounds.y, bounds.w, bounds.h, bgColor);
    d.drawText(text,
               sdl2w::RenderTextParams{
                   .fontName = "default",
                   .fontSize = sdl2w::TEXT_SIZE_28,
                   .x = bounds.x + 4,
                   .y = bounds.y,
                   .color = {255, 255, 255},
               });
  }
};

class ScrollableStringList {
public:
  int linesPerScreen;
  int currentPage = 0;
  int lineHeight = 32;
  int highlightInd = -1;
  int numScreens = 0;
  std::string focusValue;

  Button scrollUpButton;
  Button scrollDownButton;
  SDL_Rect bounds;

  std::vector<std::pair<int, SDL_Rect>> boundingBoxes;
  std::vector<std::string>* lines = nullptr;

  ScrollableStringList(int x, int y, int w, int h, int lineHeight)
      : scrollUpButton(x + w - 36, y, 36, 36, "/\\"),
        scrollDownButton(x + w - 36, y + h - 36, 36, 36, "\\/") {
    linesPerScreen = h / lineHeight - 1;
    bounds = {x, y, w, h};
    this->lineHeight = lineHeight;
  }

  void offsetPage(int offset) {
    currentPage += offset;
    if (currentPage < 0) {
      currentPage = 0;
    } else if (currentPage >= numScreens) {
      currentPage = numScreens - 1;
    }
  }

  void handleMouseWheel(int x, int y, int wheelY) {
    SDL_Point p{x, y};
    if (SDL_PointInRect(&p, &bounds)) {
      if (wheelY > 0) {
        offsetPage(-1);
      } else if (wheelY < 0) {
        offsetPage(1);
      }
    }
  }

  void handleMouseMove(int x, int y) {
    SDL_Point p{x, y};
    for (int i = 0; i < static_cast<int>(boundingBoxes.size()); i++) {
      auto [boxInd, boundingBox] = boundingBoxes[i];
      if (SDL_PointInRect(&p, &boundingBox)) {
        highlightInd = boxInd;
        return;
      }
    }
    highlightInd = -1;
  }

  void
  handleMouseDown(int x, int y, std::function<void(const std::string&)> cb) {
    SDL_Point p{x, y};

    scrollUpButton.handleMousedown(
        x, y, [&](const std::string&) { offsetPage(-1); });
    scrollDownButton.handleMousedown(
        x, y, [&](const std::string&) { offsetPage(1); });

    for (int i = 0; i < static_cast<int>(boundingBoxes.size()); i++) {
      auto [boxInd, boundingBox] = boundingBoxes[i];
      if (SDL_PointInRect(&p, &boundingBox)) {
        if (cb) {
          cb((*lines)[boxInd]);
        }
      }
    }
  }

  void render(Draw& d, std::vector<std::string>& lines) {
    this->lines = &lines;
    d.drawRect(bounds.x, bounds.y, bounds.w, bounds.h, SDL_Color{0, 0, 0, 255});

    boundingBoxes.clear();
    numScreens = (static_cast<int>(lines.size()) / linesPerScreen) + 1;

    scrollUpButton.render(d);
    scrollDownButton.render(d);

    int yPos = normalize(currentPage,
                         0,
                         std::max(1, numScreens - 1),
                         scrollUpButton.bounds.y + scrollUpButton.bounds.h,
                         scrollDownButton.bounds.y - scrollDownButton.bounds.h);
    SDL_Rect scrollPosition = {scrollUpButton.bounds.x, yPos, 36, 36};
    d.drawRect(scrollPosition.x,
               scrollPosition.y,
               scrollPosition.w,
               scrollPosition.h,
               SDL_Color{150, 150, 150, 255});

    for (int i = 0;
         (i + currentPage * linesPerScreen) < static_cast<int>(lines.size());
         i++) {
      if (i > linesPerScreen) {
        break;
      }
      const int lineInd = i + currentPage * linesPerScreen;
      const auto& str = lines[lineInd];
      SDL_Rect boundingBox = {
          bounds.x, bounds.y + i * 32, bounds.w - 36, lineHeight};
      boundingBoxes.push_back({lineInd, boundingBox});
      d.drawText(
          str,
          sdl2w::RenderTextParams{
              .fontName = "default",
              .fontSize = sdl2w::TEXT_SIZE_28,
              .x = boundingBox.x + 4,
              .y = boundingBox.y,
              .color = highlightInd == lineInd
                           ? SDL_Color{100, 100, 255}
                           : (focusValue == str ? SDL_Color{100, 255, 100}
                                                : SDL_Color{255, 255, 255}),
          });
    }
  }
};

std::vector<std::string>
getSpriteNamesForPicture(AssetLoader& assetLoader,
                         const std::string& picturePath,
                         const std::string& assetsDirPath) {
  std::vector<std::string> spriteNames;
  const auto selectedPath = std::filesystem::path(picturePath);
  const std::string selectedFileName = selectedPath.filename().string();
  std::string pictureAlias;
  std::string selectedAssetRelativeNorm;

  auto normalizePath = [](const std::filesystem::path& p) {
    return toLower(p.lexically_normal().generic_string());
  };
  const std::string selectedNorm = normalizePath(selectedPath);

  try {
    const auto assetsDirAbs = std::filesystem::absolute(assetsDirPath);
    const auto selectedAbs = std::filesystem::absolute(selectedPath);
    const auto relToAssets = std::filesystem::relative(selectedAbs, assetsDirAbs);
    const std::string relNorm = normalizePath(relToAssets);
    if (!(relNorm.size() >= 2 && relNorm[0] == '.' && relNorm[1] == '.')) {
      selectedAssetRelativeNorm =
          normalizePath(std::filesystem::path("assets") / relToAssets);
    }
  } catch (const std::exception&) {
    // Best effort only; continue with other matching strategies.
  }

  for (const auto& [loadedPathStr, loadedAlias] :
       assetLoader.picturePathToAlias) {
    const auto loadedPath = std::filesystem::path(loadedPathStr);
    const std::string loadedNorm = normalizePath(loadedPath);

    if (!selectedAssetRelativeNorm.empty() &&
        loadedNorm == selectedAssetRelativeNorm) {
      pictureAlias = loadedAlias;
      break;
    }

    if (loadedNorm == selectedNorm) {
      pictureAlias = loadedAlias;
      break;
    }

    // Deprecated assets usually store relative paths (assets/foo.png) while the
    // UI list provides absolute paths; match by filename and, when available,
    // by canonical path equality.
    if (loadedPath.filename().string() == selectedFileName) {
      pictureAlias = loadedAlias;
      break;
    }

    try {
      if (std::filesystem::exists(selectedPath) &&
          std::filesystem::exists(loadedPath) &&
          std::filesystem::equivalent(selectedPath, loadedPath)) {
        pictureAlias = loadedAlias;
        break;
      }
    } catch (const std::exception&) {
      // Ignore path resolution errors and continue trying other entries.
    }
  }

  if (pictureAlias.empty()) {
    LOG(INFO) << "No picture alias found for selected path: " << picturePath
              << LOG_ENDL;
    return spriteNames;
  }

  for (const auto& [spriteName, spritePictureAlias] :
       assetLoader.spriteNameToPictureAlias) {
    if (spritePictureAlias == pictureAlias) {
      spriteNames.push_back(spriteName);
    }
  }
  std::sort(spriteNames.begin(), spriteNames.end(), naturalComparator);
  return spriteNames;
}

std::vector<AnimationDefinition> getAnimationDefinitionsFromSpriteNames(
    Store& store, const std::vector<std::string>& spriteNames) {
  std::vector<AnimationDefinition> animationDefinitions;

  for (const auto& anim : store.anims) {
    const auto& animDef = *anim.second;
    for (const auto& spriteName : spriteNames) {
      auto it = std::find_if(animDef.sprites.begin(),
                             animDef.sprites.end(),
                             [&](const AnimSpriteDefinition& sprite) {
                               return sprite.name == spriteName;
                             });

      if (it != animDef.sprites.end()) {
        animationDefinitions.push_back(animDef);
        break;
      }
    }
  }
  return animationDefinitions;
}

//------------------------------------------------------------------------

void runProgram(int argc,
                char** argv,
                const std::string& assetsDirPath,
                const AssetLoadConfig& assetLoadConfig) {
  const int w = 1024;
  const int h = 768;

  State state;
  ScrollableStringList assetList(4, 50, w - 8, h - 50, 32);
  ScrollableStringList animList(4, 50, w / 2, h / 2 - 50, 32);
  ScrollableStringList spriteList(4, h / 2 + 50, w / 2, h / 2 - 50, 32);
  Button picturesTabButton = Button(4, 4, 130, 36, "Pictures");
  Button soundsTabButton = Button(140, 4, 110, 36, "Sounds");
  Button backButton = Button(w - 48, 4, 48, 36, " X");
  Button reloadButton = Button(w - 48 - 110, 4, 100, 36, "Reload");
  Button playButton = Button(w - 48 - 220, 4, 100, 36, "Play");
  Button X1Button = Button(10 + w / 2, h / 2, 48, 36, "X1");
  Button X2Button = Button(10 + w / 2 + 52, h / 2, 48, 36, "X2");
  Button X3Button = Button(10 + w / 2 + 52 * 2, h / 2, 48, 36, "X3");
  Button X4Button = Button(10 + w / 2 + 52 * 3, h / 2, 48, 36, "X4");
  Button X5Button = Button(10 + w / 2 + 52 * 4, h / 2, 48, 36, "X5");
  Button X6Button = Button(10 + w / 2 + 52 * 5, h / 2, 48, 36, "X6");
  Button X7Button = Button(10 + w / 2 + 52 * 6, h / 2, 48, 36, "X7");
  Button X8Button = Button(10 + w / 2 + 52 * 7, h / 2, 48, 36, "X8");

  std::string notifMessage = "";
  double notifTime = 0.;
  double notifDuration = 2000.;

  sdl2w::Store store;
  sdl2w::Window window(store,
                       {
                           .mode = sdl2w::DrawMode::CPU,
                           .title = "Anims - Asset Viewer",
                           .w = w,
                           .h = h,
                           .x = 25,
                           .y = 50,
                           .renderW = w,
                           .renderH = h,
                       });

  window.getDraw().setBackgroundColor({16, 30, 41});

  AssetLoader assetLoader(window.getDraw(), window.getStore());
  reloadAssets(assetLoader, store, assetLoadConfig);

  auto applyAssetFilter = [&]() {
    state.filteredPictures.clear();
    state.filteredSounds.clear();
    std::string filterLower = toLower(state.filter);

    for (const auto& picture : state.pictures) {
      if (toLower(picture).find(filterLower) != std::string::npos) {
        state.filteredPictures.push_back(picture);
      }
    }
    for (const auto& sound : state.sounds) {
      if (toLower(sound).find(filterLower) != std::string::npos) {
        state.filteredSounds.push_back(sound);
      }
    }
    assetList.currentPage = 0;
  };

  auto reloadAssetBrowserData = [&]() {
    state.pictures = findFilesRecursive(assetsDirPath, ".png");
    std::sort(state.pictures.begin(), state.pictures.end(), naturalComparator);

    state.sounds.clear();
    state.sounds.reserve(store.sounds.size());
    for (const auto& [soundName, sound] : store.sounds) {
      state.sounds.push_back(soundName);
    }
    std::sort(state.sounds.begin(), state.sounds.end(), naturalComparator);
    applyAssetFilter();
  };

  reloadAssetBrowserData();

  auto& d = window.getDraw();

  window.getEvents().setKeyboardEvent(
      sdl2w::ON_KEY_DOWN, [&](std::string_view keyName, int keyCode) {
        if (state.uiState == UI_SELECT_ASSET) {
          if (keyName == "Backspace") {
            if (!state.filter.empty()) {
              state.filter.pop_back();
            }
          } else if (isprint(keyName[0])) {
            state.filter += keyName;
          }
          applyAssetFilter();
        } else if (state.uiState == UI_SHOW_ANIMS) {
          if (keyName == "Escape") {
            animList.focusValue.clear();
            showAnimListScreen(state);
          }
          if (keyName == "Space") {
            if (state.selectedAnim.has_value()) {
              auto& anim = state.selectedAnim.value();
              if (anim.isInitialized()) {
                anim.start();
              }
            }
          }
        }
      });

  window.getEvents().setMouseEvent(
      sdl2w::ON_MOUSE_WHEEL, [&](int x, int y, int wheelY) {
        if (state.uiState == UI_SELECT_ASSET) {
          assetList.handleMouseWheel(x, y, wheelY);
        } else if (state.uiState == UI_SHOW_ANIMS) {
          animList.handleMouseWheel(x, y, wheelY);
          spriteList.handleMouseWheel(x, y, wheelY);
        }
      });

  window.getEvents().setMouseEvent(
      sdl2w::ON_MOUSE_DOWN, [&](int x, int y, int button) {
        if (state.uiState == UI_SELECT_ASSET) {
          picturesTabButton.handleMousedown(x, y, [&](const std::string&) {
            state.assetBrowserTab = TAB_PICTURES;
            assetList.focusValue = state.selectedPicturePath;
          });
          soundsTabButton.handleMousedown(x, y, [&](const std::string&) {
            state.assetBrowserTab = TAB_SOUNDS;
            assetList.focusValue = state.selectedSoundName;
          });
          reloadButton.handleMousedown(x, y, [&](const std::string&) {
            LOG(INFO) << "Reloading assets..." << LOG_ENDL;
            reloadAssets(assetLoader, store, assetLoadConfig);
            reloadAssetBrowserData();
            notifMessage = "Assets reloaded!";
            notifTime = 0;
          });
          assetList.handleMouseDown(x, y, [&](const std::string& str) {
            if (state.assetBrowserTab == TAB_PICTURES) {
              state.selectedPicturePath = str;
              state.selectedSpriteNames = getSpriteNamesForPicture(
                  assetLoader, state.selectedPicturePath, assetsDirPath);

              std::sort(state.selectedSpriteNames.begin(),
                        state.selectedSpriteNames.end(),
                        naturalComparator);
              state.selectedAnimDefinitions =
                  getAnimationDefinitionsFromSpriteNames(
                      store, state.selectedSpriteNames);
              state.selectedAnimNames.clear();
              for (const auto& animDef : state.selectedAnimDefinitions) {
                state.selectedAnimNames.push_back(animDef.name);
              }
              std::sort(state.selectedAnimNames.begin(),
                        state.selectedAnimNames.end(),
                        naturalComparator);
              state.uiState = UI_SHOW_ANIMS;
            } else {
              state.selectedSoundName = str;
              assetList.focusValue = str;
              window.playSound(str);
              notifMessage = "Playing sound: " + str;
              notifTime = 0;
            }
          });
        } else if (state.uiState == UI_SHOW_ANIMS) {
          backButton.handleMousedown(x, y, [&](const std::string&) {
            showAnimListScreen(state);
            animList.focusValue.clear();
          });
          X1Button.handleMousedown(
              x, y, [&](const std::string&) { state.scale = 1.; });
          X2Button.handleMousedown(
              x, y, [&](const std::string&) { state.scale = 2.; });
          X3Button.handleMousedown(
              x, y, [&](const std::string&) { state.scale = 3.; });
          X4Button.handleMousedown(
              x, y, [&](const std::string&) { state.scale = 4.; });
          X5Button.handleMousedown(
              x, y, [&](const std::string&) { state.scale = 5.; });
          X6Button.handleMousedown(
              x, y, [&](const std::string&) { state.scale = 6.; });
          X7Button.handleMousedown(
              x, y, [&](const std::string&) { state.scale = 7.; });
          X8Button.handleMousedown(
              x, y, [&](const std::string&) { state.scale = 8.; });
          playButton.handleMousedown(x, y, [&](const std::string&) {
            if (state.selectedAnim.has_value()) {
              auto& anim = state.selectedAnim.value();
              if (anim.isInitialized()) {
                anim.start();
              }
            }
          });
          reloadButton.handleMousedown(x, y, [&](const std::string&) {
            LOG(INFO) << "Reloading assets..." << LOG_ENDL;
            reloadAssets(assetLoader, store, assetLoadConfig);
            reloadAssetBrowserData();
            state.selectedAnimNames.clear();
            state.selectedAnimDefinitions.clear();

            const std::string selectedPicturePath = state.selectedPicturePath;
            state.selectedSpriteNames =
                getSpriteNamesForPicture(
                    assetLoader, selectedPicturePath, assetsDirPath);
            state.selectedAnimDefinitions =
                getAnimationDefinitionsFromSpriteNames(
                    store, state.selectedSpriteNames);
            for (const auto& animDef : state.selectedAnimDefinitions) {
              state.selectedAnimNames.push_back(animDef.name);
            }
            std::sort(state.selectedAnimNames.begin(),
                      state.selectedAnimNames.end());
            if (state.selectedAnim.has_value()) {
              try {
                state.selectedAnim =
                    store.createAnimation(state.selectedAnim.value().name);
              } catch (const std::exception& e) {
                LOG(WARN) << "Resetting animation which was not found: "
                          << e.what() << LOG_ENDL;
                state.selectedAnim.reset();
              }
            }
            if (state.selectedSpriteName.size() > 0) {
              try {
                store.getSprite(state.selectedSpriteName);
              } catch (const std::exception& e) {
                LOG(WARN) << "Resetting sprite which was not found: "
                          << e.what() << LOG_ENDL;
                state.selectedSpriteName.clear();
              }
            }
            notifMessage = "Assets reloaded!";
            notifTime = 0;
          });

          animList.handleMouseDown(x, y, [&](const std::string& str) {
            auto it = std::find_if(state.selectedAnimDefinitions.begin(),
                                   state.selectedAnimDefinitions.end(),
                                   [&](AnimationDefinition& animDef) {
                                     return animDef.name == str;
                                   });
            if (it != state.selectedAnimDefinitions.end()) {
              auto& animDef = *it;
              animList.focusValue = animDef.name;
              state.selectedAnim = store.createAnimation(animDef.name);
            }
          });
          spriteList.handleMouseDown(x, y, [&](const std::string& str) {
            auto it = std::find_if(state.selectedSpriteNames.begin(),
                                   state.selectedSpriteNames.end(),
                                   [&](const std::string& spriteName) {
                                     return spriteName == str;
                                   });
            if (it != state.selectedSpriteNames.end()) {
              spriteList.focusValue = str;
              state.selectedSpriteName = str;
              LOG(INFO) << "Selected sprite: " << str << LOG_ENDL;
            }
          });
        }
      });

  window.getEvents().setMouseEvent(
      sdl2w::ON_MOUSE_MOVE, [&](int x, int y, int button) {
        if (state.uiState == UI_SELECT_ASSET) {
          assetList.handleMouseMove(x, y);
        } else if (state.uiState == UI_SHOW_ANIMS) {
          animList.handleMouseMove(x, y);
          spriteList.handleMouseMove(x, y);
        }
      });

  window.startRenderLoop(
      []() { return true; },
      []() {},
      [&]() {
        if (state.uiState == UI_SELECT_ASSET) {
          picturesTabButton.bgColor = state.assetBrowserTab == TAB_PICTURES
                                          ? SDL_Color{70, 110, 180, 255}
                                          : SDL_Color{50, 50, 50, 255};
          soundsTabButton.bgColor = state.assetBrowserTab == TAB_SOUNDS
                                        ? SDL_Color{70, 110, 180, 255}
                                        : SDL_Color{50, 50, 50, 255};
          d.drawText(
              state.filter.size() > 0 ? state.filter : "<type for filter>",
              sdl2w::RenderTextParams{
                  .fontName = "default",
                  .fontSize = sdl2w::TEXT_SIZE_28,
                  .x = 4,
                  .y = 4,
                  .color = state.filter.size() ? SDL_Color{255, 255, 255}
                                               : SDL_Color{100, 100, 100},
                  .centered = false,
              });
          picturesTabButton.render(d);
          soundsTabButton.render(d);
          if (state.assetBrowserTab == TAB_PICTURES) {
            assetList.focusValue = state.selectedPicturePath;
            assetList.render(d, state.filteredPictures);
          } else {
            assetList.focusValue = state.selectedSoundName;
            assetList.render(d, state.filteredSounds);
          }
          reloadButton.render(d);

        } else if (state.uiState == UI_SHOW_ANIMS) {
          d.drawText("-> " + state.selectedPicturePath,
                     sdl2w::RenderTextParams{
                         .fontName = "default",
                         .fontSize = sdl2w::TEXT_SIZE_28,
                         .x = 4,
                         .y = 4,
                         .color = {100, 100, 255},
                     });

          if (state.selectedAnim.has_value()) {
            auto& anim = state.selectedAnim.value();
            anim.update(window.getDeltaTime());
            d.drawAnimation(anim,
                            sdl2w::RenderableParams{
                                .scale = {state.scale, state.scale},
                                .x = w / 2 + w / 4,
                                .y = h / 2 - h / 4,
                                .centered = true,
                            });
          }
          if (state.selectedSpriteName.size() > 0) {
            Sprite& sprite = store.getSprite(state.selectedSpriteName);
            d.drawSprite(sprite,
                         sdl2w::RenderableParams{
                             .scale = {state.scale, state.scale},
                             .x = w / 2 + w / 4,
                             .y = h / 2 + h / 4,
                             .centered = true,
                         });
          }

          backButton.render(d);
          playButton.render(d);
          reloadButton.render(d);

          animList.render(d, state.selectedAnimNames);
          spriteList.render(d, state.selectedSpriteNames);
          X1Button.render(d);
          X2Button.render(d);
          X3Button.render(d);
          X4Button.render(d);
          X5Button.render(d);
          X6Button.render(d);
          X7Button.render(d);
          X8Button.render(d);
        }

        if (notifMessage.size()) {
          notifTime += window.getDeltaTime();
          if (notifTime > notifDuration) {
            notifMessage = "";
          } else {
            d.drawText(notifMessage,
                       RenderTextParams{
                           .fontName = "default",
                           .fontSize = sdl2w::TEXT_SIZE_36,
                           .x = w / 2,
                           .y = 40,
                           .color = {100, 255, 100},
                           .centered = true,
                       });
          }
        }
        return true;
      });
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start program" << LOG_ENDL;

  std::string assetsDirPathStr;
  AssetLoadConfig assetLoadConfig;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--assets-dir" && i + 1 < argc) {
      assetsDirPathStr = argv[++i];
    } else if (arg == "--asset-file" && i + 1 < argc) {
      assetLoadConfig.assetFilePath = argv[++i];
    } else if (arg == "--sprites-file" && i + 1 < argc) {
      assetLoadConfig.deprecatedSpritesFilePath = argv[++i];
    } else if (arg == "--anims-file" && i + 1 < argc) {
      assetLoadConfig.deprecatedAnimationsFilePath = argv[++i];
    } else if (arg == "--sounds-file" && i + 1 < argc) {
      assetLoadConfig.deprecatedSoundsFilePath = argv[++i];
    }
  }

  if (assetsDirPathStr.empty()) {
    std::cerr << "Error: --assets-dir <path> is a required argument."
              << std::endl;
    std::cerr << "Usage: " << argv[0]
              << " --assets-dir <path> [--asset-file <path>] [--sprites-file "
                 "<path>] [--anims-file <path>] [--sounds-file <path>]"
              << std::endl;
    return 1;
  }
  // Back-compat: if --asset-file points to deprecated anims.txt, treat it as
  // deprecated mode and auto-wire sprites/sounds from same directory.
  if (!assetLoadConfig.assetFilePath.empty()) {
    const auto assetPath = std::filesystem::path(assetLoadConfig.assetFilePath);
    const std::string filename = assetPath.filename().string();
    if (filename == "anims.txt") {
      const auto assetDir = assetPath.parent_path();
      if (assetLoadConfig.deprecatedAnimationsFilePath.empty()) {
        assetLoadConfig.deprecatedAnimationsFilePath = assetPath.string();
      }
      if (assetLoadConfig.deprecatedSpritesFilePath.empty()) {
        const auto spritesPath = assetDir / "sprites.txt";
        if (std::filesystem::exists(spritesPath)) {
          assetLoadConfig.deprecatedSpritesFilePath = spritesPath.string();
        }
      }
      if (assetLoadConfig.deprecatedSoundsFilePath.empty()) {
        const auto soundsPath = assetDir / "sounds.txt";
        if (std::filesystem::exists(soundsPath)) {
          assetLoadConfig.deprecatedSoundsFilePath = soundsPath.string();
        }
      }
      assetLoadConfig.assetFilePath.clear();
    }
  }

  // Default deprecated file locations from assets dir when not explicitly set.
  if (assetLoadConfig.assetFilePath.empty()) {
    if (assetLoadConfig.deprecatedSpritesFilePath.empty()) {
      const auto spritesPath =
          std::filesystem::path(assetsDirPathStr) / "sprites.txt";
      if (std::filesystem::exists(spritesPath)) {
        assetLoadConfig.deprecatedSpritesFilePath = spritesPath.string();
      }
    }
    if (assetLoadConfig.deprecatedAnimationsFilePath.empty()) {
      const auto animsPath =
          std::filesystem::path(assetsDirPathStr) / "anims.txt";
      if (std::filesystem::exists(animsPath)) {
        assetLoadConfig.deprecatedAnimationsFilePath = animsPath.string();
      }
    }
    if (assetLoadConfig.deprecatedSoundsFilePath.empty()) {
      const auto soundsPath =
          std::filesystem::path(assetsDirPathStr) / "sounds.txt";
      if (std::filesystem::exists(soundsPath)) {
        assetLoadConfig.deprecatedSoundsFilePath = soundsPath.string();
      }
    }
  }

  const bool hasUnifiedAssetFile = !assetLoadConfig.assetFilePath.empty();
  const bool hasDeprecatedAssetFiles =
      !assetLoadConfig.deprecatedSpritesFilePath.empty() ||
      !assetLoadConfig.deprecatedAnimationsFilePath.empty() ||
      !assetLoadConfig.deprecatedSoundsFilePath.empty();
  if (!hasUnifiedAssetFile && !hasDeprecatedAssetFiles) {
    std::cerr
        << "Error: no asset files configured. Provide either --asset-file "
           "<path> (unified format), or deprecated args: --sprites-file "
           "<path> --anims-file <path> --sounds-file <path>."
        << std::endl;
    std::cerr << "Usage: " << argv[0]
              << " --assets-dir <path> [--asset-file <path>] [--sprites-file "
                 "<path>] [--anims-file <path>] [--sounds-file <path>]"
              << std::endl;
    return 1;
  }

  // You might want to convert to absolute paths or ensure they exist here
  // For example:
  // fs::path assetsDir = fs::absolute(fs::path(assetsDirPathStr));
  // fs::path assetFile = fs::absolute(fs::path(assetFilePathStr));
  // if (!fs::exists(assetsDir) || !fs::is_directory(assetsDir)) { ... }
  // if (!fs::exists(assetFile) || !fs::is_regular_file(assetFile)) { ... }

  sdl2w::Window::init();
  srand(time(NULL));

  runProgram(argc, argv, assetsDirPathStr, assetLoadConfig);

  sdl2w::Window::unInit();
  LOG(INFO) << "End program" << LOG_ENDL;

  return 0;
}