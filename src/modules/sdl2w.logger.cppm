module;
#include "impl_headers.h"
#include <fstream>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <source_location>
#include <stdexcept>
#include <string_view>

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/stack.h>
#include <cstdlib>
#elif defined(__linux__) || defined(__APPLE__)
#include <execinfo.h>
#endif

export module sdl2w.logger;
export import bmin.string;
export import bmin.stringstream;
import bmin.string_interop;

export namespace sdl2w {

enum LogType { DEBUG, INFO, WARN, ERROR };

class Logger {
public:
  static const bmin::String endl;
  static LogType logLevel;
  static bool disabled;
  static bool colorEnabled;
  static bool logToFile;
  static std::fstream logFile;
  static LogType localLogLevel;

  Logger() { localLogLevel = DEBUG; }
  explicit Logger(LogType level) {
    localLogLevel = DEBUG;
    get(level);
  }
  Logger(LogType level, std::source_location loc) {
    localLogLevel = DEBUG;
    get(level, loc.file_name(), static_cast<int>(loc.line()));
  }
  virtual ~Logger();
  bmin::StringStream& get(LogType level = INFO);
  bmin::StringStream& get(LogType level, const char* file, int line);
  template <typename T>
  Logger& operator<<(const T& value) {
    os << value;
    return *this;
  }
  bmin::StringStream os;
  bmin::String getLabel(LogType type);
  static void setLogToFile(bool logToFile);
  static void setLogLevel(LogType level);

  int printf(const char* format, ...);

  static bmin::String getStackTrace();
  [[noreturn]] static void throwRuntimeError(std::string_view msg);
  [[noreturn]] static void
  throwRuntimeError(std::string_view msg, const char* file, int line);
  [[noreturn]] static void throwRuntimeError(const bmin::String& msg,
                                             const char* file, int line);
};

inline Logger log(LogType level) { return Logger(level); }

inline Logger
logAt(LogType level, std::source_location loc = std::source_location::current()) {
  return Logger(level, loc);
}

[[noreturn]] inline void
fail(std::string_view msg,
     std::source_location loc = std::source_location::current()) {
  Logger::throwRuntimeError(msg, loc.file_name(), static_cast<int>(loc.line()));
}

[[noreturn]] inline void
fail(const bmin::String& msg,
     std::source_location loc = std::source_location::current()) {
  Logger::throwRuntimeError(msg, loc.file_name(), static_cast<int>(loc.line()));
}

}

export namespace sdl2w {
inline const bmin::String& endl = Logger::endl;
}

// Module consumers keep the classic logging spellings without relying on
// preprocessor macros. These declarations intentionally live in the global
// namespace so `import sdl2w;` is all that is required for LOG(INFO).
export inline constexpr sdl2w::LogType DEBUG = sdl2w::DEBUG;
export inline constexpr sdl2w::LogType INFO = sdl2w::INFO;
export inline constexpr sdl2w::LogType WARN = sdl2w::WARN;
export inline constexpr sdl2w::LogType ERROR = sdl2w::ERROR;

export inline sdl2w::Logger LOG(sdl2w::LogType level) {
  return sdl2w::Logger(level);
}

export inline sdl2w::Logger
LOG_LINE(sdl2w::LogType level,
         std::source_location loc = std::source_location::current()) {
  return sdl2w::Logger(level, loc);
}

export inline const bmin::String& LOG_ENDL = sdl2w::endl;

export [[noreturn]] inline void
THROW_RUNTIME_ERROR(std::string_view msg,
                    std::source_location loc = std::source_location::current()) {
  sdl2w::fail(msg, loc);
}

export [[noreturn]] inline void
THROW_RUNTIME_ERROR(const char* msg,
                    std::source_location loc = std::source_location::current()) {
  sdl2w::fail(std::string_view(msg), loc);
}

export [[noreturn]] inline void
THROW_RUNTIME_ERROR(const bmin::String& msg,
                    std::source_location loc = std::source_location::current()) {
  sdl2w::fail(msg, loc);
}
