#include "Events.h"
#include "Logger.h"
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_mouse.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#endif

namespace sdl2w {

auto SDL2WRAPPER_EVENTS_NO_EVENT = [](SDL_Event e) {};

const int Events::MOUSE_BUTTON_LEFT = SDL_BUTTON_LEFT;
const int Events::MOUSE_BUTTON_MIDDLE = SDL_BUTTON_MIDDLE;
const int Events::MOUSE_BUTTON_RIGHT = SDL_BUTTON_RIGHT;

EventRoute::EventRoute() {
  onmousedown = [](int, int, int) {};
  onmouseup = [](int, int, int) {};
  onmousemove = [](int, int, int) {};
  onmousewheel = [](int, int, int) {};
  onkeydown = [](std::string_view, int) {};
  onkeyup = [](std::string_view, int) {};
  onkeypress = [](std::string_view, int) {};
}

Events::Events() {
  cb = SDL2WRAPPER_EVENTS_NO_EVENT;
  pushRoute();
}

Events::~Events() {}

bool Events::isKeyPressed(std::string_view name) const {
  const bmin::String nameStr(name.data(), name.size());
  auto it = const_cast<bmin::Map<bmin::String, bool>&>(keys).find(nameStr);
  if (it == keys.end()) {
    return false;
  }
  return (*it).value;
}

bool Events::isCtrl() const {
  return isKeyPressed("Left Ctrl") || isKeyPressed("Right Ctrl");
}

void Events::pushRoute() { routes.pushBack(bmin::makeUnique<EventRoute>()); }
void Events::pushRouteNextTick() { shouldPushRoute = true; }
void Events::popRoute() {
  if (routes.size() >= 2) {
    routes.popBack();
  } else if (routes.size() == 1) {
    routes.popBack();
    pushRoute();
  }
}

void Events::popRouteNextTick() { shouldPopRoute = true; }

void Events::setMouseEvent(MouseEventCb mEventCb,
                           std::function<void(int, int, int)> cb) {
  bmin::UniquePtr<EventRoute>& route = currentRoute();
  if (mEventCb == ON_MOUSE_DOWN) {
    route->onmousedown = cb;
  } else if (mEventCb == ON_MOUSE_MOVE) {
    route->onmousemove = cb;
  } else if (mEventCb == ON_MOUSE_UP) {
    route->onmouseup = cb;
  } else if (mEventCb == ON_MOUSE_WHEEL) {
    route->onmousewheel = cb;
  } else {
    LOG(WARN) << "[sdl2w] WARNING Cannot set mouse event named: " << mEventCb
              << Logger::endl;
  }
}
void Events::setKeyboardEvent(KeyboardEventCb kEventCb,
                              std::function<void(std::string_view, int)> cb) {
  bmin::UniquePtr<EventRoute>& route = currentRoute();
  if (kEventCb == ON_KEY_DOWN) {
    route->onkeydown = cb;
  } else if (kEventCb == ON_KEY_UP) {
    route->onkeyup = cb;
  } else if (kEventCb == ON_KEY_PRESS) {
    route->onkeypress = cb;
  } else {
    LOG(WARN) << "[sdl2w] WARNING Cannot set keyboard event named: " << kEventCb
              << Logger::endl;
  }
}

void Events::mousedown(int x, int y, int button) {
  bmin::UniquePtr<EventRoute>& route = currentRoute();
  if (button == SDL_BUTTON_LEFT) {
    mouseDownX = x;
    mouseDownY = y;
    isMouseDown = true;
    route->onmousedown(x, y, button);
  } else if (button == SDL_BUTTON_RIGHT) {
    isRightMouseDown = true;
    route->onmousedown(x, y, button);
  } else if (button == SDL_BUTTON_MIDDLE) {
    isMiddleMouseDown = true;
    route->onmousedown(x, y, button);
  }
}
void Events::mouseup(int x, int y, int button) {
  bmin::UniquePtr<EventRoute>& route = currentRoute();
  if (button == SDL_BUTTON_LEFT) {
    route->onmouseup(x, y, button);
    isMouseDown = false;
  } else if (button == SDL_BUTTON_RIGHT) {
    route->onmouseup(x, y, button);
    isRightMouseDown = false;
  } else if (button == SDL_BUTTON_MIDDLE) {
    route->onmouseup(x, y, button);
    isMiddleMouseDown = false;
  }
}
void Events::mousemove(int x, int y) {
  bmin::UniquePtr<EventRoute>& route = currentRoute();
  mouseX = x;
  mouseY = y;
  route->onmousemove(x, y, 0);
}
void Events::mousewheel(int x, int y, int dir) {
  bmin::UniquePtr<EventRoute>& route = currentRoute();
  route->onmousewheel(x, y, dir);
  wheel = dir;
  if (dir > 0) {
    wheel = 1;
  } else if (dir < 0) {
    wheel = -1;
  }
}
void Events::keydown(int key) {
  bmin::UniquePtr<EventRoute>& route = currentRoute();
  const bmin::String k(SDL_GetKeyName(key));
  if (!keys.contains(k) || !keys[k]) {
    keys[k] = true;
    route->onkeydown(k.sliceView(), key);
  }
  route->onkeypress(k.sliceView(), key);
}
void Events::keyup(int key) {
  bmin::UniquePtr<EventRoute>& route = currentRoute();
  const bmin::String k(SDL_GetKeyName(key));
  keys[k] = false;

  route->onkeyup(k.sliceView(), key);
}

void Events::handleEvent(SDL_Event e) { cb(e); }
void Events::setEventHandler(std::function<void(SDL_Event)> cbA) { cb = cbA; };
void Events::update() {
  if (shouldPushRoute) {
    shouldPushRoute = false;
    pushRoute();
  }
  if (shouldPopRoute) {
    shouldPopRoute = false;
    popRoute();
  }
}
} // namespace sdl2w