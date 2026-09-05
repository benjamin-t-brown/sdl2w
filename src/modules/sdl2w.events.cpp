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

module sdl2w.events;
import bmin.containers;
import sdl2w.logger;
import bmin.string_interop;

namespace sdl2w {

static auto SDL2WRAPPER_EVENTS_NO_EVENT = [](SDL_Event) {};

EventRoute::EventRoute() {
  onmousedown = [](int, int, int) {};
  onmouseup = [](int, int, int) {};
  onmousemove = [](int, int, int) {};
  onmousewheel = [](int, int, int) {};
  onkeydown = [](std::string_view, int) {};
  onkeyup = [](std::string_view, int) {};
  onkeypress = [](std::string_view, int) {};
  oncontrollerbuttondown = [](int, int) {};
  oncontrollerbuttonup = [](int, int) {};
  oncontrolleraxis = [](int, int, double) {};
  oncontrollerconnected = [](int) {};
  oncontrollerdisconnected = [](int) {};
}

Events::Events() {
  cb = SDL2WRAPPER_EVENTS_NO_EVENT;
  pushRoute();
}

Events::~Events() { disableControllers(); }

bool Events::isKeyPressed(std::string_view name) const {
  auto it = keys.find(name);
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

void Events::setControllerButtonEvent(ControllerButtonEventCb event,
                                      std::function<void(int, int)> callback) {
  auto& route = currentRoute();
  if (event == ON_CONTROLLER_BUTTON_DOWN) {
    route->oncontrollerbuttondown = callback;
  } else if (event == ON_CONTROLLER_BUTTON_UP) {
    route->oncontrollerbuttonup = callback;
  } else {
    LOG(WARN) << "[sdl2w] Cannot set controller button event: " << event
              << Logger::endl;
  }
}

void Events::setControllerAxisEvent(
    std::function<void(int, int, double)> callback) {
  currentRoute()->oncontrolleraxis = callback;
}

void Events::setControllerConnectionEvents(
    std::function<void(int)> connected, std::function<void(int)> disconnected) {
  currentRoute()->oncontrollerconnected = connected;
  currentRoute()->oncontrollerdisconnected = disconnected;
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

namespace {
int controllerButtonIndex(Uint8 button) {
  return button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT ? static_cast<int>(button)
                                                    : -1;
}
int controllerAxisIndex(Uint8 axis) {
  return axis <= SDL_CONTROLLER_AXIS_TRIGGERRIGHT ? static_cast<int>(axis) : -1;
}
} // namespace

int Events::controllerPlayerForInstance(int instanceId) const {
  for (size_t i = 0; i < controllers.size(); ++i) {
    if (controllers[i].controller && controllers[i].instanceId == instanceId) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

void Events::openController(int deviceIndex) {
  if (!controllersEnabled || !SDL_IsGameController(deviceIndex))
    return;
  bmin::UniquePtr<SDL_GameController, SDL_Deleter> controller(
      SDL_GameControllerOpen(deviceIndex));
  if (!controller) {
    LOG(WARN) << "[sdl2w] Could not open controller " << deviceIndex << ": "
              << SDL_GetError() << Logger::endl;
    return;
  }
  const int instanceId =
      SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller.get()));
  if (controllerPlayerForInstance(instanceId) >= 0)
    return;
  size_t player = controllers.size();
  for (size_t i = 0; i < controllers.size(); ++i) {
    if (!controllers[i].controller) {
      player = i;
      break;
    }
  }
  ControllerState state;
  state.controller = bmin::move(controller);
  state.instanceId = instanceId;
  if (player == controllers.size())
    controllers.pushBack(bmin::move(state));
  else
    controllers[player] = bmin::move(state);
  currentRoute()->oncontrollerconnected(static_cast<int>(player));
}

void Events::closeController(int instanceId) {
  const int player = controllerPlayerForInstance(instanceId);
  if (player < 0)
    return;
  controllers[static_cast<size_t>(player)] = ControllerState{};
  currentRoute()->oncontrollerdisconnected(player);
}

void Events::processControllerEvent(const SDL_Event& event) {
  if (!controllersEnabled)
    return;
  if (event.type == SDL_CONTROLLERDEVICEADDED) {
    openController(event.cdevice.which);
    return;
  }
  if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
    closeController(event.cdevice.which);
    return;
  }
  if (event.type == SDL_CONTROLLERBUTTONDOWN ||
      event.type == SDL_CONTROLLERBUTTONUP) {
    const int player = controllerPlayerForInstance(event.cbutton.which);
    const int button = controllerButtonIndex(event.cbutton.button);
    if (player < 0 || button < 0)
      return;
    const bool pressed = event.type == SDL_CONTROLLERBUTTONDOWN;
    controllers[static_cast<size_t>(player)].buttons[button] = pressed;
    if (pressed)
      currentRoute()->oncontrollerbuttondown(player, button);
    else
      currentRoute()->oncontrollerbuttonup(player, button);
    return;
  }
  if (event.type == SDL_CONTROLLERAXISMOTION) {
    const int player = controllerPlayerForInstance(event.caxis.which);
    const int axis = controllerAxisIndex(event.caxis.axis);
    if (player < 0 || axis < 0)
      return;
    const int raw = event.caxis.value;
    double value = raw < 0 ? static_cast<double>(raw) / 32768.0
                           : static_cast<double>(raw) / 32767.0;
    if (axis == CONTROLLER_TRIGGER_LEFT || axis == CONTROLLER_TRIGGER_RIGHT)
      value = raw <= 0 ? 0.0 : static_cast<double>(raw) / 32767.0;
    if (value > -controllerDeadZone && value < controllerDeadZone)
      value = 0.0;
    controllers[static_cast<size_t>(player)].axes[axis] = value;
    currentRoute()->oncontrolleraxis(player, axis, value);
  }
}

void Events::handleEvent(SDL_Event e) {
  processControllerEvent(e);
  cb(e);
}
void Events::setEventHandler(std::function<void(SDL_Event)> cbA) { cb = cbA; };

bool Events::enableControllers() {
  if (controllersEnabled)
    return true;
  const Uint64 started = SDL_GetTicks64();
  if (SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
    LOG(WARN) << "[sdl2w] Could not initialize controllers: " << SDL_GetError()
              << Logger::endl;
    return false;
  }
  controllersEnabled = true;
  const int count = SDL_NumJoysticks();
  for (int i = 0; i < count; ++i)
    openController(i);
  LOG(DEBUG) << "[sdl2w] Controller support enabled in "
             << static_cast<size_t>(SDL_GetTicks64() - started) << "ms"
             << Logger::endl;
  return true;
}

void Events::disableControllers() {
  if (!controllersEnabled)
    return;
  controllers.clear();
  controllersEnabled = false;
  SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
}

size_t Events::getControllerCount() const {
  size_t count = 0;
  for (size_t i = 0; i < controllers.size(); ++i)
    if (controllers[i].controller)
      ++count;
  return count;
}

bool Events::isControllerButtonPressed(int player,
                                       ControllerButton button) const {
  return player >= 0 && static_cast<size_t>(player) < controllers.size() &&
         controllers[static_cast<size_t>(player)].controller && button >= 0 &&
         button < CONTROLLER_BUTTON_COUNT &&
         controllers[static_cast<size_t>(player)].buttons[button];
}

double Events::getControllerAxis(int player, ControllerAxis axis) const {
  if (player < 0 || static_cast<size_t>(player) >= controllers.size() ||
      !controllers[static_cast<size_t>(player)].controller || axis < 0 ||
      axis >= CONTROLLER_AXIS_COUNT)
    return 0.0;
  return controllers[static_cast<size_t>(player)].axes[axis];
}

void Events::setControllerDeadZone(double deadZone) {
  controllerDeadZone =
      deadZone < 0.0 ? 0.0 : (deadZone > 0.95 ? 0.95 : deadZone);
}

void Events::clearInputState() {
  keys.clear();
  isMouseDown = isRightMouseDown = isMiddleMouseDown = false;
  wheel = 0;
  for (size_t i = 0; i < controllers.size(); ++i) {
    for (int button = 0; button < CONTROLLER_BUTTON_COUNT; ++button)
      controllers[i].buttons[button] = false;
    for (int axis = 0; axis < CONTROLLER_AXIS_COUNT; ++axis)
      controllers[i].axes[axis] = 0.0;
  }
}
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
