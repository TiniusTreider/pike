#ifndef TABLES_H
#define TABLES_H

#include "def.h"

#include <stdint.h>

extern const uint8_t CASTLING_RIGHTS_TABLE[64];

extern const p_bitboard KNIGHT_MOVE_TABLE[64];
extern const p_bitboard KING_MOVE_TABLE[64];
extern const p_bitboard BISHOP_MOVE_TABLE[64];
extern const p_bitboard ROOK_MOVE_TABLE[64];
extern const p_bitboard BISHOP_PEXT_TABLE[64][512];
extern const p_bitboard ROOK_PEXT_TABLE[64][4096];

extern const p_bitboard BETWEEN_TABLE[64][64];

void print_tables(void);

#endif

