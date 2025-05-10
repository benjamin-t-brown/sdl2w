#include "../sdl2w/include/Animation.h"
#include "../sdl2w/include/AssetLoader.h"
#include "../sdl2w/include/Draw.h"
#include "../sdl2w/include/L10n.h"
#include "../sdl2w/include/Logger.h"
#include "../sdl2w/include/Revirtualis.h"
#include "../sdl2w/include/Window.h"

void runProgram(int argc, char** argv) {
  const int w = 640;
  const int h = 480;

  sdl2w::Store store;
  sdl2w::Window window(store,
                       {
                           .mode = sdl2w::DrawMode::GPU,
                           .title = "SDL2W Example",
                           .w = w,
                           .h = h,
                           .x = 25, // SDL_WINDOWPOS_UNDEFINED
                           .y = 50, // SDL_WINDOWPOS_UNDEFINED
                           .renderW = w,
                           .renderH = h,
                       });
  sdl2w::L10n::init(std::vector<std::string>({"en", "la"}));

  // proprietary splash screen
  revirtualis::setupRevirtualis(argc, argv, window);
  revirtualis::showRevirtualisSplash(window);

  window.getDraw().setBackgroundColor({0, 0, 145});

  sdl2w::AssetLoader assetLoader(window.getDraw(), window.getStore());
  window.getStore().loadAndStoreFont("default", "assets/monofonto.ttf");
  window.getStore().loadAndStoreFont("cabal", "assets/cabal.ttf");
  assetLoader.loadAssetsFromFile(sdl2w::ASSET_FILE, "assets/assets.txt");

  window.getStore().logAllSprites();
  window.getStore().logAllAnimationDefinitions();

  sdl2w::Draw& d = window.getDraw();

  int kenAnimState = 0;
  int kenDirection = 1;
  const double kenSpeed = .1;
  double kenX = 75;
  double kenPunchTime = 0;
  double kenPunchDuration = 600;
  std::vector<sdl2w::Animation> kenAnims = {
      store.createAnimation("ken_walk"),
      store.createAnimation("ken_walk", true),
      store.createAnimation("ken_punch"),
      store.createAnimation("ken_punch", true)};

  double fractalRotation = 0;

  std::string lastKeyPressed = "";
  window.getEvents().setKeyboardEvent(
      sdl2w::ON_KEY_DOWN, [&](const std::string& key, int button) {
        LOG(INFO) << "Keyboard down: " << key << " (" << button << ")"
                  << LOG_ENDL;
        if (key == "Left") {
          kenDirection = 0;
        } else if (key == "Right") {
          kenDirection = 1;
        } else if (key == "X") {
          kenAnimState = 1;
          kenAnims[2].start();
          kenAnims[3].start();
        } else if (key == "Space") {
          if (window.isMusicPlaying()) {
            window.stopMusic();
          } else {
            window.playMusic("song_of_time");
          }
        } else if (key == "Left Shift") {
          window.playSound("test1");
        } else if (key == "Left Alt") {
          window.playSound("test2");
        } else if (key == "Left Ctrl") {
          window.playSound("test3");
        }

        lastKeyPressed = key + " (" + std::to_string(button) + ")";
      });

  window.startRenderLoop([&]() {
    const int dt = window.getDeltaTime();

    // update
    for (auto& anim : kenAnims) {
      anim.update(dt);
    }
    if (kenAnimState == 0) {
      kenX += (kenDirection == 0 ? -1 : 1) * kenSpeed * static_cast<double>(dt);
      if (kenX < 0) {
        kenX = 0;
      } else if (kenX > w) {
        kenX = w;
      }
    } else if (kenAnimState == 1) {
      kenPunchTime += static_cast<double>(dt);
      if (kenPunchTime > kenPunchDuration) {
        kenAnimState = 0;
        kenPunchTime = 0;
      }
    }

    fractalRotation += static_cast<double>(dt) / 16.;
    if (fractalRotation > 360.) {
      fractalRotation -= 360.;
    }

    // draw
    sdl2w::RenderableParams kenRenderParams = {.scale = {1., 1.},
                                               .x = static_cast<int>(kenX),
                                               .y = h - 100,
                                               .centered = true};
    if (kenAnimState == 0) {
      // walk left/right
      d.drawAnimation(kenAnims[kenDirection == 0 ? 1 : 0], kenRenderParams);
    } else if (kenAnimState == 1) {
      // punch
      d.drawAnimation(kenAnims[kenDirection == 0 ? 3 : 2], kenRenderParams);
    }

    d.drawSprite(window.getStore().getSprite("fractal_0"),
                 sdl2w::RenderableParamsEx{.scale = {2., 2.},
                                           .angleDeg = fractalRotation,
                                           .x = w / 2,
                                           .y = h / 2,
                                           .centered = true});

    d.drawText(
        TRANSLATE("Welcome to the SDL2W Example!"),
        sdl2w::RenderTextParams{.fontName = "default",
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_24,
                                .x = w / 2,
                                .y = 24,
                                .color = {255, 255, 255},
                                .centered = true});

    d.drawText(
        TRANSLATE("Press Shift Alt or Ctrl to play sounds!"),
        sdl2w::RenderTextParams{.fontName = "default",
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_16,
                                .x = w / 2,
                                .y = h / 2 - 80 * 2,
                                .color = {200, 200, 255},
                                .centered = true});

    d.drawText(
        TRANSLATE("Press Space to start/stop music!"),
        sdl2w::RenderTextParams{.fontName = "default",
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_16,
                                .x = w / 2,
                                .y = h / 2 - 80,
                                .color = {255, 200, 200},
                                .centered = true});

    d.drawText(
        TRANSLATE("Press Left/Right or X to punch!"),
        sdl2w::RenderTextParams{.fontName = "default",
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_16,
                                .x = w / 2,
                                .y = h / 2 + 80,
                                .color = {200, 200, 200},
                                .centered = true});

    d.drawText(
        TRANSLATE("Last key pressed: ") + lastKeyPressed,
        sdl2w::RenderTextParams{.fontName = "default",
                                .fontSize = sdl2w::TextSize::TEXT_SIZE_16,
                                .x = 8,
                                .y = h - 24,
                                .color = {200, 200, 200},
                                .centered = false});

    return true;
  });
}

int main(int argc, char** argv) {
  LOG(INFO) << "Start program" << LOG_ENDL;
  sdl2w::Window::init();
  srand(time(NULL));

  runProgram(argc, argv);

  sdl2w::Window::unInit();
  LOG(INFO) << "End program" << LOG_ENDL;

  return 0;
}