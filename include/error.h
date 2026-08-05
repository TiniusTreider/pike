#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <stddef.h>

static constexpr size_t ERROR_MAX_LENGTH = 256;

#define ERROR_NAVIGATE_FILE "failed to navigate file"
#define ERROR_READ_FILE "failed to read from file"
#define ERROR_CLOSE_FILE "failed to close file"

#define ERROR_OPEN_DIR "failed to open directory"
#define ERROR_CLOSE_DIR "failed to close directory"

void error(const char *message);
void errorif(bool condition, const char *message);
void errorf(const char *message, ...);
void erroriff(bool condition, const char *message, ...);

#endif

