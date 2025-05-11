/*
This method of localization is taken from
https://gamedev.stackexchange.com/questions/168362/handling-localization-strings-in-a-performant-game-engine-c

It uses a hash map to store the strings, and a constexpr hash function to
generate the keys.

You would use this as so:

drawText(TRANSLATE("Example Text")), {});
**/

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace sdl2w {

#define TRANSLATE(text) sdl2w::L10n::trans(sdl2w::L10n::hash(text))

class L10n {
  static std::string language;
  static std::unordered_map<std::string,
                            std::unordered_map<size_t, std::string>>
      locStrings;
  static std::vector<std::string> supportedLanguages;

public:
  static void init(const std::vector<std::string>& langs = {"en"});
  static void loadLanguage(const std::string& lang,
                           const std::string& langFile);
  static void setLanguage(const std::string& lang);
  static bool isEnabled();
  static const std::unordered_map<size_t, std::string>& getStrings();
  static std::string trans(size_t id);
  static size_t hash(std::string_view str);
};

} // namespace sdl2w
