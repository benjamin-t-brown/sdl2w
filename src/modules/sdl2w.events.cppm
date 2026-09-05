module;
#include <functional>
#include <string_view>

#include "impl_headers.h"
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_mouse.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#endif

export module sdl2w.events;
export import bmin.containers;
export import sdl2w.defines;
import sdl2w.logger;
import bmin.string_interop;

export namespace sdl2w {

class EventRoute {
public:
  std::function<void(int, int, int)> onmousedown;
  std::function<void(int, int, int)> onmouseup;
  std::function<void(int, int, int)> onmousemove;
  std::function<void(int, int, int)> onmousewheel;
  std::function<void(std::string_view, int)> onkeydown;
  std::function<void(std::string_view, int)> onkeyup;
  std::function<void(std::string_view, int)> onkeypress;
  std::function<void(int, int)> oncontrollerbuttondown;
  std::function<void(int, int)> oncontrollerbuttonup;
  std::function<void(int, int, double)> oncontrolleraxis;
  std::function<void(int)> oncontrollerconnected;
  std::function<void(int)> oncontrollerdisconnected;

  EventRoute();
};

enum MouseEventCb {
  ON_MOUSE_DOWN,
  ON_MOUSE_UP,
  ON_MOUSE_WHEEL,
  ON_MOUSE_MOVE,
};

enum KeyboardEventCb {
  ON_KEY_DOWN,
  ON_KEY_PRESS,
  ON_KEY_UP,
};

enum ControllerButtonEventCb {
  ON_CONTROLLER_BUTTON_DOWN,
  ON_CONTROLLER_BUTTON_UP,
};

enum ControllerButton {
  CONTROLLER_A,
  CONTROLLER_B,
  CONTROLLER_X,
  CONTROLLER_Y,
  CONTROLLER_BACK,
  CONTROLLER_GUIDE,
  CONTROLLER_START,
  CONTROLLER_LEFT_STICK,
  CONTROLLER_RIGHT_STICK,
  CONTROLLER_LEFT_SHOULDER,
  CONTROLLER_RIGHT_SHOULDER,
  CONTROLLER_DPAD_UP,
  CONTROLLER_DPAD_DOWN,
  CONTROLLER_DPAD_LEFT,
  CONTROLLER_DPAD_RIGHT,
  CONTROLLER_BUTTON_COUNT,
};

enum ControllerAxis {
  CONTROLLER_LEFT_X,
  CONTROLLER_LEFT_Y,
  CONTROLLER_RIGHT_X,
  CONTROLLER_RIGHT_Y,
  CONTROLLER_TRIGGER_LEFT,
  CONTROLLER_TRIGGER_RIGHT,
  CONTROLLER_AXIS_COUNT,
};

class Events {
private:
  struct ControllerState {
    bmin::UniquePtr<SDL_GameController, SDL_Deleter> controller;
    int instanceId = -1;
    bool buttons[CONTROLLER_BUTTON_COUNT]{};
    double axes[CONTROLLER_AXIS_COUNT]{};
  };

  bmin::DynArray<bmin::UniquePtr<EventRoute>> routes;
  bmin::Map<bmin::String, bool> keys;
  bool shouldPushRoute = false;
  bool shouldPopRoute = false;
  std::function<void(SDL_Event)> cb;
  bmin::DynArray<ControllerState> controllers;
  bool controllersEnabled = false;
  double controllerDeadZone = 0.15;

  bmin::UniquePtr<EventRoute>& currentRoute() {
    return routes[routes.size() - 1];
  }
  int controllerPlayerForInstance(int instanceId) const;
  void openController(int deviceIndex);
  void closeController(int instanceId);
  void processControllerEvent(const SDL_Event& event);

public:
  bool isMouseDown = false;
  bool isRightMouseDown = false;
  bool isMiddleMouseDown = false;
  int mouseX = 0;
  int mouseY = 0;
  int mouseDownX = 0;
  int mouseDownY = 0;
  int wheel = 0;

  static constexpr int MOUSE_BUTTON_LEFT = SDL_BUTTON_LEFT;
  static constexpr int MOUSE_BUTTON_MIDDLE = SDL_BUTTON_MIDDLE;
  static constexpr int MOUSE_BUTTON_RIGHT = SDL_BUTTON_RIGHT;

  Events();
  ~Events();
  bool isKeyPressed(std::string_view name) const;
  bool isCtrl() const;

  void pushRoute();
  void pushRouteNextTick();
  void popRoute();
  void popRouteNextTick();
  void setMouseEvent(MouseEventCb mEventCb,
                     std::function<void(int, int, int)> cb);
  void setKeyboardEvent(KeyboardEventCb kEventCb,
                        std::function<void(std::string_view, int)> cb);
  void setControllerButtonEvent(ControllerButtonEventCb event,
                                std::function<void(int, int)> cb);
  void setControllerAxisEvent(std::function<void(int, int, double)> cb);
  void setControllerConnectionEvents(std::function<void(int)> connected,
                                     std::function<void(int)> disconnected);

  void mousedown(int x, int y, int button);
  void mouseup(int x, int y, int button);
  void mousemove(int x, int y);
  void keydown(int key);
  void keyup(int key);
  void mousewheel(int x, int y, int dir);

  void handleEvent(SDL_Event e);
  void setEventHandler(std::function<void(SDL_Event)> cbA);

  bool enableControllers();
  void disableControllers();
  bool areControllersEnabled() const { return controllersEnabled; }
  size_t getControllerCount() const;
  bool isControllerButtonPressed(int player, ControllerButton button) const;
  double getControllerAxis(int player, ControllerAxis axis) const;
  void setControllerDeadZone(double deadZone);
  void clearInputState();

  void update();
};

} // namespace sdl2w
