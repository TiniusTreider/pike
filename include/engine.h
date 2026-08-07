#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#include "wrappers.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>

struct engine_data {
        p_board board;

        bool debug;

        p_move searchmoves[218];
        size_t searchmoves_size;
        bool ponder;
        size_t wtime;
        size_t btime;
        size_t winc;
        size_t binc;
        size_t movestogo;
        size_t depth;
        size_t nodes;
        size_t mate;
        size_t movetime;
        bool infinite;
        bool perft;
        size_t perft_depth;
};

typedef struct engine_struct {
        p_mutex lock;
        p_sem task;
        atomic_bool stop;
        atomic_bool kill;

        struct engine_data data;
}* p_engine;

extern p_engine pike;

p_engine init_engine(void);
void clean_engine(p_engine engine);

#endif

