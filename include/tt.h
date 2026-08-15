#ifndef TT_H
#define TT_H

#include "board.h"

#include <stddef.h>

typedef enum : uint8_t {
        EXACT,
        LOWER
} p_tt_bound;

typedef struct {
        p_external_move best;
        uint64_t hash;
        p_eval eval;
        p_tt_bound bound;
        uint8_t depth;
} p_tt_entry;

#endif

