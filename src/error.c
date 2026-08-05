#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void error(const char *message)
{
        fflush(stdout);
        fprintf(stderr, "\nerror: %s\n", message);
        exit(EXIT_FAILURE);
}

void errorif(bool condition, const char *message)
{
        if (condition)
                error(message);
}

void errorf(const char *message, ...)
{
        va_list args;
        va_start(args, message);

        char buffer[ERROR_MAX_LENGTH];
        vsnprintf(buffer, ERROR_MAX_LENGTH, message, args);

        va_end(args);

        error(buffer);
}

void erroriff(bool condition, const char *message, ...)
{
        if (condition) {
                va_list args;
                va_start(args, message);

                char buffer[ERROR_MAX_LENGTH];
                vsnprintf(buffer, ERROR_MAX_LENGTH, message, args);

                va_end(args);

                error(buffer);
        }
}

