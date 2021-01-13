#include "gtest/gtest.h"
#include "logger/Logger.h"


TEST(LoggerTest, OutputTest) {
  Logger::init(Logger::LogLevel::LogLevelDebug);
  DINFO("test log %s", "this is test string");
  DINFO("test log %d", 5);
}
