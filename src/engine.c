#include "engine.h"
#include "memory.h"

#include <stdlib.h>
#include <stddef.h>

extern p_engine pike;

p_engine init_engine(void)
{
        p_engine engine = scalloc(1, sizeof(struct engine_struct));

        init_mutex(&engine->lock);
        init_sem(&engine->task);

        engine->board = init_board(STARTPOS_FEN);

        return engine;
}

void clean_engine(p_engine engine)
{
        clean_mutex(&engine->lock);
        clean_sem(&engine->task);

        clean_board(engine->board);

        free(engine);
}

