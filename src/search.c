#include "search.h"
#include "perft.h"
#include "engine.h"
#include "wrappers.h"
#include "debug.h"

#include <stddef.h>

static inline void search_parse(void)
{
        if (pike->data.perft) {
                perft(pike->data.perft_depth);
                return;
        }
}

void *search_wait(void*)
{
        for (;;)
        {
                wait_sem(&pike->task);

                if (pike->kill)
                        break;

                lock_mutex(&pike->lock);

                LOG("search started");

                search_parse();

                LOG("search ended");

                unlock_mutex(&pike->lock);
        }

        return NULL;
}

