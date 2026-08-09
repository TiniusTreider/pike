#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"

#include <stddef.h>

size_t generate_moves(p_board board, p_move buffer[218]);
p_bitboard is_square_attacked(p_board board, p_index square);

#endif

