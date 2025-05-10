#pragma once

#include <string>
#include <vector>

namespace sdl2w {
struct AnimationDefinition;
struct Sprite;

struct AnimSpriteDefinition {
  std::string name = "";
  int duration = 100;
};

struct Animation {
  std::vector<AnimSpriteDefinition> spriteDefinitions;
  // Animation stores its own sprites so it doesn't need access to a Store to
  // render. This makes it have a weak dependency on the Store it was created
  // from. Rust would probably not let you do this because each stored sprite
  // contains a pointer to the SDL_Surface or SDL_Texture it was created from,
  // which is owned by the Store. So if you unload the Store but keep the
  // Animation in memory, you will get a segfault when you attempt to render the
  // animation, because the pointers will no longer be valid.
  std::vector<Sprite> storedSprites;
  std::string name;
  int t;
  int totalDuration;
  int spriteIndex;
  bool loop;

  Animation();
  Animation(const std::string& nameA, const bool loopA);
  ~Animation();
  Animation(const Animation& other);
  Animation& operator=(const Animation& other);

  bool isInitialized() const;
  const Sprite& getCurrentSprite() const;
  std::string toString() const;
  void addSprite(const AnimSpriteDefinition& def, const Sprite& sprite);
  int getAnimIndex() const;

  void start();
  void update(int dt);
};

struct AnimationDefinition {
  std::vector<AnimSpriteDefinition> sprites;
  std::string name;
  bool loop;
  AnimationDefinition(const std::string& nameA, const bool loopA);

  void addSprite(const std::string& spriteName, int ms);
};
} // namespace sdl2w