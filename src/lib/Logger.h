#pragma once

#include <fstream>
#include <source_location>
#include <string_view>

#include "bmin/String.h"
#include "bmin/StringStream.h"

namespace sdl2w {

#define LOG(level) sdl2w::Logger().get(sdl2w::LogType::level)
#define LOG_LINE(level)                                                        \
  sdl2w::Logger().get(sdl2w::LogType::level, __FILE__, __LINE__)
#define LOG_ENDL sdl2w::Logger::endl
#define THROW_RUNTIME_ERROR(msg)                                               \
  sdl2w::Logger::throwRuntimeError((msg), __FILE__, __LINE__)

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

} // namespace sdl2w

namespace sdl2w {

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

inline const bmin::String& endl = Logger::endl;

} // namespace sdl2w
