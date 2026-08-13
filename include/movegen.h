#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"

#include <stddef.h>

size_t generate_capture_moves(p_board board, p_move *buffer);
size_t generate_quiet_moves(p_board board, p_move *buffer);
p_bitboard is_square_attacked(p_board board, p_index square);

#endif

