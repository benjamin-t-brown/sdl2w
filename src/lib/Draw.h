#pragma once

#include "Animation.h"
#include "Defines.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#if defined(MIYOOA30) || defined(MIYOOMINI)
#include <SDL_pixels.h>
#include <SDL_stdinc.h>
#else
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_stdinc.h>
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
  std::string fontName = "default";
  TextSize fontSize = TextSize::TEXT_SIZE_16;
  int x = 0;
  int y = 0;
  SDL_Color color = {0, 0, 0, 255};
  bool centered = false;
};

struct Renderable {
  SDL_Texture* tex = nullptr;
  SDL_Surface* surf = nullptr;
};

struct Sprite {
  std::string name;
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
  // decided when instance is created
  const DrawMode mode;
  Store& store;
  // screen size, independent of window size
  int renderWidth = 0;
  int renderHeight = 0;

  // does not own
  SDL_Renderer* sdlRenderer = nullptr;

  // does own
  SDL_Texture* intermediate = nullptr;
  SDL_Surface* screen = nullptr;
  std::unordered_map<std::string, std::unique_ptr<SDL_Texture, SDL_Deleter>>
      cachedTextures;
  std::unordered_map<std::string, std::unique_ptr<SDL_Surface, SDL_Deleter>>
      cachedSurfaces;

  SDL_Color backgroundColor = {0, 0, 0, 255};
  double renderRotationAngle = 0.0;
  int globalAlpha = 255;

  Renderable getTextRenderable(const std::string& text,
                               const RenderTextParams& params);
  SDL_Surface* getRotatedSurface(SDL_Surface* originalSurface,
                                 const std::string& name,
                                 double angleDeg,
                                 const RenderableParamsEx& params);

  void drawSpriteInner(const Sprite& sprite, const RenderableParamsEx& params);

public:
  void drawTexture(SDL_Texture* tex, const RenderableParams& params);
  void drawTexture(SDL_Texture* tex, const RenderableParamsEx& params);
  void drawSurface(SDL_Surface* surf, const RenderableParams& params);
  void drawSurface(SDL_Surface* surf, const RenderableParamsEx& params);

  Draw(DrawMode mode, Store& store);
  ~Draw();
  void setSdlRenderer(SDL_Renderer* r,
                      int renderWidth,
                      int renderHeight,
                      Uint32 format);
  SDL_Renderer* getSdlRenderer() { return sdlRenderer; }
  SDL_Texture* getIntermediate() { return intermediate; }
  SDL_Surface* getScreen() { return screen; }
  std::pair<int, int> getRenderSize() const {
    return {renderWidth, renderHeight};
  }
  void setRenderRotationAngle(double angle) { renderRotationAngle = angle; }
  void setGlobalAlpha(int alpha) { globalAlpha = alpha; }
  int getGlobalAlpha() const { return globalAlpha; }

  void setBackgroundColor(const SDL_Color& color);

  SDL_Texture* createTexture(SDL_Surface* surf);
  void drawSprite(const Sprite& sprite, const RenderableParams& params);
  void drawSprite(const Sprite& sprite, const RenderableParamsEx& params);
  void drawAnimation(const Animation& anim, const RenderableParams& params);
  void drawAnimation(const Animation& anim, const RenderableParamsEx& params);
  void drawText(const std::string& text, const RenderTextParams& params);
  void drawRect(int x, int y, int w, int h, const SDL_Color& color);
  void drawCircle(
      int x, int y, int radius, const SDL_Color& color, bool filled = true);

  void clearScreen();

  void renderIntermediate();

  // void drawText(const RenderTextParams& params);
};

} // namespace sdl2w