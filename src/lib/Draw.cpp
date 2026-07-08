#include "Draw.h"
#include "Defines.h"
#include "Logger.h"
#include "Store.h"
#include <bmin/StringStream.h>
#include <algorithm>
#include <string_view>

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

namespace sdl2w {

// https://gist.github.com/Gumichan01/332c26f6197a432db91cc4327fcabb1c
int SDL_RenderDrawCircle(SDL_Renderer* renderer, int x, int y, int radius) {
  int offsetX, offsetY, d;
  int status;

  offsetX = 0;
  offsetY = radius;
  d = radius - 1;
  status = 0;

  while (offsetY >= offsetX) {
    status += SDL_RenderDrawPoint(renderer, x + offsetX, y + offsetY);
    status += SDL_RenderDrawPoint(renderer, x + offsetY, y + offsetX);
    status += SDL_RenderDrawPoint(renderer, x - offsetX, y + offsetY);
    status += SDL_RenderDrawPoint(renderer, x - offsetY, y + offsetX);
    status += SDL_RenderDrawPoint(renderer, x + offsetX, y - offsetY);
    status += SDL_RenderDrawPoint(renderer, x + offsetY, y - offsetX);
    status += SDL_RenderDrawPoint(renderer, x - offsetX, y - offsetY);
    status += SDL_RenderDrawPoint(renderer, x - offsetY, y - offsetX);

    if (status < 0) {
      status = -1;
      break;
    }

    if (d >= 2 * offsetX) {
      d -= 2 * offsetX + 1;
      offsetX += 1;
    } else if (d < 2 * (radius - offsetY)) {
      d += 2 * offsetY - 1;
      offsetY -= 1;
    } else {
      d += 2 * (offsetY - offsetX - 1);
      offsetY -= 1;
      offsetX += 1;
    }
  }

  return status;
}

int SDL_RenderFillCircle(SDL_Renderer* renderer, int x, int y, int radius) {
  int offsetX, offsetY, d;
  int status;

  offsetX = 0;
  offsetY = radius;
  d = radius - 1;
  status = 0;

  while (offsetY >= offsetX) {

    status += SDL_RenderDrawLine(
        renderer, x - offsetY, y + offsetX, x + offsetY, y + offsetX);
    status += SDL_RenderDrawLine(
        renderer, x - offsetX, y + offsetY, x + offsetX, y + offsetY);
    status += SDL_RenderDrawLine(
        renderer, x - offsetX, y - offsetY, x + offsetX, y - offsetY);
    status += SDL_RenderDrawLine(
        renderer, x - offsetY, y - offsetX, x + offsetY, y - offsetX);

    if (status < 0) {
      status = -1;
      break;
    }

    if (d >= 2 * offsetX) {
      d -= 2 * offsetX + 1;
      offsetX += 1;
    } else if (d < 2 * (radius - offsetY)) {
      d += 2 * offsetY - 1;
      offsetY -= 1;
    } else {
      d += 2 * (offsetY - offsetX - 1);
      offsetY -= 1;
      offsetX += 1;
    }
  }

  return status;
}

std::pair<int, int> Draw::measureText(std::string_view text,
                                      const RenderTextParams& params) {
  TTF_Font* font = store.getFont(params.fontName.sliceView(), params.fontSize);
  bmin::String textStr(text.data(), text.size());
  int ww = 0, hh = 0;
  TTF_SizeUTF8(font, textStr.cStr(), &ww, &hh);
  return {ww, hh};
}

SDL_Texture* Draw::getTextTexture(std::string_view text,
                                   const RenderTextParams& params) {
  bmin::StringStream keyStream;
  keyStream << text << static_cast<int>(params.fontSize) << params.fontName
            << params.color.r << params.color.g << params.color.b;
  const bmin::String key = keyStream.str();
  if (store.hasDynamicTexture(key.sliceView())) {
    return store.getDynamicTexture(key.sliceView());
  }

  TTF_Font* font = store.getFont(params.fontName.sliceView(), params.fontSize);
  bmin::String textStr(text.data(), text.size());
  auto [ww, hh] = measureText(text, params);
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

  SDL_Surface* msg =
      TTF_RenderUTF8_Blended(font, textStr.cStr(), params.color);
  SDL_SetSurfaceBlendMode(msg, SDL_BLENDMODE_BLEND);
  SDL_BlitSurface(msg, nullptr, blitSurface, nullptr);
  SDL_FreeSurface(msg);

  SDL_Texture* texPtr = SDL_CreateTexture(
      sdlRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, ww, hh);

  SDL_SetTextureBlendMode(texPtr, SDL_BLENDMODE_BLEND);
  SDL_UpdateTexture(texPtr, nullptr, blitSurface->pixels, blitSurface->pitch);
  SDL_FreeSurface(blitSurface);

  store.storeDynamicTexture(key.sliceView(), texPtr);
  return texPtr;
}

Draw::Draw(Store& storeA) : store(storeA) {}

Draw::~Draw() {
  if (intermediate != nullptr) {
    SDL_DestroyTexture(intermediate);
  }
}

void Draw::drawTexture(SDL_Texture* tex, const RenderableParams& params) {
  int width, height;
  SDL_QueryTexture(tex, nullptr, nullptr, &width, &height);
  drawTexture(tex,
              RenderableParamsEx{.scale = params.scale,
                                 .angleDeg = 0.,
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

void Draw::setSdlRenderer(SDL_Renderer* r,
                          int renderWidthA,
                          int renderHeightA,
                          Uint32 format) {
  LOG(DEBUG) << "[sdl2w] Set sdlRenderer, renderW and renderH: " << renderWidthA
             << "," << renderHeightA << Logger::endl;
  sdlRenderer = r;
  renderWidth = renderWidthA;
  renderHeight = renderHeightA;

  intermediate = SDL_CreateTexture(sdlRenderer,
                                     format,
                                     SDL_TEXTUREACCESS_TARGET,
                                     renderWidth,
                                     renderHeight);
  SDL_SetTextureBlendMode(intermediate, SDL_BLENDMODE_BLEND);

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
  SDL_Texture* tex = sprite.renderable.tex;

  if (tex == nullptr) {
    if (!invalidSpriteWarnings.contains(sprite.name)) {
      LOG_LINE(ERROR) << "[sdl2w] Cannot drawSprite - Sprite missing required "
                         "texture: "
                      << sprite.name << Logger::endl;
      invalidSpriteWarnings[sprite.name] = true;
    }
    return;
  }

  drawTexture(tex, params);
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
                 .flipped = params.flipped || anim.flipped});
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
                     .w = sprite.w,
                     .h = sprite.h,
                     .clipX = sprite.x,
                     .clipY = sprite.y,
                     .clipW = sprite.w,
                     .clipH = sprite.h,
                     .centered = params.centered,
                     .flipped = params.flipped || anim.flipped});
  } else {
    LOG_LINE(ERROR) << "Anim has not been initialized: '" << anim.toString()
                    << "'" << Logger::endl;
    THROW_RUNTIME_ERROR(
        bmin::String(FAIL_ERROR_TEXT.data(), FAIL_ERROR_TEXT.size()));
  }
}

