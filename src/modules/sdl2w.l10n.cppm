module;
#include <initializer_list>
#include <string_view>

#include "impl_headers.h"
export module sdl2w.l10n;
export import bmin.containers;
import sdl2w.assets;
import sdl2w.defines;
import sdl2w.logger;
import bmin.string_interop;

export namespace sdl2w {

class L10n {
  static bmin::String language;
  static bmin::Map<bmin::String, bmin::Map<size_t, bmin::String>> locStrings;
  static bmin::DynArray<bmin::String> supportedLanguages;
  static bool enabledFlag;
  static const bmin::String& transRef(size_t id);

public:
  static void init(std::initializer_list<std::string_view> langs = {"en"});
  static void loadLanguage(std::string_view lang, std::string_view langText);
  static void setLanguage(std::string_view lang);
  static void setEnabled(bool enabled);
  static bool isEnabled();
  static const bmin::Map<size_t, bmin::String>& getStrings();
  static bmin::String trans(size_t id);
  static const char* translate(const char* text);
  static size_t hash(std::string_view str);
};

}

// These are typed module declarations, not macros. Their global spelling is
// deliberate so module and classic consumers can write the same source code.
export inline const char* TRANSLATE(const char* text) {
  return sdl2w::L10n::translate(text);
}

export inline constexpr const char* DISABLE_TRANSLATIONS = "default";
