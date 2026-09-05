#include "Window.h"
#include "AssetLoader.h"
#include "Defines.h"
#include "EmscriptenHelpers.h"
#include "Logger.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#if __has_include(<SDL.h>)
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

#include <algorithm>

namespace sdl2w {
bool Window::_soundEnabled = true;
bool Window::_inputEnabled = true;
bool Window::_isInit = false;
bool Window::_audioInitialized = false;
bool Window::_imageInitialized = false;
int Window::_mixerCodecFlags = 0;
Window* Window::_activeWindow = nullptr;

bool Window::isInit() { return _isInit; }

Window::Window(Store& store, const Window2Params& params)
    : store(store), draw(store) {
  if (!_isInit) {
    THROW_RUNTIME_ERROR("[sdl2w] Call Window::init() before creating a Window");
  }
  if (_activeWindow != nullptr) {
    THROW_RUNTIME_ERROR("[sdl2w] Only one Window may exist at a time");
  }
  if (params.w <= 0 || params.h <= 0 || params.renderW <= 0 ||
      params.renderH <= 0) {
    THROW_RUNTIME_ERROR("[sdl2w] Window dimensions must be positive");
  }

  LOG(DEBUG) << "[sdl2w] Create window:"
             << " " << params.w << " " << params.h << Logger::endl;

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
  Uint32 windowFlags = SDL_WINDOW_SHOWN;
  if (params.resizable)
    windowFlags |= SDL_WINDOW_RESIZABLE;
  if (params.borderless)
    windowFlags |= SDL_WINDOW_BORDERLESS;
  if (params.fullscreen)
    windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  sdlWindow = SDL_CreateWindow(
      params.title.cStr(), params.x, params.y, params.w, params.h, windowFlags);
  if (sdlWindow == nullptr) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Could not create window: ") +
                        SDL_GetError());
  }
  Uint32 rendererFlags = params.vsync ? SDL_RENDERER_PRESENTVSYNC : 0;
  rendererFlags |= (params.mode == DrawMode::GPU) ? SDL_RENDERER_ACCELERATED
                                                  : SDL_RENDERER_SOFTWARE;
  sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, rendererFlags);
  if (sdlRenderer == nullptr) {
    SDL_DestroyWindow(sdlWindow);
    sdlWindow = nullptr;
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Could not create renderer: ") +
                        SDL_GetError());
  }
  if (SDL_RenderSetLogicalSize(sdlRenderer, params.renderW, params.renderH) !=
      0) {
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
    sdlRenderer = nullptr;
    sdlWindow = nullptr;
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] Could not set logical size: ") +
                        SDL_GetError());
  }
  Uint32 format = SDL_GetWindowPixelFormat(sdlWindow);
  try {
    draw.setSdlRenderer(sdlRenderer, params.renderW, params.renderH, format);
  } catch (...) {
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
    sdlRenderer = nullptr;
    sdlWindow = nullptr;
    throw;
  }

  Mix_AllocateChannels(numSoundChannels);

  windowWidth = params.w;
  windowHeight = params.h;
  renderWidth = params.renderW;
  renderHeight = params.renderH;
  maxDeltaTime = params.maxDeltaTime > 0.0 ? params.maxDeltaTime : 100.0;

  AssetLoader::initFs();

  emshelpers::setEmscriptenWindow(this);
  _activeWindow = this;
#ifdef __EMSCRIPTEN__
  emshelpers::notifyTargetWindowSize(renderWidth, renderHeight);
#endif
}

Window::~Window() {
  if (_activeWindow != this)
    return;
  events.disableControllers();
  store.clear();
  draw.releaseRendererResources();
  SDL_DestroyRenderer(sdlRenderer);
  SDL_DestroyWindow(sdlWindow);
  sdlRenderer = nullptr;
  sdlWindow = nullptr;
  emshelpers::setEmscriptenWindow(nullptr);
  _activeWindow = nullptr;
}

bool Window::isReady() const { return _isInit && AssetLoader::fsReady; }

void Window::setSoundPct(int pct) {
  soundPct = std::clamp(pct, 0, 100);
  if (!_soundEnabled) {
    return;
  }
  Mix_Volume(
      -1, static_cast<int>(double(soundPct) / 100.0 * double(MIX_MAX_VOLUME)));
}

void Window::setMusicPct(int pct) {
  musicPct = std::clamp(pct, 0, 100);
  if (!_soundEnabled) {
    return;
  }
  Mix_VolumeMusic(
      static_cast<int>(double(musicPct) / 100.0 * double(MIX_MAX_VOLUME)));
}

void Window::playSound(std::string_view name) {
  if (!_soundEnabled) {
    return;
  }

  auto sound = store.getSound(name);
  const float volumeMult = store.getSoundVolume(name);
  const int channel = Mix_PlayChannel(-1, sound, 0);
  if (channel == -1) {
    LOG(WARN) << "[sdl2w] Unable to play sound in channel.  sound=" << name
              << " err=" << Mix_GetError() << Logger::endl;
    return;
  }
  Mix_Volume(channel,
             static_cast<int>(double(soundPct) / 100.0 * double(volumeMult) *
                              double(MIX_MAX_VOLUME)));
}

