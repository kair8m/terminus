#ifndef TERMINUS_LOGGER_H
#define TERMINUS_LOGGER_H

#include <string>
#include <iostream>
#include <cstdarg>
#include <memory>

/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST


/*! thread safe debug api */
class Logger {

public:
  enum class LogLevel {
    /*! print only errors */
    LogLevelErr,
    /*! print errors and warnings*/
    LogLevelWarn,
    /*! print errors, warnings and info */
    LogLevelInfo,
    /*! print everything */
    LogLevelDebug
  };

  explicit Logger(LogLevel level);

private:
  LogLevel fLevel;
  static std::shared_ptr<Logger> fInstance;
private:

private:

  void logInternal(LogLevel level, const std::string &msg);

public:

  static void init(LogLevel level);

public:
  static void log(LogLevel level, const char *format, ...);
};

#define DINFO(format, arg...)   Logger::log(Logger::LogLevel::LogLevelInfo , KGRN "[%s] INFO  %s %s:%d " format, __TIME__, __FUNCTION__, __FILE__, __LINE__, ##arg)
#define DWARN(format, arg...)   Logger::log(Logger::LogLevel::LogLevelWarn , KYEL "[%s] WARN  %s %s:%d " format, __TIME__, __FUNCTION__, __FILE__, __LINE__, ##arg)
#define DERROR(format, arg...)  Logger::log(Logger::LogLevel::LogLevelErr  , KRED "[%s] ERROR %s %s:%d " format, __TIME__, __FUNCTION__, __FILE__, __LINE__, ##arg)


#endif //TERMINUS_LOGGER_H
