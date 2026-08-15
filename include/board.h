#ifndef BOARD_H
#define BOARD_H

// TODO history start in unmake

#include "def.h"

typedef struct board_struct* p_board;

typedef struct {
        p_index from;
        p_index to;
        p_piece_type promotion;
        uint8_t flags;
} p_move;

typedef struct {
        p_move move;
        p_piece piece;
} p_external_move;

static constexpr uint8_t MOVE_PROMOTION = 0b00000001;
static constexpr uint8_t MOVE_EN_PASSANT = 0b00000010;
static constexpr uint8_t MOVE_LONG_CASTLE = 0b00000100;
static constexpr uint8_t MOVE_SHORT_CASTLE = 0b00001000;

typedef struct {
        p_piece captured_piece;
        uint8_t castling_rights;
        p_index ep_square;
        uint64_t zobrist;
        uint8_t history_start;
        uint64_t history;
} p_unmake;

p_unmake make_move(p_board board, p_move move);
void unmake_move(p_board board, p_move move, p_unmake data);

p_board init_board(char *fen);
void set_board(p_board board, char *fen);
void clean_board(p_board board);

bool is_draw(p_board board);

bool moves_are_equal(p_move a, p_move b);

#endif

