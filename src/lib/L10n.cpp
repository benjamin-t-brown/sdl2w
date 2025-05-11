#include "L10n.h"
#include "AssetLoader.h"
#include "Logger.h"
#include <unordered_map>

namespace sdl2w {

std::string L10n::language = "en";
std::unordered_map<std::string, std::unordered_map<size_t, std::string>>
    L10n::locStrings;
std::vector<std::string> L10n::supportedLanguages = {
    // default supported languages
    "en"};

void L10n::init(const std::vector<std::string>& langs) {
  Logger().get() << "Localization is "
                 << "enabled" << Logger::endl;
  if (!isEnabled()) {
    return;
  }

  locStrings["default"] = std::unordered_map<size_t, std::string>();

  supportedLanguages = langs;

  for (const auto& lang : supportedLanguages) {
    const std::string path = "assets/translation." + lang + ".txt";
    try {
      LOG_LINE(DEBUG) << "[sdl2w] Loading translation file "
                      << (std::string(ASSETS_PREFIX) + path) << Logger::endl;
      std::ifstream file(std::string(ASSETS_PREFIX) + path);
      if (!file) {
        LOG_LINE(ERROR) << "[sdl2w] Error opening file: " << path
                        << Logger::endl;
        throw std::runtime_error(std::string(FAIL_ERROR_TEXT));
      }
      std::stringstream buffer;
      buffer << file.rdbuf();

      loadLanguage(lang, buffer.str());
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

  locStrings[lang] = std::unordered_map<size_t, std::string>();
  std::istringstream iss(langText);
  std::string line;
  while (std::getline(iss, line)) {
    // Find the first quoted part (original)
    size_t firstQuote = line.find('[');
    if (firstQuote == std::string::npos) {
      continue;
    }
    size_t secondQuote = line.find(']', firstQuote + 1);
    if (secondQuote == std::string::npos) {
      continue;
    }
    std::string original =
        line.substr(firstQuote + 1, secondQuote - firstQuote - 1);

    // Find the second quoted part (translated)
    size_t thirdQuote = line.find('{', secondQuote + 1);
    if (thirdQuote == std::string::npos) {
      continue;
    }
    size_t fourthQuote = line.find('}', thirdQuote + 1);
    if (fourthQuote == std::string::npos) {
      continue;
    }
    std::string translated =
        line.substr(thirdQuote + 1, fourthQuote - thirdQuote - 1);

    locStrings[lang][hash(original)] = translated;
  }
}

bool L10n::isEnabled() { return true; }

void L10n::setLanguage(const std::string& lang) {
  if (!isEnabled()) {
    return;
  }

  auto it = locStrings.find(lang);
  if (it != locStrings.end()) {
    language = lang;
    LOG(DEBUG) << "Language set to '" << lang << "'" << Logger::endl;
  } else {
    LOG_LINE(ERROR) << "Language '" << lang << "' not supported."
                    << Logger::endl;
  }
}

const std::unordered_map<size_t, std::string>& L10n::getStrings() {
  return locStrings[language];
}

std::string L10n::trans(size_t id) {
  auto it = getStrings().find(id);
  if (it != getStrings().end()) {
    return it->second;
  }
  return "?MISSING?";
}

size_t L10n::hash(std::string_view str) {
  std::hash<std::string> hasher;
  size_t result = hasher(std::string(str));

  if (locStrings["default"].find(result) != locStrings["default"].end()) {
    locStrings["default"][result] = std::string(str);
  }

  return result;
}

} // namespace sdl2w