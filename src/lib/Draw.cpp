#include "Draw.h"
#include "Defines.h"
#include "Logger.h"
#include "Store.h"
#include <sstream>

#if defined(MIYOOA30) || defined(MIYOOMINI)
#include <SDL.h>
#include <SDL2_rotozoom.h>
#include <SDL_image.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <SDL_surface.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL2_rotozoom.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#endif

namespace sdl2w {

Renderable Draw::getTextRenderable(const std::string& text,
                                   const RenderTextParams& params) {
  std::stringstream keyStream;
  keyStream << text << params.fontSize << params.fontName << params.color.r
            << params.color.g << params.color.b;
  const std::string key = keyStream.str();
  if (store.hasDynamicTextureOrSurface(key)) {
    return Renderable{store.getDynamicTexture(key),
                      store.getDynamicSurface(key)};
  }

  TTF_Font* font = store.getFont(params.fontName, params.fontSize);
  int ww = 0, hh = 0;
  TTF_SizeUTF8(font, text.c_str(), &ww, &hh);
  SDL_Surface* blitSurface = SDL_CreateRGBSurface(0,
                                                  ww,
                                                  hh,
                                                  32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                                                  0xff000000,
                                                  0x00ff0000,
                                                  0x0000ff00,
                                                  0x000000ff
#else
                                                  0x000000ff,
                                                  0x0000ff00,
                                                  0x00ff0000,
                                                  0xff000000
#endif
  );
  SDL_SetSurfaceBlendMode(blitSurface, SDL_BLENDMODE_BLEND);
  SDL_FillRect(
      blitSurface, nullptr, SDL_MapRGBA(blitSurface->format, 0, 0, 0, 0));

  SDL_Surface* msg = TTF_RenderUTF8_Blended(font, text.c_str(), params.color);
  SDL_SetSurfaceBlendMode(msg, SDL_BLENDMODE_BLEND);
  SDL_BlitSurface(msg, nullptr, blitSurface, nullptr);
  SDL_FreeSurface(msg);

  SDL_Texture* texPtr = SDL_CreateTexture(
      sdlRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, ww, hh);

  SDL_SetTextureBlendMode(texPtr, SDL_BLENDMODE_BLEND);
  SDL_UpdateTexture(texPtr, nullptr, blitSurface->pixels, blitSurface->pitch);

  store.storeDynamicTexture(key, texPtr);
  store.storeDynamicSurface(key, blitSurface);
  return {texPtr, blitSurface};
}

SDL_Surface* Draw::getRotatedSurface(SDL_Surface* originalSurface,
                                     const std::string& name,
                                     double angleDeg,
                                     const RenderableParamsEx& params) {
  while (angleDeg < 0) {
    angleDeg += 360;
  }
  while (angleDeg >= 360) {
    angleDeg -= 360;
  }

  std::stringstream keyStream;
  keyStream << name << originalSurface->w << "," << originalSurface->h << ","
            << angleDeg << "," << params.clipX << "," << params.clipY << ","
            << params.clipW << "," << params.clipH << "," << params.scale.first
            << "," << params.scale.second << "," << params.flipped;
  std::string key = keyStream.str();

  if (store.hasDynamicTextureOrSurface(key)) {
    return store.getDynamicSurface(key);
  }

  SDL_Rect clip = {params.clipX, params.clipY, params.clipW, params.clipH};

  // Create a new surface for just the clip.
  SDL_Surface* spriteSurface =
      SDL_CreateRGBSurface(0,
                           clip.w * params.scale.first,
                           clip.h * params.scale.second,
                           originalSurface->format->BitsPerPixel,
                           originalSurface->format->Rmask,
                           originalSurface->format->Gmask,
                           originalSurface->format->Bmask,
                           originalSurface->format->Amask);
  SDL_Rect destRect = {
      .x = 0,
      .y = 0,
      .w = static_cast<int>(clip.w * params.scale.first),
      .h = static_cast<int>(clip.h * params.scale.second),
  };
  SDL_BlitScaled(originalSurface, &clip, spriteSurface, &destRect);
  SDL_Surface* rotatedSurface = rotozoomSurface(
      spriteSurface, static_cast<double>(angleDeg), 1., SMOOTHING_OFF);

  SDL_FreeSurface(spriteSurface);
  store.storeDynamicSurface(key, rotatedSurface);

  return rotatedSurface;
}

Draw::Draw(DrawMode mode, Store& storeA) : mode(mode), store(storeA) {
  // pointers to intermediate and screen are created in setSdlRenderer
}

Draw::~Draw() {
  if (intermediate != nullptr) {
    SDL_DestroyTexture(intermediate);
  }
  if (screen != nullptr) {
    SDL_FreeSurface(screen);
  }
}

void Draw::drawTexture(SDL_Texture* tex, const RenderableParams& params) {
  int width, height;
  SDL_QueryTexture(tex, nullptr, nullptr, &width, &height);
  drawTexture(tex,
              {.scale = params.scale,
               .angleDeg = 0,
               .x = params.x,
               .y = params.y,
               .w = width,
               .h = height,
               .clipX = 0,
               .clipY = 0,
               .clipW = width,
               .clipH = height,
               .centered = params.centered,
               .flipped = params.flipped});
}

void Draw::drawTexture(SDL_Texture* tex, const RenderableParamsEx& params) {
  SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

  auto [scaleLocal,
        angleDeg,
        x,
        y,
        w,
        h,
        clipX,
        clipY,
        clipW,
        clipH,
        centered,
        flipped] = params;
  SDL_SetTextureAlphaMod(tex, globalAlpha);

  SDL_RendererFlip flip = flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

  const double scaledW = static_cast<double>(w) * scaleLocal.first;
  const double scaledH = static_cast<double>(h) * scaleLocal.second;

  const int halfW = static_cast<int>(scaledW) / 2;
  const int halfH = static_cast<int>(scaledH) / 2;

  const SDL_Rect pos = {
      .x = x + (centered ? -halfW : 0),
      .y = y + (centered ? -halfH : 0),
      .w = static_cast<int>(scaledW),
      .h = static_cast<int>(scaledH),
  };
  const SDL_Rect clip = {clipX, clipY, clipW, clipH};

  SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
  SDL_RenderCopyEx(sdlRenderer, tex, &clip, &pos, angleDeg, nullptr, flip);
}

void Draw::drawSurface(SDL_Surface* surf, const RenderableParams& params) {
  drawSurface(surf,
              {.scale = params.scale,
               .angleDeg = 0,
               .x = params.x,
               .y = params.y,
               .w = surf->w,
               .h = surf->h,
               .clipX = 0,
               .clipY = 0,
               .clipW = surf->w,
               .clipH = surf->h,
               .centered = params.centered,
               .flipped = params.flipped});
}

void Draw::drawSurface(SDL_Surface* surf, const RenderableParamsEx& params) {
  auto [scale,
        angleDeg,
        x,
        y,
        w,
        h,
        clipX,
        clipY,
        clipW,
        clipH,
        centered,
        flipped] = params;

  const double scaledW = static_cast<double>(w) * scale.first;
  const double scaledH = static_cast<double>(h) * scale.second;

  const int halfScaledW = static_cast<int>(scaledW) / 2;
  const int halfScaledH = static_cast<int>(scaledH) / 2;

  SDL_Rect pos = {
      .x = x + (centered ? -halfScaledW : 0),
      .y = y + (centered ? -halfScaledH : 0),
      .w = static_cast<int>(scaledW),
      .h = static_cast<int>(scaledH),
  };
  SDL_Rect clip = {clipX, clipY, clipW, clipH};
  SDL_SetSurfaceAlphaMod(surf, globalAlpha);

  if (clip.w == 0 || clip.h == 0) {
    SDL_BlitScaled(surf, nullptr, screen, &pos);
  } else {
    SDL_BlitScaled(surf, &clip, screen, &pos);
  }
}

void Draw::setSdlRenderer(SDL_Renderer* r,
                          int renderWidthA,
                          int renderHeightA,
                          Uint32 format) {
  LOG(DEBUG) << "[sdl2w] Set sdlRenderer, renderW and renderH: " << renderWidthA
             << "," << renderHeightA << Logger::endl;
  sdlRenderer = r;
  renderWidth = renderWidthA;
  renderHeight = renderHeightA;

  screen = SDL_CreateRGBSurface(0, renderWidth, renderHeight, 16, 0, 0, 0, 0);
  if (mode == CPU) {
    // CPU mode streams blitted SDL_Surfaces to an SDL_Texture.
    intermediate = SDL_CreateTexture(sdlRenderer,
                                     SDL_PIXELFORMAT_RGB565,
                                     SDL_TEXTUREACCESS_STREAMING,
                                     renderWidth,
                                     renderHeight);
  } else if (mode == GPU) {
    // GPU mode renders directly to an SDL_Texture as a target texture.
    intermediate = SDL_CreateTexture(sdlRenderer,
                                     format,
                                     SDL_TEXTUREACCESS_TARGET,
                                     renderWidth,
                                     renderHeight);
    SDL_SetTextureBlendMode(intermediate, SDL_BLENDMODE_BLEND);
  }

  SDL_SetRenderTarget(sdlRenderer, intermediate);
}

void Draw::setBackgroundColor(const SDL_Color& color) {
  backgroundColor = color;
  SDL_SetRenderDrawColor(sdlRenderer,
                         backgroundColor.r,
                         backgroundColor.g,
                         backgroundColor.b,
                         backgroundColor.a);
}

SDL_Texture* Draw::createTexture(SDL_Surface* surf) {
  return SDL_CreateTextureFromSurface(sdlRenderer, surf);
}

void Draw::drawSprite(const Sprite& sprite, const RenderableParams& params) {
  drawSpriteInner(sprite,
                  {.scale = params.scale,
                   .angleDeg = 0,
                   .x = params.x,
                   .y = params.y,
                   .w = sprite.w,
                   .h = sprite.h,
                   .clipX = sprite.x,
                   .clipY = sprite.y,
                   .clipW = sprite.w,
                   .clipH = sprite.h,
                   .centered = params.centered,
                   .flipped = params.flipped});
}

// use this to extract sprite into RenderableParamsEx
void Draw::drawSprite(const Sprite& sprite, const RenderableParamsEx& params) {
  drawSpriteInner(sprite,
                  {.scale = params.scale,
                   .angleDeg = params.angleDeg,
                   .x = params.x,
                   .y = params.y,
                   .w = sprite.w,
                   .h = sprite.h,
                   .clipX = sprite.x,
                   .clipY = sprite.y,
                   .clipW = sprite.w,
                   .clipH = sprite.h,
                   .centered = params.centered,
                   .flipped = params.flipped});
}

void Draw::drawSpriteInner(const Sprite& sprite,
                           const RenderableParamsEx& params) {
  auto [tex, surf] = sprite.renderable;

  if (tex == nullptr || surf == nullptr) {
    LOG_LINE(ERROR) << "[sdl2w] Cannot drawSprite - Sprite missing required "
                       "texture and/or surface: "
                    << sprite.name << " tex=" << tex << " surf=" << surf
                    << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }

  if (mode == DrawMode::CPU) {
    RenderableParamsEx cpuParams = params;
    if (params.flipped) {
      // instead of flipping the texture at draw time, the cpu method uses an
      // sdl surface, which is stored a flipped version of the whole
      // spritesheet. This means the draw function must also flip the
      // horizontal location of the clipping rect
      cpuParams.clipX = sprite.spritesheetWidth - sprite.x - sprite.w;
    }
    if (params.angleDeg != 0) {
      // the sdl2 gfx library's rotozoomSurface function is used to rotate the
      // surface, which creates a new surface with sprite extracted from the
      // spritesheet, rotated and zoomed to params scale.  After this operation,
      // the params for drawSurface must be updated to account for the new
      // surface, since it no longer should have a clipping rect and it has a
      // different size than the original clip (due to rotation)
      SDL_Surface* rotatedSurface =
          getRotatedSurface(surf, sprite.name, params.angleDeg, cpuParams);
      cpuParams.clipX = 0;
      cpuParams.clipY = 0;
      cpuParams.clipW = 0;
      cpuParams.clipH = 0;
      cpuParams.scale = {1., 1.};
      cpuParams.w = rotatedSurface->w;
      cpuParams.h = rotatedSurface->h;
      if (!params.centered) {
        cpuParams.x += params.w * params.scale.first / 2;
        cpuParams.y += params.h * params.scale.second / 2;
      }
      cpuParams.centered = true;
      drawSurface(rotatedSurface, cpuParams);
    } else {
      drawSurface(surf, cpuParams);
    }
  } else {
    drawTexture(tex, params);
  }
}

void Draw::drawAnimation(const Animation& anim,
                         const RenderableParams& params) {
  const Sprite& sprite = anim.getCurrentSprite();
  drawAnimation(anim,
                {.scale = params.scale,
                 .angleDeg = 0,
                 .x = params.x,
                 .y = params.y,
                 .w = sprite.w,
                 .h = sprite.h,
                 .clipX = sprite.x,
                 .clipY = sprite.y,
                 .clipW = sprite.w,
                 .clipH = sprite.h,
                 .centered = params.centered,
                 .flipped = params.flipped});
}

void Draw::drawAnimation(const Animation& anim,
                         const RenderableParamsEx& params) {
  if (anim.isInitialized()) {
    const Sprite& sprite = anim.getCurrentSprite();
    drawSpriteInner(sprite,
                    {.scale = params.scale,
                     .angleDeg = params.angleDeg,
                     .x = params.x,
                     .y = params.y,
                     .w = params.w,
                     .h = params.h,
                     .clipX = sprite.x,
                     .clipY = sprite.y,
                     .clipW = sprite.w,
                     .clipH = sprite.h,
                     .centered = params.centered,
                     .flipped = params.flipped});
  } else {
    LOG_LINE(ERROR) << "Anim has not been initialized: '" << anim.toString()
                    << "'" << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }
}

void Draw::drawText(const std::string& text, const RenderTextParams& params) {
  Renderable r = getTextRenderable(text, params);

  if (mode == DrawMode::CPU) {
    drawSurface(r.surf,
                RenderableParams{
                    .scale = {1., 1.},
                    .x = params.x,
                    .y = params.y,
                    .centered = params.centered,
                });
  } else {
    drawTexture(r.tex,
                RenderableParams{
                    .scale = {1., 1.},
                    .x = params.x,
                    .y = params.y,
                    .centered = params.centered,
                });
  }

  // int w, h;
  // SDL_QueryTexture(tex, nullptr, nullptr, &(w), &(h));
  // SDL_SetTextureAlphaMod(tex, globalAlpha);
  // const SDL_Rect pos = {x, transformY(y, h), w, h};
  // SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);
  // SDL_RenderCopy(renderer.get(), tex, nullptr, &pos);
}

void Draw::drawRect(int x, int y, int w, int h, const SDL_Color& color) {
  if (mode == DrawMode::CPU) {
    SDL_Rect rect = {x, y, w, h};
    SDL_FillRect(
        screen,
        &rect,
        SDL_MapRGBA(screen->format, color.r, color.g, color.b, color.a));
  } else if (mode == DrawMode::GPU) {
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(sdlRenderer, &rect);
    SDL_SetRenderDrawColor(sdlRenderer,
                           backgroundColor.r,
                           backgroundColor.g,
                           backgroundColor.b,
                           backgroundColor.a);
  }
}

void Draw::clearScreen() {
  if (mode == DrawMode::CPU) {
    SDL_FillRect(screen,
                 NULL,
                 SDL_MapRGB(screen->format,
                            backgroundColor.r,
                            backgroundColor.g,
                            backgroundColor.b));
  } else if (mode == DrawMode::GPU) {
    SDL_SetRenderTarget(sdlRenderer, intermediate);
    SDL_SetRenderDrawColor(sdlRenderer,
                           backgroundColor.r,
                           backgroundColor.g,
                           backgroundColor.b,
                           backgroundColor.a);
    SDL_RenderClear(sdlRenderer);
  }
}

void Draw::renderIntermediate() {
  if (mode == DrawMode::CPU) {
    SDL_UpdateTexture(intermediate, NULL, screen->pixels, screen->pitch);
  }
  SDL_SetRenderTarget(sdlRenderer, nullptr);
  SDL_RenderClear(sdlRenderer);
  SDL_RenderCopyEx(sdlRenderer,
                   intermediate,
                   nullptr,
                   nullptr,
                   renderRotationAngle,
                   nullptr,
                   SDL_FLIP_NONE);
  SDL_RenderPresent(sdlRenderer);
  SDL_SetRenderTarget(sdlRenderer, intermediate);
  clearScreen();
}
} // namespace sdl2w