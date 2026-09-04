module;
#include <string_view>
#include <utility>

#include "impl_headers.h"
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_image.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <SDL_surface.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#endif


export module sdl2w.draw;
export import sdl2w.animation;
export import sdl2w.store;
export import sdl2w.types;
export import bmin.containers;
import sdl2w.defines;
import sdl2w.logger;
import bmin.string_interop;
import bmin.stringstream;

export namespace sdl2w {

class Draw {
  Store& store;
  int renderWidth = 0;
  int renderHeight = 0;

  SDL_Renderer* sdlRenderer = nullptr;
  SDL_Texture* intermediate = nullptr;

  SDL_Color backgroundColor = {0, 0, 0, 255};
  double renderRotationAngle = 0.0;
  int globalAlpha = 255;
  bmin::Map<bmin::String, bool> invalidSpriteWarnings;

  SDL_Texture* getTextTexture(std::string_view text,
                              const RenderTextParams& params);
  void drawSpriteInner(const Sprite& sprite, const RenderableParamsEx& params);

public:
  void drawTexture(SDL_Texture* tex, const RenderableParams& params);
  void drawTexture(SDL_Texture* tex, const RenderableParamsEx& params);

  Draw(Store& store);
  ~Draw();
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
  void setGlobalAlpha(int alpha) { globalAlpha = alpha; }
  int getGlobalAlpha() const { return globalAlpha; }

  void setBackgroundColor(const SDL_Color& color);

  SDL_Texture* createTexture(SDL_Surface* surf);
  void drawSprite(const Sprite& sprite, const RenderableParams& params);
  void drawSprite(const Sprite& sprite, const RenderableParamsEx& params);
  void drawAnimation(const Animation& anim, const RenderableParams& params);
  void drawAnimation(const Animation& anim, const RenderableParamsEx& params);
  void drawText(std::string_view text, const RenderTextParams& params);
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
};

}
