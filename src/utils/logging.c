#include "logging.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

static enum LogLevel log_level = WARNING;

void printlog(enum LogLevel level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (level >= log_level) {
        switch (level) {
            case DEBUG:
                printf("[DEBUG] ");
                break;
            case INFO:
                printf("[INFO] ");
                break;
            case WARNING:
                printf("[WARNING] ");
                break;
            case ERROR:
                printf("[ERROR] ");
                break;
        }

        vprintf(fmt, args);

        if (level == ERROR) {
           printf("\n");exit(1);
        }

        printf("\n");
    }
    va_end(args);

    fflush(stdout);
}