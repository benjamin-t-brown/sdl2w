#include "Events.h"
#include "Logger.h"
#if defined(MIYOOA30) || defined(MIYOOMINI)
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
  onkeydown = [](const std::string&, int) {};
  onkeyup = [](const std::string&, int) {};
  onkeypress = [](const std::string&, int) {};
}

Events::Events() {
  cb = SDL2WRAPPER_EVENTS_NO_EVENT;
  pushRoute();
}

Events::~Events() {}

bool Events::isKeyPressed(const std::string& name) const {
  if (keys.find(name) == keys.end()) {
    return false;
  } else {
    return keys.at(name);
  }
}

bool Events::isCtrl() const {
  return isKeyPressed("Left Ctrl") || isKeyPressed("Right Ctrl");
}

void Events::pushRoute() { routes.push(std::make_unique<EventRoute>()); }
void Events::pushRouteNextTick() { shouldPushRoute = true; }
void Events::popRoute() {
  if (routes.size() >= 2) {
    routes.pop();
  } else if (routes.size() == 1) {
    routes.pop();
    pushRoute();
  }
}

void Events::popRouteNextTick() { shouldPopRoute = true; }

void Events::setMouseEvent(MouseEventCb mEventCb,
                           std::function<void(int, int, int)> cb) {
  std::unique_ptr<EventRoute>& route = routes.top();
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
                              std::function<void(const std::string&, int)> cb) {
  std::unique_ptr<EventRoute>& route = routes.top();
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
  std::unique_ptr<EventRoute>& route = routes.top();
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
  std::unique_ptr<EventRoute>& route = routes.top();
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
  std::unique_ptr<EventRoute>& route = routes.top();
  mouseX = x;
  mouseY = y;
  route->onmousemove(x, y, 0);
}
void Events::mousewheel(int x, int y, int dir) {
  std::unique_ptr<EventRoute>& route = routes.top();
  route->onmousewheel(x, y, dir);
  wheel = dir;
  if (dir > 0) {
    wheel = 1;
  } else if (dir < 0) {
    wheel = -1;
  }
}
void Events::keydown(int key) {
  std::unique_ptr<EventRoute>& route = routes.top();
  const std::string k = std::string(SDL_GetKeyName(key));
  if (!keys[k]) {
    keys[k] = true;
    route->onkeydown(k, key);
  }
  route->onkeypress(k, key);
}
void Events::keyup(int key) {
  std::unique_ptr<EventRoute>& route = routes.top();
  const std::string k = std::string(SDL_GetKeyName(key));
  keys[k] = false;

  route->onkeyup(k, key);
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