void Draw::drawText(std::string_view text, const RenderTextParams& params) {
  SDL_Texture* tex = getTextTexture(text, params);
  int width, height;
  SDL_QueryTexture(tex, nullptr, nullptr, &width, &height);
  drawTexture(tex,
              RenderableParamsEx{.scale = params.scale,
                                 .angleDeg = params.angleDeg,
                                 .x = params.x,
                                 .y = params.y,
                                 .w = width,
                                 .h = height,
                                 .clipX = 0,
                                 .clipY = 0,
                                 .clipW = width,
                                 .clipH = height,
                                 .centered = params.centered,
                                 .flipped = false});
}

void Draw::drawRect(int x, int y, int w, int h, const SDL_Color& color) {
  SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
  SDL_Rect rect = {x, y, w, h};
  SDL_RenderFillRect(sdlRenderer, &rect);
  SDL_SetRenderDrawColor(sdlRenderer,
                         backgroundColor.r,
                         backgroundColor.g,
                         backgroundColor.b,
                         backgroundColor.a);
}

void Draw::drawLine(const std::pair<int, int>& from,
                    const std::pair<int, int>& to,
                    int lineWidth,
                    const SDL_Color& color) {
  const int w = std::max(1, lineWidth);

  if (from.first == to.first && from.second == to.second) {
    const Sint16 x = static_cast<Sint16>(from.first);
    const Sint16 y = static_cast<Sint16>(from.second);
    if (w <= 1) {
      pixelRGBA(sdlRenderer, x, y, color.r, color.g, color.b, color.a);
    } else {
      const int halfW = w / 2;
      boxRGBA(sdlRenderer,
              x - halfW,
              y - halfW,
              x - halfW + w - 1,
              y - halfW + w - 1,
              color.r,
              color.g,
              color.b,
              color.a);
    }
    SDL_SetRenderDrawColor(sdlRenderer,
                           backgroundColor.r,
                           backgroundColor.g,
                           backgroundColor.b,
                           backgroundColor.a);
    return;
  }

  const Uint8 gfxW = static_cast<Uint8>(std::min(w, 255));

  thickLineRGBA(sdlRenderer,
                static_cast<Sint16>(from.first),
                static_cast<Sint16>(from.second),
                static_cast<Sint16>(to.first),
                static_cast<Sint16>(to.second),
                gfxW,
                color.r,
                color.g,
                color.b,
                color.a);
  SDL_SetRenderDrawColor(sdlRenderer,
                         backgroundColor.r,
                         backgroundColor.g,
                         backgroundColor.b,
                         backgroundColor.a);
}

void Draw::drawCircle(
    int x, int y, int radius, const SDL_Color& color, bool filled) {
  SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
  if (filled) {
    SDL_RenderFillCircle(sdlRenderer, x, y, radius);
  } else {
    SDL_RenderDrawCircle(sdlRenderer, x, y, radius);
  }
  SDL_SetRenderDrawColor(sdlRenderer,
                         backgroundColor.r,
                         backgroundColor.g,
                         backgroundColor.b,
                         backgroundColor.a);
}

void Draw::clearScreen() {
  SDL_SetRenderTarget(sdlRenderer, intermediate);
  SDL_SetRenderDrawColor(sdlRenderer,
                         backgroundColor.r,
                         backgroundColor.g,
                         backgroundColor.b,
                         backgroundColor.a);
  SDL_RenderClear(sdlRenderer);
}

void Draw::renderIntermediate() {
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
