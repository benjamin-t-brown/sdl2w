#include "Window.h"
#include "Defines.h"
#include "Logger.h"

#if defined(MIYOOA30) || defined(MIYOOMINI)
#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_surface.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#endif

namespace sdl2w {
bool Window::_soundEnabled = true;
bool Window::_isInit = false;

Window::Window(Store& store, const Window2Params& params)
    : store(store), draw(sdl2w::Draw(params.mode, store)) {
  if (!_isInit) {
    LOG(WARN) << "[sdl2w] SDL is not initialized, so Window cannot be created."
              << Logger::endl;
    return;
  }

  LOG_LINE(DEBUG) << "[sdl2w] Create window:" << " " << params.w << " "
                  << params.h << Logger::endl;

  // create window and renderer
  sdlWindow = SDL_CreateWindow(params.title.c_str(),
                               params.x,
                               params.y,
                               params.w,
                               params.h,
                               SDL_WINDOW_SHOWN);
  sdlRenderer = SDL_CreateRenderer(
      sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  SDL_RenderSetLogicalSize(sdlRenderer, params.renderW, params.renderH);
  Uint32 format = SDL_GetWindowPixelFormat(sdlWindow);
  draw.setSdlRenderer(sdlRenderer, params.renderW, params.renderH, format);

  Mix_AllocateChannels(numSoundChannels);
}

Window::~Window() {}

void Window::setSoundPct(int pct) { soundPct = pct; }

void Window::playSound(const std::string& name) {
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

void Window::playMusic(const std::string& name) {
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
      static_cast<int>(double(soundPct) / 100.0 * double(MIX_MAX_VOLUME)));
}

void Window::stopMusic() {
  if (!_soundEnabled) {
    return;
  }

  if (Mix_PlayingMusic()) {
    Mix_HaltMusic();
  }
}

bool Window::isMusicPlaying() const {
  if (!_soundEnabled) {
    return false;
  }

  if (Mix_PlayingMusic()) {
    return true;
  }
  return false;
}

std::pair<int, int> Window::getDims() const {
  return std::make_pair(windowWidth, windowHeight);
}

void Window::init() {
  if (_isInit) {
    LOG(WARN) << "[sdl2w] SDL is already initialized." << Logger::endl;
    return;
  }

  LOG_LINE(DEBUG) << "[sdl2w] Init SDL" << Logger::endl;

  SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO |
           SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS);

  // initialize fonts
  if (TTF_Init() < 0) {
    LOG_LINE(ERROR) << "[sdl2w] SDL_ttf could not initialize! "
                    << std::string(TTF_GetError()) << Logger::endl;
    throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
  }

  // initialize audio
  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 1, 4096) < 0) {
    LOG_LINE(ERROR) << "[sdl2w] SDL_mixer could not initialize! "
                    << std::string(Mix_GetError()) << Logger::endl;
    _soundEnabled = false;
  }

  _isInit = true;
}

void Window::unInit() {
  if (_isInit) {
    LOG_LINE(DEBUG) << "[sdl2w] UnInit SDL" << Logger::endl;

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
    firstLoop = false;
    pastFrameTimes.push_back(deltaTime);
  } else {
    deltaTime = static_cast<double>((nowMicroSeconds - lastFrameTime) * div) /
                static_cast<double>(freq);
  }

  lastFrameTime = nowMicroSeconds;
  pastFrameTimes.push_back(deltaTime);
  if (pastFrameTimes.size() > 10) {
    pastFrameTimes.pop_front();
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

  events.update();
  isLooping = renderCb();
  draw.renderIntermediate();
  firstLoop = false;
}

#ifdef __EMSCRIPTEN__
void RenderLoopCallback(void* arg) { static_cast<Window*>(arg)->renderLoop(); }
#endif
void Window::startRenderLoop(std::function<bool(void)> cb) {
  firstLoop = true;
  isLooping = true;
  renderCb = cb;
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