void Window::playMusic(std::string_view name) {
  if (!_soundEnabled) {
    return;
  }

  auto music = store.getMusic(name);
  if (music == nullptr) {
    LOG(WARN) << "[sdl2w] Unable to play music.  music=" << name
              << " err=" << SDL_GetError() << Logger::endl;
    return;
  }
  if (Mix_PlayMusic(music, -1) != 0) {
    LOG(WARN) << "[sdl2w] Unable to play music. music=" << name
              << " err=" << Mix_GetError() << Logger::endl;
    return;
  }
  Mix_VolumeMusic(
      static_cast<int>(double(musicPct) / 100.0 * double(MIX_MAX_VOLUME)));
}

void Window::stopMusic() {
  if (Mix_PlayingMusic()) {
    Mix_HaltMusic();
  }
}

bool Window::isMusicPlaying() const {
  if (Mix_PlayingMusic()) {
    return true;
  }
  return false;
}

std::pair<int, int> Window::getDims() const {
  return std::make_pair(windowWidth, windowHeight);
}

std::pair<int, int> Window::getRenderDims() const {
  return std::make_pair(renderWidth, renderHeight);
}

void Window::init(int audioChannels) {
  if (_isInit) {
    LOG(WARN) << "[sdl2w] SDL is already initialized." << Logger::endl;
    return;
  }

  LOG(DEBUG) << "[sdl2w] Init SDL" << Logger::endl;

  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO |
               SDL_INIT_EVENTS) != 0) {
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] SDL could not initialize: ") +
                        SDL_GetError());
  }

  if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
    SDL_Quit();
    THROW_RUNTIME_ERROR(bmin::String("[sdl2w] SDL_image PNG support failed: ") +
                        IMG_GetError());
  }
  _imageInitialized = true;

  // initialize fonts
  if (TTF_Init() < 0) {
    LOG_LINE(ERROR) << "[sdl2w] SDL_ttf could not initialize! "
                    << TTF_GetError() << Logger::endl;
    IMG_Quit();
    _imageInitialized = false;
    SDL_Quit();
    THROW_RUNTIME_ERROR(
        bmin::String(FAIL_ERROR_TEXT.data(), FAIL_ERROR_TEXT.size()));
  }

  _mixerCodecFlags = Mix_Init(MIX_INIT_OGG);
  if ((_mixerCodecFlags & MIX_INIT_OGG) == 0) {
    LOG(WARN) << "[sdl2w] OGG support is unavailable: " << Mix_GetError()
              << Logger::endl;
  }
  const int channels = audioChannels == 1 ? 1 : 2;
  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, channels, 1024) <
      0) {
    LOG(WARN) << "[sdl2w] Audio is unavailable; continuing without sound: "
              << Mix_GetError() << Logger::endl;
    _soundEnabled = false;
  } else {
    _audioInitialized = true;
    _soundEnabled = true;
  }

  _isInit = true;
}

void Window::unInit() {
  if (_isInit) {
    if (_activeWindow != nullptr) {
      LOG(WARN) << "[sdl2w] Destroy the active Window before Window::unInit()"
                << Logger::endl;
      return;
    }
    LOG(DEBUG) << "[sdl2w] UnInit SDL" << Logger::endl;

    if (_audioInitialized)
      Mix_CloseAudio();
    _audioInitialized = false;
    Mix_Quit();
    _mixerCodecFlags = 0;
    TTF_Quit();
    if (_imageInitialized)
      IMG_Quit();
    _imageInitialized = false;
    SDL_Quit();
    _isInit = false;
  }
}

