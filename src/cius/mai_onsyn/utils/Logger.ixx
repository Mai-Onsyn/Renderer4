module;
#include <iostream>
export module Logger;
import Types;
import Time;

using std::cout;
using std::endl;

Mutex mtx;
ConditionVariable cv;

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

String toString(const LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
    }
    return "LOG";
}

export class Log {
    static void log(const String& msg, const LogLevel level) {
        {
            LockGuard lock(mtx);
            cout << formatTimeMillis(millisTime(), "[HH:mm:ss] [") << toString(level) << "] " << msg << endl;
        }
        cv.notify_all();
    }
public:
    static void debug(const String& msg) {
        log(msg, LogLevel::DEBUG);
    }
    static void info(const String& msg) {
        log(msg, LogLevel::INFO);
    }
    static void warn(const String& msg) {
        log(msg, LogLevel::WARN);
    }
    static void error(const String& msg) {
        log(msg, LogLevel::ERROR);
    }
};
