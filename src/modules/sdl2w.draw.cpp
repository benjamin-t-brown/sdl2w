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

module sdl2w.draw;
import sdl2w.animation;
import sdl2w.store;
import sdl2w.types;
import bmin.containers;
import sdl2w.defines;
import sdl2w.logger;
import bmin.string_interop;
import bmin.stringstream;

namespace sdl2w {

namespace {

int textureCapacity(int required) {
  int capacity = 1;
  while (capacity < required && capacity <= (1 << 29)) {
    capacity *= 2;
  }
  return capacity;
}

} // namespace

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
  if (text.empty()) {
    return {0, 0};
  }
  TTF_Font* font = store.getFont(
      params.fontName.sliceView(), params.fontSize, params.outlined);
  bmin::String textStr(text.data(), text.size());
  int ww = 0, hh = 0;
  if (TTF_SizeUTF8(font, textStr.cStr(), &ww, &hh) != 0) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Failed to measure text: ") +
                        TTF_GetError());
  }
  return {ww, hh};
}

bmin::String Draw::makeTextStyleKey(const RenderTextParams& params) const {
  bmin::StringStream key;
  key << params.fontName.size() << ':' << params.fontName << ':'
      << static_cast<int>(params.fontSize) << ':' << params.color.r << ':'
      << params.color.g << ':' << params.color.b << ':' << params.color.a << ':'
      << params.outlined;
  return key.str();
}

bmin::String Draw::makeTextCacheKey(std::string_view text,
                                    const RenderTextParams& params) const {
  bmin::StringStream key;
  key << text.size() << ':' << text << ':' << makeTextStyleKey(params);
  return key.str();
}

void Draw::evictOldestTextTexture() {
  if (textCache.empty()) {
    return;
  }
  auto oldest = textCache.begin();
  for (auto it = textCache.begin(); it != textCache.end(); ++it) {
    if ((*it).value.lastUsed < (*oldest).value.lastUsed) {
      oldest = it;
    }
  }
  textCache.erase(oldest);
}

SDL_Texture* Draw::getTextTexture(std::string_view text,
                                  const RenderTextParams& params) {
  const bmin::String key = makeTextCacheKey(text, params);
  auto cached = textCache.find(key);
  if (cached != textCache.end()) {
    (*cached).value.lastUsed = ++textUseCounter;
    return (*cached).value.texture.get();
  }

  TTF_Font* font = store.getFont(
      params.fontName.sliceView(), params.fontSize, params.outlined);
  bmin::String textStr(text.data(), text.size());
  bmin::UniquePtr<SDL_Surface, SDL_Deleter> surface(
      TTF_RenderUTF8_Blended(font, textStr.cStr(), params.color));
  if (!surface) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Failed to render text: ") +
                        TTF_GetError());
  }
  bmin::UniquePtr<SDL_Texture, SDL_Deleter> texture(
      SDL_CreateTextureFromSurface(sdlRenderer, surface.get()));
  if (!texture) {
    THROW_RUNTIME_ERROR(
        bmin::String("[sdl2w] Failed to create text texture: ") +
        SDL_GetError());
  }
  SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND);

  while (textCache.size() >= textCacheLimit) {
    evictOldestTextTexture();
  }
  TextTextureEntry entry;
  entry.width = surface->w;
  entry.height = surface->h;
  entry.lastUsed = ++textUseCounter;
  SDL_Texture* result = texture.get();
  entry.texture = std::move(texture);
  textCache.insert(key, std::move(entry));
  return result;
}

