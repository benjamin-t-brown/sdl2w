/*
This method of localization is taken from
https://gamedev.stackexchange.com/questions/168362/handling-localization-strings-in-a-performant-game-engine-c

It uses a hash map to store the strings, and a constexpr hash function to
generate the keys.

You would use this as so:

drawText(TRANSLATE("Example Text")), {});
**/

#pragma once

#include "bmin/DynArray.h"
#include "bmin/Map.h"
#include "bmin/String.h"
#include <initializer_list>
#include <string_view>

namespace sdl2w {

#define TRANSLATE(text)                                                        \
  (sdl2w::L10n::isEnabled()                                                    \
       ? sdl2w::L10n::trans(sdl2w::L10n::hash(text)).cStr()                    \
       : (text))
#define DISABLE_TRANSLATIONS "default"

class L10n {
  static bmin::String language;
  static bmin::Map<bmin::String, bmin::Map<size_t, bmin::String>> locStrings;
  static bmin::DynArray<bmin::String> supportedLanguages;
  static bool enabledFlag;

public:
  static void init(std::initializer_list<std::string_view> langs = {"en"});
  static void loadLanguage(std::string_view lang, std::string_view langText);
  static void setLanguage(std::string_view lang);
  static void setEnabled(bool enabled);
  static bool isEnabled();
  static const bmin::Map<size_t, bmin::String>& getStrings();
  static bmin::String trans(size_t id);
  static size_t hash(std::string_view str);
};

} // namespace sdl2w
