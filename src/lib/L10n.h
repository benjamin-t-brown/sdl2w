/*
This method of localization is taken from
https://gamedev.stackexchange.com/questions/168362/handling-localization-strings-in-a-performant-game-engine-c

It uses a hash map to store the strings, and a constexpr hash function to
generate the keys.

You would use this as so:

drawText(L10n::trans(LOCSTR(<message>)), 0, 0);
**/

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace sdl2w {

#ifdef SDL2W_ENABLE_L10N
#define LOCSTR(token) sdl2w::hash(token)
#define LOCSTR_TYPE size_t
constexpr bool L10N_ENABLED = true;
#else
#define LOCSTR(token) token
#define LOCSTR_TYPE std::string
constexpr bool L10N_ENABLED = false;
#endif

inline LOCSTR_TYPE hash(std::string_view str) {
  #ifdef SDL2W_ENABLE_L10N
  std::hash<std::string> hasher;
  return hasher(std::string(str));
  #else
  return std::string(str);
  #endif
}

#define TRANSLATE(text) sdl2w::L10n::trans(sdl2w::hash(text))

class L10n {
  static std::string language;
  static std::unordered_map<std::string,
                            std::unordered_map<LOCSTR_TYPE, std::string>>
      locStrings;
  static std::vector<std::string> supportedLanguages;

public:
  static void init(const std::vector<std::string>& langs = {"en"});
  static void loadLanguage(const std::string& lang,
                           const std::string& langFile);
  static void setLanguage(const std::string& lang);
  static bool isEnabled();
  static const std::unordered_map<LOCSTR_TYPE, std::string>& getStrings();
  static std::string trans(const std::string& id);
  static std::string trans(int id);
};

} // namespace sdl2w
