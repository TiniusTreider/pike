#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"

#include <stddef.h>

size_t generate_capture_moves(p_board board, p_move *buffer);
size_t generate_quiet_moves(p_board board, p_move *buffer);
p_bitboard is_square_attacked(p_board board, p_index square);
bool is_move_legal_in_position(p_board board, p_external_move move);

typedef struct {
        p_bitboard pin_rays[64];
        p_bitboard checkers;
        p_bitboard bishop_pin_board;
        p_bitboard rook_pin_board;
        p_bitboard check_ray;
} p_pins;

#endif

