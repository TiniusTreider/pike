#ifndef STRUCTBOARD_H
#define STRUCTBOARD_H

#include "def.h"

struct board_struct {
        p_color player;

        p_piece mailbox[64];
        p_bitboard bitboards[13];

        p_bitboard all_pieces;
        p_bitboard white_pieces;
        p_bitboard black_pieces;

        uint8_t castling_rights;
        p_index ep_square;
};

static constexpr uint8_t W_LONG_CASTLE = 0b00000001;
static constexpr uint8_t W_SHORT_CASTLE = 0b00000010;
static constexpr uint8_t B_LONG_CASTLE = 0b00000100;
static constexpr uint8_t B_SHORT_CASTLE = 0b00001000;

#endif