Draw::DynamicTextEntry&
Draw::getDynamicTextTexture(std::string_view slot,
                            std::string_view text,
                            const RenderTextParams& params) {
  const bmin::String slotKey(slot.data(), slot.size());
  auto found = dynamicTextCache.find(slotKey);
  if (found == dynamicTextCache.end()) {
    dynamicTextCache.insert(slotKey, DynamicTextEntry{});
    found = dynamicTextCache.find(slotKey);
  }
  DynamicTextEntry& entry = (*found).value;
  const bmin::String styleKey = makeTextStyleKey(params);
  const bmin::String textStr(text.data(), text.size());
  if (entry.texture && entry.text == textStr && entry.styleKey == styleKey) {
    return entry;
  }

  TTF_Font* font = store.getFont(
      params.fontName.sliceView(), params.fontSize, params.outlined);
  bmin::UniquePtr<SDL_Surface, SDL_Deleter> rendered(
      TTF_RenderUTF8_Blended(font, textStr.cStr(), params.color));
  if (!rendered) {
    THROW_RUNTIME_ERROR(
        bmin::String("[sdl2w] Failed to render dynamic text: ") +
        TTF_GetError());
  }
  bmin::UniquePtr<SDL_Surface, SDL_Deleter> surface(
      SDL_ConvertSurfaceFormat(rendered.get(), SDL_PIXELFORMAT_RGBA32, 0));
  if (!surface) {
    THROW_RUNTIME_ERROR(
        bmin::String("[sdl2w] Failed to convert dynamic text surface: ") +
        SDL_GetError());
  }

  if (!entry.texture || surface->w > entry.capacityWidth ||
      surface->h > entry.capacityHeight) {
    entry.capacityWidth = textureCapacity(surface->w);
    entry.capacityHeight = textureCapacity(surface->h);
    entry.texture.reset(SDL_CreateTexture(sdlRenderer,
                                          SDL_PIXELFORMAT_RGBA32,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          entry.capacityWidth,
                                          entry.capacityHeight));
    if (!entry.texture) {
      THROW_RUNTIME_ERROR(
          bmin::String("[sdl2w] Failed to create dynamic text texture: ") +
          SDL_GetError());
    }
    SDL_SetTextureBlendMode(entry.texture.get(), SDL_BLENDMODE_BLEND);
  }

  const SDL_Rect updateRect = {0, 0, surface->w, surface->h};
  if (SDL_UpdateTexture(
          entry.texture.get(), &updateRect, surface->pixels, surface->pitch) !=
      0) {
    THROW_RUNTIME_ERROR(
        bmin::String("[sdl2w] Failed to update dynamic text texture: ") +
        SDL_GetError());
  }
  entry.text = textStr;
  entry.styleKey = styleKey;
  entry.width = surface->w;
  entry.height = surface->h;
  return entry;
}

Draw::Draw(Store& storeA) : store(storeA) {}

Draw::~Draw() { releaseRendererResources(); }

void Draw::releaseRendererResources() {
  clearTextCache();
  if (intermediate != nullptr) {
    SDL_DestroyTexture(intermediate);
    intermediate = nullptr;
  }
  sdlRenderer = nullptr;
}

void Draw::drawTexture(SDL_Texture* tex, const RenderableParams& params) {
  if (tex == nullptr || sdlRenderer == nullptr)
    return;
  int width, height;
  if (SDL_QueryTexture(tex, nullptr, nullptr, &width, &height) != 0)
    return;
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
  if (tex == nullptr || sdlRenderer == nullptr)
    return;
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
  releaseRendererResources();
  if (r == nullptr || renderWidthA <= 0 || renderHeightA <= 0)
    THROW_RUNTIME_ERROR("[sdl2w] Invalid renderer or render dimensions");
  sdlRenderer = r;
  renderWidth = renderWidthA;
  renderHeight = renderHeightA;

  intermediate = SDL_CreateTexture(
      sdlRenderer, format, SDL_TEXTUREACCESS_TARGET, renderWidth, renderHeight);
  if (intermediate == nullptr && format != SDL_PIXELFORMAT_RGBA8888)
    intermediate = SDL_CreateTexture(sdlRenderer,
                                     SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_TARGET,
                                     renderWidth,
                                     renderHeight);
  if (intermediate == nullptr)
    THROW_RUNTIME_ERROR(
        bmin::String("[sdl2w] Could not create render target: ") +
        SDL_GetError());
  if (SDL_SetTextureBlendMode(intermediate, SDL_BLENDMODE_BLEND) != 0 ||
      SDL_SetRenderTarget(sdlRenderer, intermediate) != 0) {
    releaseRendererResources();
    THROW_RUNTIME_ERROR(
        bmin::String("[sdl2w] Could not configure render target: ") +
        SDL_GetError());
  }
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
                   .flipped = params.flipped != sprite.flipped});
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
                   .flipped = params.flipped != sprite.flipped});
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
  if (text.empty()) {
    return;
  }
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

void Draw::drawDynamicText(std::string_view slot,
                           std::string_view text,
                           const RenderTextParams& params) {
  if (text.empty()) {
    return;
  }
  DynamicTextEntry& entry = getDynamicTextTexture(slot, text, params);
  drawTexture(entry.texture.get(),
              RenderableParamsEx{.scale = params.scale,
                                 .angleDeg = params.angleDeg,
                                 .x = params.x,
                                 .y = params.y,
                                 .w = entry.width,
                                 .h = entry.height,
                                 .clipX = 0,
                                 .clipY = 0,
                                 .clipW = entry.width,
                                 .clipH = entry.height,
                                 .centered = params.centered,
                                 .flipped = false});
}

void Draw::clearTextCache() {
  textCache.clear();
  dynamicTextCache.clear();
  textUseCounter = 0;
}

void Draw::clearDynamicText(std::string_view slot) {
  dynamicTextCache.erase(slot);
}

void Draw::setTextCacheLimit(size_t limit) {
  textCacheLimit = std::max<size_t>(1, limit);
  while (textCache.size() > textCacheLimit) {
    evictOldestTextTexture();
  }
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
  if (sdlRenderer == nullptr || intermediate == nullptr)
    return;
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
