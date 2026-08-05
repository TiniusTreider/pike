#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#include "wrappers.h"

#include <pthread.h>
#include <semaphore.h>

struct engine_data {
        p_board board;
        bool debug;
};

typedef struct engine_struct {
        p_mutex lock;
        p_sem task;

        struct engine_data data;
}* p_engine;

extern p_engine pike;

p_engine init_engine(void);
void clean_engine(p_engine engine);

#endif

