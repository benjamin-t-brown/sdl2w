#pragma once

#include <functional>
#include <map>
#include <memory>
#include <stack>
#include <string>

union SDL_Event;

namespace sdl2w {

class EventRoute {
public:
  std::function<void(int, int, int)> onmousedown;
  std::function<void(int, int, int)> onmouseup;
  std::function<void(int, int, int)> onmousemove;
  std::function<void(int, int, int)> onmousewheel;
  std::function<void(const std::string&, int)> onkeydown;
  std::function<void(const std::string&, int)> onkeyup;
  std::function<void(const std::string&, int)> onkeypress;

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
  ON_KEY_PRESS, // can have repeats
  ON_KEY_UP,
};

class Events {
private:
  std::stack<std::unique_ptr<EventRoute>> routes;
  std::map<std::string, bool> keys;
  bool shouldPushRoute = false;
  bool shouldPopRoute = false;
  std::function<void(SDL_Event)> cb;

public:
  bool isMouseDown = false;
  bool isRightMouseDown = false;
  bool isMiddleMouseDown = false;
  int mouseX = 0;
  int mouseY = 0;
  int mouseDownX = 0;
  int mouseDownY = 0;
  int wheel = 0;

  static const int MOUSE_BUTTON_LEFT;
  static const int MOUSE_BUTTON_MIDDLE;
  static const int MOUSE_BUTTON_RIGHT;

  Events();
  ~Events();
  bool isKeyPressed(const std::string& name) const;
  bool isCtrl() const;

  void pushRoute();
  void pushRouteNextTick();
  void popRoute();
  void popRouteNextTick();
  void setMouseEvent(MouseEventCb mEventCb,
                     std::function<void(int, int, int)> cb);
  void setKeyboardEvent(KeyboardEventCb kEventCb,
                        std::function<void(const std::string&, int)> cb);

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
} // namespace sdl2w