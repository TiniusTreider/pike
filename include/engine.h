#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"

#include <pthread.h>
#include <semaphore.h>

typedef struct engine_struct {
        pthread_mutex_t lock;
        sem_t task;

        p_board board;
}* p_engine;

extern p_engine pike;

p_engine init_engine(void);
void clean_engine(p_engine engine);

#endif

