#include "engine.h"
#include "memory.h"

#include <stdlib.h>
#include <stddef.h>

extern p_engine pike;

p_engine init_engine(void)
{
        p_engine engine = smalloc(sizeof(struct engine_struct));
        *engine = (struct engine_struct){
                .board = init_board(STARTPOS_FEN)
        };

        return engine;
}

void clean_engine(p_engine engine)
{
        free(engine);
}

