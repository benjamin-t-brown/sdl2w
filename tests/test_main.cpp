#include <cmath>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#ifdef SDL2W_USE_MODULES
import sdl2w;
import bmin.string_interop;
#else
#include "Animation.h"
#include "AssetLoader.h"
#include "Draw.h"
#include "Events.h"
#include "L10n.h"
#include "Logger.h"
#include "Store.h"
#include "Window.h"
#endif

namespace {

int failures = 0;

void check(bool condition, std::string_view message) {
  if (!condition) {
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
  }
}

void testLogger() {
  const char* path =
#ifdef SDL2W_USE_MODULES
      "module-test.log";
#else
      "header-test.log";
#endif
  sdl2w::Logger::colorEnabled = false;
  sdl2w::Logger::disabled = false;
  sdl2w::Logger::setLogLevel(sdl2w::INFO);
  sdl2w::Logger::setLogToFile(true, path);
  sdl2w::log(sdl2w::DEBUG) << "filtered-message" << sdl2w::endl;
  sdl2w::log(sdl2w::INFO) << "visible-message" << sdl2w::endl;
  sdl2w::Logger::setLogToFile(false);
  std::ifstream input(path);
  const std::string contents((std::istreambuf_iterator<char>(input)), {});
  check(contents.find("visible-message") != std::string::npos,
        "logger writes messages at the configured level");
  check(contents.find("filtered-message") == std::string::npos,
        "logger filters messages below the configured level");
  std::remove(path);
  sdl2w::Logger::setLogLevel(sdl2w::DEBUG);
}

void testLocalization() {
  sdl2w::L10n::init({"language-with-no-file"});
  check(std::string_view(sdl2w::L10n::translate("Source text")) ==
            "Source text",
        "missing translation files fall back to source text");
  sdl2w::L10n::loadLanguage("pirate", "[Hello]{Ahoy}\n");
  check(sdl2w::L10n::setLanguage("pirate"),
        "a loaded language can be selected");
  check(std::string_view(sdl2w::L10n::translate("Hello")) == "Ahoy",
        "loaded translations are returned");
  check(std::string_view(sdl2w::L10n::translate("Untranslated")) ==
            "Untranslated",
        "missing entries fall back to source text");
  sdl2w::L10n::setEnabled(false);
  check(std::string_view(sdl2w::L10n::translate("Disabled")) == "Disabled",
        "disabled localization returns the original pointer");
}

void testAnimationAndStore() {
  sdl2w::Sprite first{.name = "first", .renderable = {}};
  sdl2w::Sprite second{.name = "second", .renderable = {}};
  sdl2w::Animation animation("walk", false);
  animation.addSprite({.name = "first", .duration = 10}, first);
  animation.addSprite({.name = "second", .duration = 20}, second);
  animation.update(10.0);
  check(animation.getAnimIndex() == 1,
        "animation advances exactly on a frame boundary");
  animation.update(1000.0);
  check(animation.isFinished() && animation.getAnimIndex() == 1,
        "non-looping animation clamps to its final frame");
  animation.start();
  check(animation.getAnimIndex() == 0 && !animation.isFinished(),
        "animation start resets its frame and completion state");
  animation.update(-1.0);
  check(animation.getAnimIndex() == 0, "negative animation deltas are ignored");

  sdl2w::Store store;
  store.storeSprite("hero", first);
  check(store.hasSprite("hero"), "store exposes resource presence queries");
  bool duplicateRejected = false;
  try {
    store.storeSprite("hero", second);
  } catch (const std::exception&) {
    duplicateRejected = true;
  }
  check(duplicateRejected, "store rejects duplicate resource names");
  auto& definition = store.storeAnimationDefinition("walk", true);
  definition.addSprite("hero", 25);
  auto created = store.createAnimation("walk");
  auto moved = static_cast<sdl2w::Animation&&>(created);
  check(moved.isInitialized(),
        "created animation instances can move without touching templates");
  auto secondInstance = store.createAnimation("walk");
  check(secondInstance.isInitialized() && secondInstance.getAnimIndex() == 0,
        "animation templates create independent instances");
}

void testEvents() {
  sdl2w::Events events;
  int mouseCalls = 0;
  events.setMouseEvent(sdl2w::ON_MOUSE_DOWN, [&](int x, int y, int button) {
    check(x == 3 && y == 4 && button == sdl2w::Events::MOUSE_BUTTON_LEFT,
          "mouse callback receives event coordinates");
    ++mouseCalls;
  });
  events.mousedown(3, 4, sdl2w::Events::MOUSE_BUTTON_LEFT);
  check(mouseCalls == 1 && events.isMouseDown,
        "mouse state and callback update together");
  events.keydown('a');
  check(events.isKeyPressed("A"), "keyboard state records keydown");
  events.clearInputState();
  check(!events.isMouseDown && !events.isKeyPressed("A"),
        "input state clears on lifecycle boundaries");
  check(!events.areControllersEnabled(),
        "controller discovery is opt-in by default");
  events.setControllerDeadZone(2.0);
  check(events.getControllerCount() == 0,
        "controller queries are safe without initialization");
}

void testAssetValidation() {
  sdl2w::Store store;
  sdl2w::Draw draw(store);
  sdl2w::AssetLoader loader(draw, store);
  auto valid = loader.validateAssetsFromFile(sdl2w::ASSET_FILE,
                                             "assets/valid-assets.txt");
  check(valid.success && valid.assetCount == 6,
        "valid asset manifests pass preflight validation");
  auto invalid = loader.validateAssetsFromFile(sdl2w::ASSET_FILE,
                                               "assets/invalid-assets.txt");
  check(!invalid.success && !invalid.issues.empty() &&
            invalid.issues[0].line != 0,
        "invalid asset manifests report line-numbered diagnostics");
}

void testWindowAndText() {
  sdl2w::Window::init();
  check(sdl2w::Window::isInit(), "window subsystem initializes");
  {
    sdl2w::Store store;
    {
      sdl2w::Window window(store,
                           {.mode = sdl2w::CPU,
                            .title = "sdl2w tests",
                            .w = 160,
                            .h = 120,
                            .x = 0,
                            .y = 0,
                            .renderW = 160,
                            .renderH = 120,
                            .vsync = false,
                            .maxDeltaTime = 25.0});
      check(sdl2w::Window::getActiveWindow() == &window,
            "window registers as the single active instance");
      check(!window.getEvents().areControllersEnabled(),
            "window construction does not scan controllers");
      check(window.getAverageFrameTime() == 0.0 && window.getFps() == 0.0,
            "frame statistics are safe before the render loop starts");
      store.loadAndStoreFont("default", "../example/assets/monofonto.ttf");
      check(!store.hasFont("default", 24),
            "registered fonts do not eagerly open every size");
      check(store.getFont("default", 24) != nullptr &&
                store.hasFont("default", 24),
            "font size combinations open and cache on first use");
      check(store.getFont("default", 24, true) != nullptr &&
                store.hasFont("default", 24, true),
            "outlined font combinations are independently cached");
      auto& draw = window.getDraw();
      draw.setTextCacheLimit(2);
      const sdl2w::RenderTextParams textParams{.fontName = "default",
                                               .fontSize = sdl2w::TEXT_SIZE_16,
                                               .x = 1,
                                               .y = 1,
                                               .color = {255, 255, 255, 255}};
      draw.drawText("one", textParams);
      draw.drawText("two", textParams);
      draw.drawText("three", textParams);
      check(draw.getTextCacheSize() == 2,
            "static text cache evicts least-recently-used entries");
      draw.drawDynamicText("score", "1", textParams);
      draw.drawDynamicText("score", "123456", textParams);
      check(draw.getDynamicTextCount() == 1,
            "dynamic text reuses a named streaming texture slot");
      bool secondWindowRejected = false;
      try {
        sdl2w::Store otherStore;
        sdl2w::Window other(otherStore,
                            {.mode = sdl2w::CPU,
                             .title = "second",
                             .w = 1,
                             .h = 1,
                             .x = 0,
                             .y = 0,
                             .renderW = 1,
                             .renderH = 1,
                             .vsync = false});
      } catch (const std::exception&) {
        secondWindowRejected = true;
      }
      check(secondWindowRejected, "the one-window contract is enforced");
      sdl2w::Window::unInit();
      check(sdl2w::Window::isInit(),
            "SDL cannot be shut down while its Window is alive");
    }
    check(sdl2w::Window::getActiveWindow() == nullptr,
          "window destruction clears the active instance");
  }
  sdl2w::Window::unInit();
  check(!sdl2w::Window::isInit(), "window subsystem shuts down cleanly");
}

} // namespace

int main() {
  testLogger();
  testLocalization();
  testAnimationAndStore();
  testEvents();
  testAssetValidation();
  testWindowAndText();
  if (failures != 0) {
    std::cerr << failures << " test(s) failed\n";
    return 1;
  }
#ifdef SDL2W_USE_MODULES
  std::cout << "module API: all behavioral tests passed\n";
#else
  std::cout << "header API: all behavioral tests passed\n";
#endif
  return 0;
}
