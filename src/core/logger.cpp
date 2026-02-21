#include "logger.h"
#include <iostream>
#include <ctime>

Logger* Logger::instance = nullptr;

Logger* Logger::getInstance() {
    if (!instance) {
        instance = new Logger();
    }
    return instance;
}

void Logger::log(LogLevel level, const std::string& message) {
    // 获取当前时间
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", timeinfo);

    // 输出日志级别
    const char* levelStr;
    switch (level) {
        case DEBUG:
            levelStr = "DEBUG";
            break;
        case INFO:
            levelStr = "INFO";
            break;
        case WARNING:
            levelStr = "WARNING";
            break;
        case ERROR:
            levelStr = "ERROR";
            break;
        default:
            levelStr = "UNKNOWN";
            break;
    }

    // 输出日志
    std::cout << "[" << timestamp << "] " << levelStr << ": " << message << std::endl;
}

void Logger::debug(const std::string& message) {
    log(DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(INFO, message);
}

void Logger::warning(const std::string& message) {
    log(WARNING, message);
}

void Logger::error(const std::string& message) {
    log(ERROR, message);
}
