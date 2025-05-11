#include "Logger.h"
#include <regex>
#include <stdarg.h>
#include <stdio.h>

namespace sdl2w {

static std::string removeANSIEscapeCodes(const std::string& str) {
  static const std::regex ansiRegex("\x1B\\[[0-9;]*m");
  return std::regex_replace(str, ansiRegex, "");
}

const std::string Logger::endl = std::string("\n");
LogType Logger::logLevel = DEBUG;
LogType Logger::localLogLevel = DEBUG;
bool Logger::disabled = false;
bool Logger::colorEnabled = true;
bool Logger::logToFile = false;
std::fstream Logger::logFile;

std::ostringstream& Logger::get(LogType level) {
  localLogLevel = logLevel;
  std::string label = getLabel(level);
  os << label;
  return os;
}
std::ostringstream& Logger::get(LogType level, const char* file, int line) {
  localLogLevel = logLevel;
  std::string label = getLabel(level);
  std::string colorPre = Logger::colorEnabled ? "\033[90m" : "";
  std::string colorPost = Logger::colorEnabled ? "\033[0m" : "";
  os << label << colorPre << "<" << file << ":" << line << ">" << colorPost
     << " ";
  return os;
}
std::string Logger::getLabel(LogType type) {
  std::string label;
  switch (type) {
  case DEBUG:
    label = "{ DEBUG } ";
    if (Logger::colorEnabled) {
      label = "\033[36m" + label + "\033[0m";
    }
    break;
  case INFO:
    label = "{ INFO  } ";
    if (Logger::colorEnabled) {
      label = "\033[32m" + label + "\033[0m";
    }
    break;
  case WARN:
    label = "{ WARN  } ";
    if (Logger::colorEnabled) {
      label = "\033[33m" + label + "\033[0m";
    }
    break;
  case ERROR:
    label = "{ ERROR } ";
    if (Logger::colorEnabled) {
      label = "\033[31m" + label + "\033[0m";
    }
    break;
  }
  return label;
}
Logger::~Logger() {
  if (Logger::logLevel > localLogLevel) {
    return;
  }
  if (!Logger::disabled) {
    fprintf(stdout, "%s", os.str().c_str());
    fflush(stdout);
    if (Logger::logToFile && Logger::logFile.is_open()) {
      Logger::logFile << removeANSIEscapeCodes(os.str());
      Logger::logFile.flush();
    }
  }
}

void Logger::setLogToFile(bool logToFileA) {
  if (logToFileA) {
    Logger::logFile.open("output.log", std::ios::out | std::ios::trunc);
  } else if (!logToFileA && Logger::logFile.is_open()) {
    Logger::logFile.close();
  }
  Logger::logToFile = logToFileA;
}

void Logger::setLogLevel(LogType level) { Logger::logLevel = level; }

int Logger::printf(const char* c, ...) {
  if (Logger::disabled) {
    return 0;
  }
  if (Logger::logLevel > localLogLevel) {
    return 0;
  }

  va_list lst;
  va_start(lst, c);
  while (*c != '\0') {
    if (*c != '%') {
      putchar(*c);
      c++;
      continue;
    }
    c++;
    switch (*c) {
    case 's':
      fputs(va_arg(lst, char*), stdout);
      break;
    case 'c':
      putchar(va_arg(lst, int));
      break;
    }
    c++;
  }
  va_end(lst);
  return 0;
}
} // namespace sdl2w
