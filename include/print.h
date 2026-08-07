#ifndef PRINT_H
#define PRINT_H

#include "board.h"

void print_move(p_move move, char string[6]);
p_move parse_move(p_board board, char *string);
void print_board(p_board board);
void print_bitboard(p_bitboard board);

#endif

