#include "engine.h"
#include "memory.h"

#include <stdlib.h>
#include <stddef.h>

extern p_engine pike;

p_engine init_engine(void)
{
        p_engine engine = scalloc(1, sizeof(struct engine_struct));

        pthread_mutex_init(&engine->lock, NULL);
        sem_init(&engine->task, 0, 0);

        engine->board = init_board(STARTPOS_FEN);

        return engine;
}

void clean_engine(p_engine engine)
{
        pthread_mutex_destroy(&engine->lock);
        sem_destroy(&engine->task);

        clean_board(engine->board);

        free(engine);
}

