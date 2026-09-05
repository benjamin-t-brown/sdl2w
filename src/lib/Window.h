#pragma once

#include "Draw.h"
#include "Events.h"
#include "Store.h"
#include "bmin/DynArray.h"
#include "bmin/Queue.h"
#include "bmin/String.h"
#include <functional>
#include <string_view>

namespace sdl2w {

constexpr const char* FONT_DEFAULT = "default";

struct Window2Params {
  DrawMode mode = DrawMode::GPU;
  bmin::String title;
  int w;
  int h;
  int x;
  int y;
  int renderW;
  int renderH;
  bool vsync = true;
  bool resizable = false;
  bool borderless = false;
  bool fullscreen = false;
  double maxDeltaTime = 100.0;
};

struct ExternalEvent {
  int event;
  bmin::String payload;
};

class Window {
  Store& store;
  Draw draw;
  Events events;
  bmin::Queue<double> pastFrameTimes;
  std::function<bool(void)> initializingCb;
  std::function<void(void)> onInitCb;
  std::function<bool(void)> loopCb;
  bmin::DynArray<ExternalEvent> externalEvents;

  std::pair<int, int> mousePos;
  uint64_t now;
  uint64_t lastFrameTime = 0;
  double deltaTime = 0.;
  SDL_Window* sdlWindow = nullptr;
  SDL_Renderer* sdlRenderer = nullptr;
  int windowWidth = 0;
  int windowHeight = 0;
  int renderWidth = 0;
  int renderHeight = 0;
  int soundPct = 100;
  int musicPct = 100;
  int numSoundChannels = 16;
  double initTime = 0;
  double initTimeMax = 500;
  double maxDeltaTime = 100.0;
  bool firstLoop = true;
  bool isLooping = false;

  static bool _isInit;
  static bool _audioInitialized;
  static bool _imageInitialized;
  static int _mixerCodecFlags;
  static Window* _activeWindow;

public:
  static bool isInit();
  static void init(int audioChannels = 2);
  static void unInit();
  static Window* getActiveWindow() { return _activeWindow; }
  static bool _soundEnabled;
  static bool _inputEnabled;

  Window(Store& store, const Window2Params& params);
  ~Window();
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
  Window(Window&&) = delete;
  Window& operator=(Window&&) = delete;

  Draw& getDraw() { return draw; }
  Store& getStore() { return store; }
  Events& getEvents() { return events; }
  void pushExternalEvent(int event, std::string_view payload) {
    externalEvents.pushBack(
        {event, bmin::String(payload.data(), payload.size())});
  }
  void
  processExternalEvents(std::function<void(int, std::string_view)> callback) {
    for (size_t i = 0; i < externalEvents.size(); ++i) {
      callback(externalEvents[i].event, externalEvents[i].payload.sliceView());
    }
    externalEvents.clear();
  }
  bool isReady() const;
  void setSoundPct(int pct);
  int getSoundPct() const { return soundPct; }
  void setMusicPct(int pct);
  int getMusicPct() const { return musicPct; }
  void playSound(std::string_view name);
  void playMusic(std::string_view name);
  void stopMusic();
  bool isMusicPlaying() const;
  std::pair<int, int> getDims() const;
  std::pair<int, int> getRenderDims() const;
  double getDeltaTime() const { return deltaTime; }
  double getAverageFrameTime() const;
  double getFps() const;
  void setMaxDeltaTime(double ms);
  void setTitle(std::string_view title);
  void setFullscreen(bool enabled);
  bool enableControllers() { return events.enableControllers(); }
  void disableControllers() { events.disableControllers(); }

  void renderLoop();
  void setInitTimeMax(int max);
  void startRenderLoop(std::function<bool(void)> _initializingCb,
                       std::function<void(void)> _onInitCb,
                       std::function<bool(void)> _loopCb);
};

} // namespace sdl2w
