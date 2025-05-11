#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

// Struct to hold language and its associated file path
struct LangInfo {
  std::string langCode;
  std::string filePath; // Path to existing/output translation file
  std::map<std::string, std::string>
      translations; // original -> translated (use map for sorting by key)
};

// Unescapes C++-style string literal content (handles \\\\" and \\")
std::string unescapeStringLiteralContent(const std::string& s) {
  std::string res;
  for (size_t i = 0; i < s.length(); ++i) {
    if (s[i] == '\x5C' &&
        i + 1 < s.length()) { // Changed '\\\\' to '\x5C' to avoid -Wmultichar
      switch (s[i + 1]) {
      case '\"':
        res += '\"';
        i++;
        break;
      case '\x5C':
        res += '\x5C';
        i++;
        break; // Changed '\\\\' to '\x5C'
      // Add other C++ escapes like \\n, \\t if needed, though
      // standard string literals in macros usually don't contain raw newlines.
      default:
        res += '\x5C'; // Changed '\\\\' to '\x5C'
      }
    } else {
      res += s[i];
    }
  }
  return res;
}

// Function to parse a line from a translation file: "original" "translated"
bool parseTranslationLine(const std::string& line,
                          std::string& original,
                          std::string& translated) {
  static const std::regex lineRegex(R"raw(\[(.*)\] \{(.*)\})raw");
  std::smatch match;
  if (std::regex_match(line, match, lineRegex) && match.size() == 3) {
    original = "[" + unescapeStringLiteralContent(match[1].str()) + "]";
    translated = "{" + unescapeStringLiteralContent(match[2].str()) + "}";
    return true;
  }
  return false;
}

// Function to scan a single file for TRANSLATE macros
void scanFileForTranslations(const fs::path& filePath,
                             std::unordered_set<std::string>& foundStrings) {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::cerr << "Warning: Could not open file " << filePath.string()
              << std::endl;
    return;
  }

  std::cout << "Scanning file: " << filePath.string() << std::endl;

  static const std::regex macroRegex(R"raw(TRANSLATE\("(.*)"\))raw");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  auto it = std::sregex_iterator(content.begin(), content.end(), macroRegex);
  auto end = std::sregex_iterator();
  for (; it != end; ++it) {
    std::smatch match = *it;
    if (match.size() >= 2) {
      std::string str = match[1].str();
      std::stringstream ss;
      ss << "[" << str << "]";
      foundStrings.insert(ss.str());
    }
  }
}

