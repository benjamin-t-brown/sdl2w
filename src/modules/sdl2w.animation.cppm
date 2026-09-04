module;
#include <string_view>

#include "impl_headers.h"
#include "macros.h"

export module sdl2w.animation;
export import sdl2w.types;
export import bmin.containers;
import sdl2w.logger;
import bmin.string_interop;

export namespace sdl2w {

struct AnimationDefinition;

struct AnimSpriteDefinition {
  bmin::String name = "";
  int duration = 100;
};

struct Animation {
  bmin::DynArray<AnimSpriteDefinition> spriteDefinitions;
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

}
