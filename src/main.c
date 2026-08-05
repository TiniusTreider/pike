#include "uci.h"
#include "engine.h"
#include "def.h"
#include "search.h"

#include <stdio.h>

#define USAGE VERSION " USAGE:\n        pike    Starts in UCI (Universal Chess Interface) mode\n\n"

p_engine pike;

int main(int argc, char **argv)
{
        (void)argv;

        if (argc != 1) {
                printf(USAGE);
                return 0;
        }

        pike = init_engine();

        pthread_t search_thread;
        pthread_create(&search_thread, NULL, search_wait, NULL);

        uci();

        pthread_join(search_thread, NULL);

        clean_engine(pike);

        return 0;
}

