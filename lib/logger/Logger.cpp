#include "Logger.h"

std::shared_ptr<Logger> Logger::fInstance = nullptr;

Logger::Logger(Logger::LogLevel level) : fLevel(level) {
}

void Logger::init(Logger::LogLevel level) {
  if (!fInstance) {
    fInstance = std::make_shared<Logger>(level);
  }
}

void Logger::logInternal(Logger::LogLevel level, const std::string &msg) {
  if (level < fLevel && fLevel != LogLevel::LogLevelDebug)
    return;
  std::cout << msg << std::endl;
}


void Logger::log(Logger::LogLevel level, const char *format, ...) {
  const int tempSize = 2048;
  auto temp = new char[tempSize];
  va_list args;
  va_start(args, format);
  auto status = vsnprintf(temp, tempSize, format, args);
  va_end(args);

  if (level == Logger::LogLevel::LogLevelCritical) {
    std::cout << temp << std::endl;
    return;
  }

  if (fInstance && (status > 0))
    fInstance->logInternal(level, temp);
  delete[] temp;
}