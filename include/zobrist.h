#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <stdint.h>

extern uint64_t piece_hash[64][13];
extern uint64_t black_hash;
extern uint64_t castling_hash[16];
extern uint64_t ep_hash[8];

void init_zobrist();

#endif

