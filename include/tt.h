#ifndef TT_H
#define TT_H

#include "board.h"

#include <stddef.h>

typedef struct {
        p_move best;
        uint8_t depth;
        p_eval eval;
        uint64_t hash;
} p_tt_entry;

#endif

