#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/UniquePtr.h"
#include <string_view>

namespace sdl2w {
struct AnimationDefinition;
struct Sprite;
class Store;

struct AnimSpriteDefinition {
  bmin::String name = "";
  int duration = 100;
};

struct Animation {
  bmin::DynArray<AnimSpriteDefinition> spriteDefinitions;
  // Animation stores its own sprites so it doesn't need access to a Store to
  // render. This makes it have a weak dependency on the Store it was created
  // from. Rust would probably not let you do this because each stored sprite
  // contains a pointer to the SDL_Surface or SDL_Texture it was created from,
  // which is owned by the Store. So if you unload the Store but keep the
  // Animation in memory, you will get a segfault when you attempt to render the
  // animation, because the pointers will no longer be valid.
  bmin::DynArray<Sprite> storedSprites;
  bmin::String name;
  int t;
  int totalDuration;
  int spriteIndex;
  bool loop;
  bool flipped = false;
  static bmin::UniquePtr<Sprite> staticDefaultSprite;

  Animation();
  Animation(std::string_view nameA, const bool loopA);
  ~Animation();
  Animation(const Animation& other);
  Animation& operator=(const Animation& other);
  Animation(const AnimationDefinition& def, Store& store);

  bool isInitialized() const;
  const Sprite& getCurrentSprite() const;
  bmin::String toString() const;
  void addSprite(const AnimSpriteDefinition& def, const Sprite& sprite);
  int getAnimIndex() const;

  void start();
  void update(int dt);
};

struct AnimationDefinition {
  bmin::DynArray<AnimSpriteDefinition> sprites;
  bmin::String name;
  bool loop;
  AnimationDefinition(std::string_view nameA, const bool loopA);

  void addSprite(std::string_view spriteName, int ms);
};
} // namespace sdl2w
