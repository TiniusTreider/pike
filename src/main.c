#include "uci.h"
#include "engine.h"
#include "def.h"
#include "search.h"
#include "wrappers.h"

#include <stdio.h>

#define USAGE VERSION " USAGE:\n\n    pike    Starts in UCI (Universal Chess Interface) mode\n\n"

p_engine pike;

static inline void init_all(p_thread *thread)
{
        pike = init_engine();
        init_thread(thread, search_wait, NULL);
}

static inline void clean_all(p_thread *thread)
{
        pike->kill = true;
        clean_thread(thread);
        clean_engine(pike);
}

int main(int argc, char **argv)
{
        (void)argv;
        if (argc != 1) {
                printf(USAGE);
                return 0;
        }

        p_thread search_thread;
        init_all(&search_thread);
        uci();
        clean_all(&search_thread);

        return 0;
}

