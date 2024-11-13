#ifndef LOGGING_H
#define LOGGING_H

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

void printlog(enum LogLevel level, const char* fmt, ...);

#endif