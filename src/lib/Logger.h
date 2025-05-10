#pragma once

#include <fstream>
#include <sstream>
#include <string>

namespace sdl2w {

#define LOG(level) sdl2w::Logger().get(sdl2w::LogType::level)
#define LOG_LINE(level) sdl2w::Logger().get(sdl2w::LogType::level, __FILE__, __LINE__)
#define LOG_ENDL sdl2w::Logger::endl

enum LogType { DEBUG, INFO, WARN, ERROR };

class Logger {
public:
  static const std::string endl;
  static bool disabled;
  static bool colorEnabled;
  static bool logToFile;
  static std::fstream logFile;

  Logger() {};
  virtual ~Logger();
  std::ostringstream& get(LogType level = INFO);
  std::ostringstream& get(LogType level, const char* file, int line);
  std::ostringstream os;
  std::string getLabel(LogType type);
  static void setLogToFile(bool logToFile);

  int printf(const char* format, ...);
};

} // namespace sdl2w