void Window::renderLoop() {
  const Uint64 div = 1000;
  const Uint64 nowMicroSeconds = SDL_GetPerformanceCounter();
  auto freq = SDL_GetPerformanceFrequency();
  if (!static_cast<bool>(freq)) {
    freq = 1;
  }
  now = (nowMicroSeconds * div) / freq;
  if (firstLoop) {
    deltaTime = 16.6666;
  } else {
    deltaTime = static_cast<double>((nowMicroSeconds - lastFrameTime) * div) /
                static_cast<double>(freq);
    if (deltaTime > maxDeltaTime)
      deltaTime = maxDeltaTime;
  }

  lastFrameTime = nowMicroSeconds;
  pastFrameTimes.push(deltaTime);
  while (pastFrameTimes.size() > 10) {
    pastFrameTimes.pop();
  }

  SDL_Event e;
  while (SDL_PollEvent(&e) != 0) {
#ifdef __EMSCRIPTEN__
    if (e.type == SDL_QUIT) {
      LOG(WARN) << "[sdl2w] QUIT is overridden in EMSCRIPTEN" << Logger::endl;
      break;
    }
#else
    if (e.type == SDL_QUIT) {
      isLooping = false;
    } else if (e.type == SDL_WINDOWEVENT &&
               e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
      lastFrameTime = SDL_GetPerformanceCounter();
    } else if (e.type == SDL_WINDOWEVENT &&
               e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
      events.clearInputState();
    } else if (e.type == SDL_WINDOWEVENT &&
               e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
      windowWidth = e.window.data1;
      windowHeight = e.window.data2;
    }
#endif
    if (!_inputEnabled) {
      events.handleEvent(e);
      continue;
    }
    if (e.type == SDL_KEYDOWN) {
      events.keydown(e.key.keysym.sym);
    } else if (e.type == SDL_KEYUP) {
      events.keyup(e.key.keysym.sym);
    } else if (e.type == SDL_MOUSEMOTION) {
      int x = e.motion.x, y = e.motion.y;
      float logicalX = 0, logicalY = 0;
      SDL_RenderWindowToLogical(sdlRenderer, x, y, &logicalX, &logicalY);
      x = static_cast<int>(logicalX);
      y = static_cast<int>(logicalY);
      events.mousemove(x, y);
      mousePos = std::make_pair(x, y);
    } else if (e.type == SDL_MOUSEBUTTONDOWN) {
      int x = e.button.x, y = e.button.y;
      float logicalX = 0, logicalY = 0;
      SDL_RenderWindowToLogical(sdlRenderer, x, y, &logicalX, &logicalY);
      x = static_cast<int>(logicalX);
      y = static_cast<int>(logicalY);
      events.mousedown(x, y, static_cast<int>(e.button.button));
    } else if (e.type == SDL_MOUSEBUTTONUP) {
      int x = e.button.x, y = e.button.y;
      float logicalX = 0, logicalY = 0;
      SDL_RenderWindowToLogical(sdlRenderer, x, y, &logicalX, &logicalY);
      x = static_cast<int>(logicalX);
      y = static_cast<int>(logicalY);
      events.mouseup(x, y, static_cast<int>(e.button.button));
    }
    if (e.type == SDL_MOUSEWHEEL) {
      events.wheel =
          e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -e.wheel.y : e.wheel.y;
      int x, y;
      SDL_GetMouseState(&x, &y);
      float logicalX = 0, logicalY = 0;
      SDL_RenderWindowToLogical(sdlRenderer, x, y, &logicalX, &logicalY);
      x = static_cast<int>(logicalX);
      y = static_cast<int>(logicalY);
      events.mousewheel(x, y, events.wheel);
    } else {
      events.wheel = 0;
    }
    events.handleEvent(e);
  }
  if (!isLooping) {
    return;
  }

  if (initTimeMax > initTime) {
    initTime += deltaTime;
  }

  events.update();
  if (isReady() && initTime >= initTimeMax) {
    if (firstLoop) {
      onInitCb();
      firstLoop = false;
    }
    isLooping = loopCb();
  } else {
    isLooping = initializingCb();
  }

  draw.renderIntermediate();
}

void Window::setInitTimeMax(int max) { initTimeMax = max; }

double Window::getAverageFrameTime() const {
  if (pastFrameTimes.empty())
    return 0.0;
  double total = 0.0;
  bmin::Queue<double> copy = pastFrameTimes;
  while (!copy.empty()) {
    total += copy.front();
    copy.pop();
  }
  return total / static_cast<double>(pastFrameTimes.size());
}

double Window::getFps() const {
  const double average = getAverageFrameTime();
  return average > 0.0 ? 1000.0 / average : 0.0;
}

void Window::setMaxDeltaTime(double ms) {
  if (ms > 0.0)
    maxDeltaTime = ms;
}

void Window::setTitle(std::string_view title) {
  bmin::String value(title.data(), title.size());
  SDL_SetWindowTitle(sdlWindow, value.cStr());
}

void Window::setFullscreen(bool enabled) {
  if (SDL_SetWindowFullscreen(
          sdlWindow, enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
    LOG(WARN) << "[sdl2w] Could not change fullscreen state: " << SDL_GetError()
              << Logger::endl;
  }
}

#ifdef __EMSCRIPTEN__
void RenderLoopCallback(void* arg) { static_cast<Window*>(arg)->renderLoop(); }
#endif
void Window::startRenderLoop(std::function<bool(void)> _initializingCb,
                             std::function<void(void)> _onInitCb,
                             std::function<bool(void)> _loopCb) {
  firstLoop = true;
  isLooping = true;
  initializingCb = _initializingCb;
  loopCb = _loopCb;
  onInitCb = _onInitCb;
  Window::lastFrameTime = SDL_GetPerformanceCounter();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(&RenderLoopCallback, this, -1, 1);
#else
  while (isLooping) {
    renderLoop();
  }
#endif
}

} // namespace sdl2w
