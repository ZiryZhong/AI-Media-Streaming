#ifndef LOGGER_H
#define LOGGER_H

#include <string>

class Logger {
private:
    static Logger* instance;
    Logger() {}

public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static Logger* getInstance();
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
};

#define LOG_DEBUG(msg) Logger::getInstance()->debug(msg)
#define LOG_INFO(msg) Logger::getInstance()->info(msg)
#define LOG_WARNING(msg) Logger::getInstance()->warning(msg)
#define LOG_ERROR(msg) Logger::getInstance()->error(msg)

#endif // LOGGER_H
