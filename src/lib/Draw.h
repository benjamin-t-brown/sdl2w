#pragma once

#include "Animation.h"
#include "Defines.h"
#include "bmin/Map.h"
#include "bmin/String.h"
#include <cstdint>
#include <string_view>
#include <utility>

#if __has_include(<SDL2/SDL_pixels.h>) && __has_include(<SDL2/SDL_stdinc.h>)
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_stdinc.h>
#elif __has_include(<SDL_pixels.h>) && __has_include(<SDL_stdinc.h>)
#include <SDL_pixels.h>
#include <SDL_stdinc.h>
#else
#error                                                                         \
    "Could not find SDL pixel/stdinc headers in either SDL2/ or root include paths"
#endif

namespace sdl2w {

class Store;

struct RenderableParamsEx {
  std::pair<double, double> scale = {0., 0.};
  double angleDeg = 0.;
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  int clipX = 0;
  int clipY = 0;
  int clipW = 0;
  int clipH = 0;
  bool centered = true;
  bool flipped = false;
};

struct RenderableParams {
  std::pair<double, double> scale = {0., 0.};
  int x = 0;
  int y = 0;
  bool centered = true;
  bool flipped = false;
};

struct RenderTextParams {
  bmin::String fontName = "default";
  TextSize fontSize = TextSize::TEXT_SIZE_16;
  int x = 0;
  int y = 0;
  SDL_Color color = {0, 0, 0, 255};
  bool centered = false;
  double angleDeg = 0.;
  std::pair<double, double> scale = {1., 1.};
  bool outlined = false;
};

struct Renderable {
  SDL_Texture* tex = nullptr;
  SDL_Surface* surf = nullptr;
};

struct Sprite {
  bmin::String name;
  Renderable renderable;
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  int spritesheetWidth = 0;
  bool flipped = false;
};

enum DrawMode {
  CPU,
  GPU,
};

class Draw {
  struct TextTextureEntry {
    bmin::UniquePtr<SDL_Texture, SDL_Deleter> texture;
    int width = 0;
    int height = 0;
    uint64_t lastUsed = 0;
  };

  struct DynamicTextEntry {
    bmin::UniquePtr<SDL_Texture, SDL_Deleter> texture;
    bmin::String text;
    bmin::String styleKey;
    int width = 0;
    int height = 0;
    int capacityWidth = 0;
    int capacityHeight = 0;
  };

  Store& store;
  int renderWidth = 0;
  int renderHeight = 0;

  SDL_Renderer* sdlRenderer = nullptr;
  SDL_Texture* intermediate = nullptr;

  SDL_Color backgroundColor = {0, 0, 0, 255};
  double renderRotationAngle = 0.0;
  int globalAlpha = 255;
  bmin::Map<bmin::String, bool> invalidSpriteWarnings;
  bmin::Map<bmin::String, TextTextureEntry> textCache;
  bmin::Map<bmin::String, DynamicTextEntry> dynamicTextCache;
  size_t textCacheLimit = 256;
  uint64_t textUseCounter = 0;

  bmin::String makeTextStyleKey(const RenderTextParams& params) const;
  bmin::String makeTextCacheKey(std::string_view text,
                                const RenderTextParams& params) const;
  void evictOldestTextTexture();
  SDL_Texture* getTextTexture(std::string_view text,
                              const RenderTextParams& params);
  DynamicTextEntry& getDynamicTextTexture(std::string_view slot,
                                          std::string_view text,
                                          const RenderTextParams& params);
  void drawSpriteInner(const Sprite& sprite, const RenderableParamsEx& params);

public:
  void drawTexture(SDL_Texture* tex, const RenderableParams& params);
  void drawTexture(SDL_Texture* tex, const RenderableParamsEx& params);

  Draw(Store& store);
  ~Draw();
  Draw(const Draw&) = delete;
  Draw& operator=(const Draw&) = delete;
  void setSdlRenderer(SDL_Renderer* r,
                      int renderWidth,
                      int renderHeight,
                      Uint32 format);
  SDL_Renderer* getSdlRenderer() { return sdlRenderer; }
  SDL_Texture* getIntermediate() { return intermediate; }
  std::pair<int, int> getRenderSize() const {
    return {renderWidth, renderHeight};
  }
  void setRenderRotationAngle(double angle) { renderRotationAngle = angle; }
  void setGlobalAlpha(int alpha) {
    globalAlpha = alpha < 0 ? 0 : (alpha > 255 ? 255 : alpha);
  }
  int getGlobalAlpha() const { return globalAlpha; }

  void setBackgroundColor(const SDL_Color& color);

  SDL_Texture* createTexture(SDL_Surface* surf);
  void drawSprite(const Sprite& sprite, const RenderableParams& params);
  void drawSprite(const Sprite& sprite, const RenderableParamsEx& params);
  void drawAnimation(const Animation& anim, const RenderableParams& params);
  void drawAnimation(const Animation& anim, const RenderableParamsEx& params);
  void drawText(std::string_view text, const RenderTextParams& params);
  void drawDynamicText(std::string_view slot,
                       std::string_view text,
                       const RenderTextParams& params);
  void clearTextCache();
  void clearDynamicText(std::string_view slot);
  void setTextCacheLimit(size_t limit);
  size_t getTextCacheLimit() const { return textCacheLimit; }
  size_t getTextCacheSize() const { return textCache.size(); }
  size_t getDynamicTextCount() const { return dynamicTextCache.size(); }
  std::pair<int, int> measureText(std::string_view text,
                                  const RenderTextParams& params);
  void drawRect(int x, int y, int w, int h, const SDL_Color& color);
  void drawLine(const std::pair<int, int>& from,
                const std::pair<int, int>& to,
                int lineWidth,
                const SDL_Color& color);
  void drawCircle(
      int x, int y, int radius, const SDL_Color& color, bool filled = true);

  void clearScreen();

  void renderIntermediate();
  void releaseRendererResources();
};

} // namespace sdl2w
