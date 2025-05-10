#pragma once

#include "Draw.h"
#include "Events.h"
#include "Store.h"
#include <deque>
#include <functional>
#include <string>

namespace sdl2w {

struct Window2Params {
  DrawMode mode = DrawMode::CPU;
  std::string title;
  int w;
  int h;
  int x;
  int y;
  int renderW;
  int renderH;
};

class Window {
  Store& store;
  Draw draw;
  Events events;
  std::deque<double> pastFrameTimes;
  std::function<bool(void)> renderCb;

  std::pair<int, int> mousePos;
  Uint64 now;
  uint64_t lastFrameTime = 0;
  double deltaTime = 0.;
  SDL_Window* sdlWindow = nullptr;
  SDL_Renderer* sdlRenderer = nullptr;
  int windowWidth = 0;
  int windowHeight = 0;
  int soundPct = 100;
  int numSoundChannels = 16;
  bool firstLoop = true;
  bool isLooping = false;

  static bool _soundEnabled;
  static bool _isInit;

public:
  static bool isInit();
  static void init();
  static void unInit();

  Window(Store& store, const Window2Params& params);
  ~Window();

  Draw& getDraw() { return draw; }
  Store& getStore() { return store; }
  Events& getEvents() { return events; }
  void setSoundPct(int pct);
  int getSoundPct() const { return soundPct; }
  void playSound(const std::string& name);
  void playMusic(const std::string& name);
  void stopMusic();
  bool isMusicPlaying() const;
  std::pair<int, int> getDims() const;
  int getDeltaTime() const { return static_cast<int>(deltaTime); }

  void renderLoop();
  void startRenderLoop(std::function<bool(void)> cb);
};

} // namespace sdl2w