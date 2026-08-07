#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1

#if DEBUG
        #include <stdio.h>
        #define LOG(MESSAGE) do { fprintf(stderr, "DEBUG: %s\n", MESSAGE); } while (false)
#else
        #define LOG(MESSAGE) ((void)0)
#endif

#endif

