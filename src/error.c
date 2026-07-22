#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void error(const char *message)
{
        fflush(stdout);
        fprintf(stderr, "\033[31m%s\033[0m\n", message);
        exit(EXIT_FAILURE);
}

void errorif(bool statement, const char *message)
{
        if (statement)
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

void erroriff(bool statement, const char *message, ...)
{
        if (statement) {
                va_list args;
                va_start(args, message);

                char buffer[ERROR_MAX_LENGTH];
                vsnprintf(buffer, ERROR_MAX_LENGTH, message, args);

                va_end(args);

                error(buffer);
        }
}

