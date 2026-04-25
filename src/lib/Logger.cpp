#include "Logger.h"
#include <regex>
#include <stdarg.h>
#include <stdexcept>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/stack.h>
#elif defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#ifdef ERROR
#undef ERROR
#endif
#elif defined(__linux__) || defined(__APPLE__)
#include <execinfo.h>
#endif

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

std::string Logger::getStackTrace() {
#if defined(__EMSCRIPTEN__)
  const int required = emscripten_get_callstack(EM_LOG_C_STACK, nullptr, 0);
  if (required <= 0) {
    return "";
  }

  std::string trace(static_cast<size_t>(required), '\0');
  const int written =
      emscripten_get_callstack(EM_LOG_C_STACK, trace.data(), required);
  if (written <= 0) {
    return "";
  }

  // Drop trailing '\0' returned by the C API, if present.
  if (!trace.empty() && trace.back() == '\0') {
    trace.pop_back();
  }
  return trace;
#elif defined(_WIN32)
  void* frames[64];
  const USHORT frameCount = CaptureStackBackTrace(0, 64, frames, nullptr);
  if (frameCount == 0) {
    return "";
  }
  std::ostringstream os;
  os << "frames=" << frameCount << "\n";
  for (USHORT i = 0; i < frameCount; ++i) {
    os << "  [" << i << "] 0x" << std::hex
       << reinterpret_cast<uintptr_t>(frames[i]) << std::dec << "\n";
  }
  return os.str();
#elif defined(__linux__) || defined(__APPLE__)
  void* frames[64];
  const int frameCount = backtrace(frames, 64);
  if (frameCount <= 0) {
    return "";
  }
  char** symbols = backtrace_symbols(frames, frameCount);
  std::ostringstream os;
  os << "frames=" << frameCount << "\n";
  if (symbols) {
    for (int i = 0; i < frameCount; ++i) {
      os << "  [" << i << "] " << symbols[i] << "\n";
    }
    free(symbols);
  } else {
    for (int i = 0; i < frameCount; ++i) {
      os << "  [" << i << "] " << frames[i] << "\n";
    }
  }
  return os.str();
#else
  return "Stack trace not available for this platform.\n";
#endif
}

[[noreturn]] void Logger::throwRuntimeError(const std::string& msg) {
  throwRuntimeError(msg, nullptr, 0);
}

[[noreturn]] void Logger::throwRuntimeError(const std::string& msg,
                                            const char* file, int line) {
  const std::string trace = getStackTrace();
  if (file != nullptr) {
    Logger().get(ERROR, file, line) << msg << Logger::endl;
    if (!trace.empty()) {
      Logger().get(ERROR, file, line) << "Stack trace:\n"
                                      << trace << Logger::endl;
    }
  } else {
    Logger().get(ERROR) << msg << Logger::endl;
    if (!trace.empty()) {
      Logger().get(ERROR) << "Stack trace:\n" << trace << Logger::endl;
    }
  }

  std::string exceptionMsg = msg;
  if (!trace.empty()) {
    exceptionMsg += "\nStack trace:\n";
    exceptionMsg += trace;
  }
  throw std::runtime_error(exceptionMsg);
}
} // namespace sdl2w
