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

#include "macros.h"

export module sdl2w.events;
export import bmin.containers;
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

class Events {
private:
  bmin::DynArray<bmin::UniquePtr<EventRoute>> routes;
  bmin::Map<bmin::String, bool> keys;
  bool shouldPushRoute = false;
  bool shouldPopRoute = false;
  std::function<void(SDL_Event)> cb;

  bmin::UniquePtr<EventRoute>& currentRoute() {
    return routes[routes.size() - 1];
  }

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

  void mousedown(int x, int y, int button);
  void mouseup(int x, int y, int button);
  void mousemove(int x, int y);
  void keydown(int key);
  void keyup(int key);
  void mousewheel(int x, int y, int dir);

  void handleEvent(SDL_Event e);
  void setEventHandler(std::function<void(SDL_Event)> cbA);

  void update();
};

}
