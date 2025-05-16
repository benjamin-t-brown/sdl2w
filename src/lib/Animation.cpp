#include "Animation.h"
#include "Draw.h"
#include "Logger.h"
#include "Store.h"

namespace sdl2w {

Animation::Animation()
    : name(""), t(0), totalDuration(0), spriteIndex(0), loop(true) {}

Animation::Animation(const std::string& nameA, const bool loopA)
    : name(nameA), t(0), totalDuration(0), spriteIndex(0), loop(loopA) {}

Animation::Animation(const Animation& other)
    : spriteDefinitions(other.spriteDefinitions),
      storedSprites(other.storedSprites),
      name(other.name),
      t(other.t),
      totalDuration(other.totalDuration),
      spriteIndex(other.spriteIndex),
      loop(other.loop) {}

Animation& Animation::operator=(const Animation& other) {
  if (this == &other) {
    return *this;
  }

  spriteDefinitions = other.spriteDefinitions;
  storedSprites = other.storedSprites;
  name = other.name;
  t = other.t;
  totalDuration = other.totalDuration;
  spriteIndex = other.spriteIndex;
  loop = other.loop;

  return *this;
}

Animation::Animation(const AnimationDefinition& def, Store& store)
    : name(""), t(0), totalDuration(0), spriteIndex(0), loop(true) {
  for (const auto& spriteDef : def.sprites) {
    Sprite& sprite = store.getSprite(spriteDef.name);
    addSprite(spriteDef, sprite);
  }
  name = def.name;
  loop = def.loop;
}

Animation::~Animation() {}
bool Animation::isInitialized() const {
  return spriteDefinitions.size() > 0 && storedSprites.size() > 0;
}

const Sprite& Animation::getCurrentSprite() const {
  if (spriteIndex < static_cast<int>(storedSprites.size())) {
    return storedSprites[spriteIndex];
  } else {
    LOG_LINE(ERROR) << "Cannot get current sprite because spriteIndex is out "
                       "of bounds: "
                    << spriteIndex << " (animation=" << name << ")"
                    << Logger::endl;
    return storedSprites[0];
  }
}

std::string Animation::toString() const {
  const std::string spriteName = getCurrentSprite().name;
  return name + " " + spriteName;
}

void Animation::addSprite(const AnimSpriteDefinition& def,
                          const Sprite& sprite) {
  spriteDefinitions.push_back(def);
  storedSprites.push_back(sprite);
  totalDuration += def.duration;
}

int Animation::getAnimIndex() const {
  const unsigned int numSprites = spriteDefinitions.size();
  if (numSprites > 0) {
    unsigned int offsetDuration = t;
    unsigned int currentDuration = 0;
    for (unsigned int i = 0; i < numSprites; i++) {
      currentDuration += spriteDefinitions[i].duration;
      if (offsetDuration < currentDuration) {
        return i;
      }
    }
    return numSprites - 1;
  } else {
    return 0;
  }
}

void Animation::start() { t = 0; }

void Animation::update(int dt) {
  if (spriteDefinitions.size()) {
    t += dt;
    if (loop && t > totalDuration) {
      spriteIndex = 0;
      if (totalDuration > 0) {
        t = t % totalDuration;
      } else {
        t = 0;
      }
    }
    spriteIndex = getAnimIndex();
  }
}

AnimationDefinition::AnimationDefinition(const std::string& nameA,
                                         const bool loopA)
    : name(nameA), loop(loopA) {}

void AnimationDefinition::addSprite(const std::string& spriteName, int ms) {
  std::string localSpriteName = spriteName;
  sprites.push_back(AnimSpriteDefinition{spriteName, ms});
}

} // namespace sdl2w
