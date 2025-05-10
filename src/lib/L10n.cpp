#include "L10n.h"
#include "AssetLoader.h"
#include "Logger.h"
#include <unordered_map>

namespace sdl2w {

std::string L10n::language = "en";
std::unordered_map<std::string, std::unordered_map<LOCSTR_TYPE, std::string>>
    L10n::locStrings;
std::vector<std::string> L10n::supportedLanguages = {
    // default supported languages
    "en"};

void L10n::init(const std::vector<std::string>& langs) {
  Logger().get() << "Localization is "
                 << (L10N_ENABLED ? "enabled" : "disabled") << Logger::endl;
  if (!isEnabled()) {
    return;
  }

  supportedLanguages = langs;

  for (const auto& lang : supportedLanguages) {
    const std::string path = "assets/translation." + lang + ".txt";
    try {
      const std::string languageFile = loadFileAsString(path);
      loadLanguage(lang, languageFile);
    } catch (std::exception& e) {
      LOG_LINE(ERROR) << "Failed to load language file '" << path
                      << "': " << e.what() << Logger::endl;
    }
  }
}

void L10n::loadLanguage(const std::string& lang, const std::string& langText) {
  if (!isEnabled()) {
    return;
  }

  locStrings[lang] = std::unordered_map<LOCSTR_TYPE, std::string>();
  std::istringstream iss(langText);
  std::string line;
  while (std::getline(iss, line)) {
    // Find the first quoted part (original)
    size_t firstQuote = line.find('\"');
    if (firstQuote == std::string::npos) {
      continue;
    }
    size_t secondQuote = line.find('\"', firstQuote + 1);
    if (secondQuote == std::string::npos) {
      continue;
    }
    std::string original =
        line.substr(firstQuote + 1, secondQuote - firstQuote - 1);

    // Find the second quoted part (translated)
    size_t thirdQuote = line.find('\"', secondQuote + 1);
    if (thirdQuote == std::string::npos) {
      continue;
    }
    size_t fourthQuote = line.find('\"', thirdQuote + 1);
    if (fourthQuote == std::string::npos) {
      continue;
    }
    std::string translated =
        line.substr(thirdQuote + 1, fourthQuote - thirdQuote - 1);

    locStrings[lang][LOCSTR(original)] = translated;
  }
}

bool L10n::isEnabled() { return L10N_ENABLED; }

void L10n::setLanguage(const std::string& lang) {
  if (!isEnabled()) {
    return;
  }

  auto it = locStrings.find(lang);
  if (it != locStrings.end()) {
    language = lang;
  } else {
    LOG_LINE(ERROR) << "Language '" << lang << "' not supported."
                    << Logger::endl;
  }
}

const std::unordered_map<LOCSTR_TYPE, std::string>& L10n::getStrings() {
  return locStrings[language];
}

std::string L10n::trans(const std::string& id) { return id; }

std::string L10n::trans(int id) {
#ifdef SDL2W_ENABLE_L10N
  auto strings = getStrings();
  auto it = strings.find(id);
  if (it != strings.end() && it->second.size()) {
    return it->second;
  }
  return "?MISSING?";
#else
  return std::string(id);
#endif
}

} // namespace sdl2w