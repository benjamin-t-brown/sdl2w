#include "L10n.h"
#include "AssetLoader.h"
#include "Defines.h"
#include "Logger.h"
#include <string_view>

namespace sdl2w {

bmin::String L10n::language = "en";
bmin::Map<bmin::String, bmin::Map<size_t, bmin::String>> L10n::locStrings;
bmin::DynArray<bmin::String> L10n::supportedLanguages;
bool L10n::enabledFlag = false;

void L10n::init(std::initializer_list<std::string_view> langs) {
  if (!isEnabled()) {
    return;
  }

  locStrings[bmin::String(DEFAULT_LANGUAGE)] =
      bmin::Map<size_t, bmin::String>();

  supportedLanguages.clear();
  for (std::string_view lang : langs) {
    supportedLanguages.pushBack(bmin::String(lang.data(), lang.size()));
  }

  for (size_t i = 0; i < supportedLanguages.size(); ++i) {
    const bmin::String& lang = supportedLanguages[i];
    const bmin::String path =
        bmin::String("assets/translation.") + lang + ".txt";
    try {
      LOG(DEBUG) << "[sdl2w] Loading translation file "
                 << (bmin::String(ASSETS_PREFIX.data(), ASSETS_PREFIX.size()) +
                     path)
                 << Logger::endl;
      const bmin::String content = loadFileAsString(path.sliceView());
      loadLanguage(lang.sliceView(), content.sliceView());
    } catch (std::exception& e) {
      LOG_LINE(ERROR) << "Failed to load language file '" << path
                      << "': " << e.what() << Logger::endl;
    }
  }
}

void L10n::loadLanguage(std::string_view lang, std::string_view langText) {
  if (!isEnabled()) {
    return;
  }

  const bmin::String langStr(lang.data(), lang.size());
  locStrings[langStr] = bmin::Map<size_t, bmin::String>();

  size_t start = 0;
  while (start <= langText.size()) {
    const size_t end = langText.find('\n', start);
    std::string_view line;
    if (end == std::string_view::npos) {
      line = langText.substr(start);
      start = langText.size() + 1;
    } else {
      line = langText.substr(start, end - start);
      start = end + 1;
    }
    if (!line.empty() && line.back() == '\r') {
      line = line.substr(0, line.size() - 1);
    }

    const size_t firstQuote = line.find('[');
    if (firstQuote == std::string_view::npos) {
      if (end == std::string_view::npos) {
        break;
      }
      continue;
    }
    const size_t secondQuote = line.find(']', firstQuote + 1);
    if (secondQuote == std::string_view::npos) {
      if (end == std::string_view::npos) {
        break;
      }
      continue;
    }
    const bmin::String original(line.data() + firstQuote + 1,
                                secondQuote - firstQuote - 1);

    const size_t thirdQuote = line.find('{', secondQuote + 1);
    if (thirdQuote == std::string_view::npos) {
      if (end == std::string_view::npos) {
        break;
      }
      continue;
    }
    const size_t fourthQuote = line.find('}', thirdQuote + 1);
    if (fourthQuote == std::string_view::npos) {
      if (end == std::string_view::npos) {
        break;
      }
      continue;
    }
    const bmin::String translated(line.data() + thirdQuote + 1,
                                  fourthQuote - thirdQuote - 1);

    locStrings[langStr][hash(original.sliceView())] = translated;

    if (end == std::string_view::npos) {
      break;
    }
  }
}

void L10n::setEnabled(bool enabled) { enabledFlag = enabled; }
bool L10n::isEnabled() { return enabledFlag; }

void L10n::setLanguage(std::string_view lang) {
  if (!isEnabled()) {
    return;
  }

  const bmin::String langStr(lang.data(), lang.size());
  if (locStrings.contains(langStr)) {
    language = langStr;
    LOG(DEBUG) << "[sdl2w] Language set to '" << langStr << "'" << Logger::endl;
  } else {
    LOG_LINE(ERROR) << "Language '" << langStr << "' not supported."
                    << Logger::endl;
  }
}

const bmin::Map<size_t, bmin::String>& L10n::getStrings() {
  return locStrings[language];
}

bmin::String L10n::trans(size_t id) {
  const bmin::Map<size_t, bmin::String>& strings = getStrings();
  auto it = const_cast<bmin::Map<size_t, bmin::String>&>(strings).find(id);
  if (it != strings.end()) {
    return (*it).value;
  }
  return "?MISSING?";
}

size_t L10n::hash(std::string_view str) {
  const size_t result = std::hash<std::string_view>{}(str);

  bmin::Map<size_t, bmin::String>& defaults =
      locStrings[bmin::String(DEFAULT_LANGUAGE)];
  if (!defaults.contains(result)) {
    defaults[result] = bmin::String(str.data(), str.size());
  }

  return result;
}

} // namespace sdl2w