int main(int argc, char* argv[]) {
  std::string inputDirPathStr;
  std::string outputDirPathStr;
  std::vector<std::string> langCodeStrs;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--input-dir") {
      if (i + 1 < argc) {
        inputDirPathStr = argv[++i];
      } else {
        std::cerr << "Error: --input-dir option requires a path argument."
                  << std::endl;
        return 1;
      }
    } else if (arg == "--output-dir") {
      if (i + 1 < argc) {
        outputDirPathStr = argv[++i];
      } else {
        std::cerr << "Error: --output-dir option requires a path argument."
                  << std::endl;
        return 1;
      }
    } else if (arg.length() == 2 &&
               std::isalpha(static_cast<unsigned char>(arg[0])) &&
               std::isalpha(static_cast<unsigned char>(arg[1]))) {
      langCodeStrs.push_back(arg);
    } else {
      std::cerr << "Warning: Ignoring invalid argument: " << arg << std::endl;
    }
  }

  if (inputDirPathStr.empty()) {
    std::cerr << "Error: --input-dir is a required argument." << std::endl;
    std::cerr << "Usage: " << argv[0]
              << " --input-dir <source_code_directory> --output-dir "
                 "<translations_directory> <lang_code1> <lang_code2> ..."
              << std::endl;
    std::cerr << "Example: " << argv[0]
              << " --input-dir ./src --output-dir ./assets en la fr"
              << std::endl;
    return 1;
  }
  if (outputDirPathStr.empty()) {
    std::cerr << "Error: --output-dir is a required argument." << std::endl;
    std::cerr << "Usage: " << argv[0]
              << " --input-dir <source_code_directory> --output-dir "
                 "<translations_directory> <lang_code1> <lang_code2> ..."
              << std::endl;
    std::cerr << "Example: " << argv[0]
              << " --input-dir ./src --output-dir ./assets en la fr"
              << std::endl;
    return 1;
  }
  if (langCodeStrs.empty()) {
    std::cerr << "Error: No language codes provided." << std::endl;
    std::cerr << "Usage: " << argv[0]
              << " --input-dir <source_code_directory> --output-dir "
                 "<translations_directory> <lang_code1> <lang_code2> ..."
              << std::endl;
    std::cerr << "Example: " << argv[0]
              << " --input-dir ./src --output-dir ./assets en la fr"
              << std::endl;
    return 1;
  }

  fs::path sourceScanDirectory = fs::absolute(fs::path(inputDirPathStr));
  if (!fs::exists(sourceScanDirectory) ||
      !fs::is_directory(sourceScanDirectory)) {
    std::cerr << "Error: Input source directory "
              << sourceScanDirectory.string()
              << " does not exist or is not a directory." << std::endl;
    return 1;
  }
  std::cout << "Using source code input directory: "
            << sourceScanDirectory.string() << std::endl;

  fs::path outputDirectory = fs::absolute(fs::path(outputDirPathStr));

  if (!fs::exists(outputDirectory)) {
    std::cout << "Attempting to create output directory: "
              << outputDirectory.string() << std::endl;
    if (!fs::create_directories(outputDirectory)) {
      std::cerr << "Error: Could not create output directory "
                << outputDirectory.string() << std::endl;
      return 1;
    }
    std::cout << "Successfully created output directory." << std::endl;
  } else {
    std::cout << "Using output directory: " << outputDirectory.string()
              << std::endl;
  }

  std::vector<LangInfo> languagesToProcess;
  for (const std::string& lcStr : langCodeStrs) {
    LangInfo currentLang;
    currentLang.langCode = lcStr;
    currentLang.filePath =
        (outputDirectory / ("translation." + lcStr + ".txt")).string();
    languagesToProcess.push_back(currentLang);
  }

  for (LangInfo& langInfo : languagesToProcess) {
    fs::path currentFilePath(langInfo.filePath);
    if (fs::exists(currentFilePath)) {
      std::ifstream infile(currentFilePath);
      if (infile.is_open()) {
        std::string line;
        std::string original, translated;
        while (std::getline(infile, line)) {
          if (parseTranslationLine(line, original, translated)) {
            langInfo.translations[original] = translated;
          }
        }
        infile.close();
        std::cout << "Loaded existing translations for " << langInfo.langCode
                  << " from " << langInfo.filePath << std::endl;
      } else {
        std::cerr << "Warning: Could not open existing translation file "
                  << langInfo.filePath << " for language " << langInfo.langCode
                  << std::endl;
      }
    } else {
      std::cout << "No existing translation file found for "
                << langInfo.langCode << " at " << langInfo.filePath
                << ". A new file will be created if strings are found."
                << std::endl;
    }
  }

  std::unordered_set<std::string> allFoundOriginalStrings;

  // Scan the specified input directory
  std::cout << "Scanning directory: " << sourceScanDirectory.string()
            << std::endl;
  for (const auto& entry :
       fs::recursive_directory_iterator(sourceScanDirectory)) {
    if (entry.is_regular_file()) {
      std::string ext = entry.path().extension().string();
      if (ext == ".cpp" || ext == ".h" || ext == ".hpp") {
        scanFileForTranslations(entry.path(), allFoundOriginalStrings);
      }
    }
  }

  std::cout << "Found " << allFoundOriginalStrings.size()
            << " unique translatable string(s) in the codebase." << std::endl;

  for (LangInfo& langInfo : languagesToProcess) {
    int numUpdated = 0;
    std::map<std::string, std::string> finalTranslationsSorted;

    for (const std::string& originalKey : allFoundOriginalStrings) {
      auto it = langInfo.translations.find(originalKey);
      if (it != langInfo.translations.end()) {
        finalTranslationsSorted[originalKey] = it->second;
      } else {
        numUpdated++;
        if (langInfo.langCode == "en") {
          finalTranslationsSorted[originalKey] =
              "{" + originalKey.substr(1, originalKey.size() - 2) + "}";
        } else {
          finalTranslationsSorted[originalKey] = "{MISSING!}";
        }
      }
    }

    for (const auto& pair : langInfo.translations) {
      if (finalTranslationsSorted.find(pair.first) ==
          finalTranslationsSorted.end()) {
        finalTranslationsSorted[pair.first] = pair.second;
      }
    }

    if (finalTranslationsSorted.empty() && !fs::exists(langInfo.filePath)) {
      std::cout << "No strings to write and no existing file for "
                << langInfo.langCode << " at " << langInfo.filePath
                << ". Skipping file creation." << std::endl;
      continue;
    }

    fs::path outputFilePathObj(langInfo.filePath);

    std::ofstream outfile(outputFilePathObj);
    if (!outfile.is_open()) {
      std::cerr << "Error: Could not open file " << outputFilePathObj.string()
                << " for writing." << std::endl;
      continue;
    }

    for (const auto& pair : finalTranslationsSorted) {
      outfile << pair.first << " " << pair.second << "\n";
    }
    outfile.close();

    if (numUpdated > 0) {
      std::cout << "Updated " << numUpdated
                << " translation(s) in the file for language "
                << langInfo.langCode << std::endl;
    } else {
      std::cout << "No new translations found for language "
                << langInfo.langCode << std::endl;
    }
  }

  return 0;
}
