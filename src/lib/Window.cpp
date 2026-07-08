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
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#endif

namespace sdl2w {
bool Window::_soundEnabled = true;
bool Window::_inputEnabled = true;
bool Window::_isInit = false;

Window::Window(Store& store, const Window2Params& params)
    : store(store), draw(store) {
  if (!_isInit) {
    LOG(WARN) << "[sdl2w] SDL is not initialized, so Window cannot be created."
              << Logger::endl;
    return;
  }

  LOG(DEBUG) << "[sdl2w] Create window:"
             << " " << params.w << " " << params.h << Logger::endl;

  // create window and renderer
  sdlWindow = SDL_CreateWindow(params.title.cStr(),
                               params.x,
                               params.y,
                               params.w,
                               params.h,
                               SDL_WINDOW_SHOWN);
  Uint32 rendererFlags = SDL_RENDERER_PRESENTVSYNC;
  rendererFlags |= (params.mode == DrawMode::GPU) ? SDL_RENDERER_ACCELERATED
                                                  : SDL_RENDERER_SOFTWARE;
  sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, rendererFlags);
  SDL_RenderSetLogicalSize(sdlRenderer, params.renderW, params.renderH);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest"); // or "nearest"
  Uint32 format = SDL_GetWindowPixelFormat(sdlWindow);
  draw.setSdlRenderer(sdlRenderer, params.renderW, params.renderH, format);

  Mix_AllocateChannels(numSoundChannels);

  windowWidth = params.w;
  windowHeight = params.h;
  renderWidth = params.renderW;
  renderHeight = params.renderH;

  AssetLoader::initFs();

  emshelpers::setEmscriptenWindow(this);
}

Window::~Window() {}

bool Window::isReady() const { return _isInit && AssetLoader::fsReady; }

void Window::setSoundPct(int pct) {
  soundPct = pct;
  if (!_soundEnabled) {
    return;
  }
  Mix_Volume(
      -1, static_cast<int>(double(soundPct) / 100.0 * double(MIX_MAX_VOLUME)));
}

void Window::setMusicPct(int pct) {
  musicPct = pct;
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
  const int channel = Mix_PlayChannel(-1, sound, 0);
  if (channel == -1) {
    LOG(WARN) << "[sdl2w] Unable to play sound in channel.  sound=" << name
              << " err=" << SDL_GetError() << Logger::endl;
    return;
  }
  Mix_Volume(
      channel,
      static_cast<int>(double(soundPct) / 100.0 * double(MIX_MAX_VOLUME)));
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
  Mix_PlayMusic(music, -1);
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

void Window::init() {
  if (_isInit) {
    LOG(WARN) << "[sdl2w] SDL is already initialized." << Logger::endl;
    return;
  }

  LOG(DEBUG) << "[sdl2w] Init SDL" << Logger::endl;

  // SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO |
  //          SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS);

  SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  // initialize fonts
  if (TTF_Init() < 0) {
    LOG_LINE(ERROR) << "[sdl2w] SDL_ttf could not initialize! "
                    << TTF_GetError() << Logger::endl;
    THROW_RUNTIME_ERROR(
        bmin::String(FAIL_ERROR_TEXT.data(), FAIL_ERROR_TEXT.size()));
  }

  // initialize audio
  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 1, 1024) < 0) {
    LOG_LINE(ERROR) << "[sdl2w] SDL_mixer could not initialize! "
                    << Mix_GetError() << Logger::endl;
    _soundEnabled = false;
  }

  _isInit = true;
}

void Window::unInit() {
  if (_isInit) {
    LOG(DEBUG) << "[sdl2w] UnInit SDL" << Logger::endl;

    TTF_Quit();
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
    _isInit = false;
  }
}

void Window::renderLoop() {
  const Uint64 div = 1000;
  const Uint64 nowMicroSeconds = SDL_GetPerformanceCounter();
  auto freq = SDL_GetPerformanceFrequency();
  now = (nowMicroSeconds * div) / freq;

  if (!static_cast<bool>(freq)) {
    freq = 1;
  }
  if (firstLoop) {
    deltaTime = 16.6666;
  } else {
    deltaTime = static_cast<double>((nowMicroSeconds - lastFrameTime) * div) /
                static_cast<double>(freq);
  }

  lastFrameTime = nowMicroSeconds;
  pastFrameTimes.push(deltaTime);
  while (pastFrameTimes.size() > 10) {
    pastFrameTimes.pop();
  }

  SDL_Event e;
  while (SDL_PollEvent(&e) != 0) {
    // empty event queue
    if (!_inputEnabled) {
      continue;
    }
#ifdef __EMSCRIPTEN__
    if (e.type == SDL_QUIT) {
      LOG(WARN) << "[sdl2w] QUIT is overridden in EMSCRIPTEN" << Logger::endl;
      break;
    }
#else
    if (e.type == SDL_QUIT) {
      isLooping = false;
      break;
    } else if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
      break;
    } else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
      break;
    }
#endif
    else if (e.type == SDL_KEYDOWN) {
      events.keydown(e.key.keysym.sym);
    } else if (e.type == SDL_KEYUP) {
      events.keyup(e.key.keysym.sym);
    } else if (e.type == SDL_MOUSEMOTION) {
      int x, y;
      SDL_GetMouseState(&x, &y);
      events.mousemove(x, y);
      mousePos = std::make_pair(x, y);
    } else if (e.type == SDL_MOUSEBUTTONDOWN) {
      int x, y;
      SDL_GetMouseState(&x, &y);
      events.mousedown(x, y, static_cast<int>(e.button.button));
    } else if (e.type == SDL_MOUSEBUTTONUP) {
      int x, y;
      SDL_GetMouseState(&x, &y);
      events.mouseup(x, y, static_cast<int>(e.button.button));
    }
    if (e.type == SDL_MOUSEWHEEL) {
      events.wheel = e.wheel.y;
      int x, y;
      SDL_GetMouseState(&x, &y);
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
  Window::now = SDL_GetPerformanceCounter();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(&RenderLoopCallback, this, -1, 1);
#else
  while (isLooping) {
    renderLoop();
  }
#endif
}

} // namespace sdl2w