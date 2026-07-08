#include "Logger.h"
#include <stdarg.h>
#include <stdexcept>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/stack.h>
#include <cstdlib>
#elif defined(__linux__) || defined(__APPLE__)
#include <execinfo.h>
#endif

namespace sdl2w {

static bmin::String removeANSIEscapeCodes(std::string_view str) {
  bmin::String out;
  out.reserve(str.size());
  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] == '\x1B' && i + 1 < str.size() && str[i + 1] == '[') {
      i += 2;
      while (i < str.size() && str[i] != 'm') {
        ++i;
      }
      continue;
    }
    out.pushBack(str[i]);
  }
  return out;
}

const bmin::String Logger::endl = bmin::String("\n");
LogType Logger::logLevel = DEBUG;
LogType Logger::localLogLevel = DEBUG;
bool Logger::disabled = false;
bool Logger::colorEnabled = true;
bool Logger::logToFile = false;
std::fstream Logger::logFile;

bmin::StringStream& Logger::get(LogType level) {
  localLogLevel = logLevel;
  const bmin::String label = getLabel(level);
  os << label;
  return os;
}
bmin::StringStream& Logger::get(LogType level, const char* file, int line) {
  localLogLevel = logLevel;
  const bmin::String label = getLabel(level);
  const char* colorPre = Logger::colorEnabled ? "\033[90m" : "";
  const char* colorPost = Logger::colorEnabled ? "\033[0m" : "";
  os << label << colorPre << "<" << file << ":" << line << ">" << colorPost
     << " ";
  return os;
}
bmin::String Logger::getLabel(LogType type) {
  bmin::String label;
  switch (type) {
  case DEBUG:
    label = "{ DEBUG } ";
    if (Logger::colorEnabled) {
      label = bmin::String("\033[36m") + label + "\033[0m";
    }
    break;
  case INFO:
    label = "{ INFO  } ";
    if (Logger::colorEnabled) {
      label = bmin::String("\033[32m") + label + "\033[0m";
    }
    break;
  case WARN:
    label = "{ WARN  } ";
    if (Logger::colorEnabled) {
      label = bmin::String("\033[33m") + label + "\033[0m";
    }
    break;
  case ERROR:
    label = "{ ERROR } ";
    if (Logger::colorEnabled) {
      label = bmin::String("\033[31m") + label + "\033[0m";
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
    fprintf(stdout, "%s", os.str().cStr());
    fflush(stdout);
    if (Logger::logToFile && Logger::logFile.is_open()) {
      const bmin::String stripped = removeANSIEscapeCodes(os.str().sliceView());
      Logger::logFile << stripped.cStr();
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

bmin::String Logger::getStackTrace() {
#if defined(__EMSCRIPTEN__)
  const int required = emscripten_get_callstack(EM_LOG_C_STACK, nullptr, 0);
  if (required <= 0) {
    return "";
  }

  char* buffer = static_cast<char*>(malloc(static_cast<size_t>(required)));
  if (buffer == nullptr) {
    return "";
  }
  const int written =
      emscripten_get_callstack(EM_LOG_C_STACK, buffer, required);
  if (written <= 0) {
    free(buffer);
    return "";
  }
  size_t len = static_cast<size_t>(written);
  if (len > 0 && buffer[len - 1] == '\0') {
    --len;
  }
  const bmin::String result(buffer, len);
  free(buffer);
  return result;
#elif defined(_WIN32)
  return "";
#elif defined(__linux__) || defined(__APPLE__)
  void* frames[64];
  const int frameCount = backtrace(frames, 64);
  if (frameCount <= 0) {
    return "";
  }
  char** symbols = backtrace_symbols(frames, frameCount);
  bmin::StringStream os;
  os << "frames=" << frameCount << "\n";
  if (symbols) {
    for (int i = 0; i < frameCount; ++i) {
      os << "  [" << i << "] " << symbols[i] << "\n";
    }
    free(symbols);
  } else {
    for (int i = 0; i < frameCount; ++i) {
      char buf[64];
      snprintf(buf, sizeof(buf), "  [%d] %p\n", i, frames[i]);
      os << buf;
    }
  }
  return os.str();
#else
  return "Stack trace not available for this platform.\n";
#endif
}

[[noreturn]] void Logger::throwRuntimeError(std::string_view msg) {
  throwRuntimeError(msg, nullptr, 0);
}

[[noreturn]] void Logger::throwRuntimeError(std::string_view msg,
                                            const char* file, int line) {
  const bmin::String msgStr(msg.data(), msg.size());
  const bmin::String trace = getStackTrace();
  if (file != nullptr) {
    Logger().get(ERROR, file, line) << msgStr << Logger::endl;
    if (!trace.empty()) {
      Logger().get(ERROR, file, line) << "Stack trace:\n"
                                      << trace << Logger::endl;
    }
  } else {
    Logger().get(ERROR) << msgStr << Logger::endl;
    if (!trace.empty()) {
      Logger().get(ERROR) << "Stack trace:\n" << trace << Logger::endl;
    }
  }

  bmin::String exceptionMsg = msgStr;
  if (!trace.empty()) {
    exceptionMsg += "\nStack trace:\n";
    exceptionMsg += trace;
  }
  throw std::runtime_error(exceptionMsg.cStr());
}

[[noreturn]] void Logger::throwRuntimeError(const bmin::String& msg,
                                            const char* file, int line) {
  throwRuntimeError(msg.sliceView(), file, line);
}
} // namespace sdl2w
