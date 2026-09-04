#include "Animation.h"
#include "Draw.h"
#include "Logger.h"
#include "Store.h"

namespace sdl2w {

bmin::UniquePtr<Sprite> Animation::staticDefaultSprite =
    bmin::makeUnique<Sprite>(Sprite{
        .name = "staticDefaultSprite",
        .renderable = Renderable{.tex = nullptr, .surf = nullptr},
    });

Animation::Animation()
    : name(""), t(0), totalDuration(0), spriteIndex(0), loop(true) {}

Animation::Animation(std::string_view nameA, const bool loopA)
    : name(nameA.data(), nameA.size()),
      t(0),
      totalDuration(0),
      spriteIndex(0),
      loop(loopA) {}

Animation::Animation(const Animation& other)
    : spriteDefinitions(other.spriteDefinitions),
      storedSprites(other.storedSprites),
      name(other.name),
      t(other.t),
      totalDuration(other.totalDuration),
      spriteIndex(other.spriteIndex),
      loop(other.loop),
      flipped(other.flipped) {}

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
  flipped = other.flipped;

  return *this;
}

Animation::Animation(const AnimationDefinition& def, Store& store)
    : name(""), t(0), totalDuration(0), spriteIndex(0), loop(true) {
  for (size_t i = 0; i < def.sprites.size(); ++i) {
    Sprite& sprite = store.getSprite(def.sprites[i].name.sliceView());
    addSprite(def.sprites[i], sprite);
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
    return storedSprites[static_cast<size_t>(spriteIndex)];
  } else {
    LOG_LINE(ERROR) << "Cannot get current sprite because spriteIndex is out "
                       "of bounds: "
                    << spriteIndex << " (animation=" << name << ")"
                    << Logger::endl;
    if (storedSprites.size() > 0) {
      return storedSprites[0];
    } else {
      return *staticDefaultSprite;
    }
  }
}

bmin::String Animation::toString() const {
  const bmin::String spriteName = getCurrentSprite().name;
  return name + " " + spriteName;
}

void Animation::addSprite(const AnimSpriteDefinition& def,
                          const Sprite& sprite) {
  spriteDefinitions.pushBack(def);
  storedSprites.pushBack(sprite);
  totalDuration += def.duration;
}

int Animation::getAnimIndex() const {
  const size_t numSprites = spriteDefinitions.size();
  if (numSprites > 0) {
    unsigned int offsetDuration = static_cast<unsigned int>(t);
    unsigned int currentDuration = 0;
    for (size_t i = 0; i < numSprites; i++) {
      currentDuration += static_cast<unsigned int>(spriteDefinitions[i].duration);
      if (offsetDuration < currentDuration) {
        return static_cast<int>(i);
      }
    }
    return static_cast<int>(numSprites - 1);
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

AnimationDefinition::AnimationDefinition(std::string_view nameA,
                                         const bool loopA)
    : name(nameA.data(), nameA.size()), loop(loopA) {}

void AnimationDefinition::addSprite(std::string_view spriteName, int ms) {
  AnimSpriteDefinition def;
  def.name = bmin::String(spriteName.data(), spriteName.size());
  def.duration = ms;
  sprites.pushBack(def);
}

} // namespace sdl2w
