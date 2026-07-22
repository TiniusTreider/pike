#ifndef DEF_H
#define DEF_H

#include <stdint.h>

typedef uint8_t p_index;
typedef uint64_t p_bitboard;

static constexpr p_index NO_SQUARE = 64;

typedef enum : uint8_t {
        W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
        B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
        EMPTY
} p_piece;
typedef enum : uint8_t {
        PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
} p_piece_type;
typedef enum : uint8_t {
        WHITE, BLACK, NO
} p_color;

static constexpr p_bitboard FILE_A = 0x0101010101010101ULL;
static constexpr p_bitboard FILE_B = FILE_A << 1;
static constexpr p_bitboard FILE_C = FILE_A << 2;
static constexpr p_bitboard FILE_D = FILE_A << 3;
static constexpr p_bitboard FILE_E = FILE_A << 4;
static constexpr p_bitboard FILE_F = FILE_A << 5;
static constexpr p_bitboard FILE_G = FILE_A << 6;
static constexpr p_bitboard FILE_H = FILE_A << 7;

static constexpr p_bitboard RANK_1 = 0x00000000000000FFULL;
static constexpr p_bitboard RANK_2 = RANK_1 << 8;
static constexpr p_bitboard RANK_3 = RANK_1 << 16;
static constexpr p_bitboard RANK_4 = RANK_1 << 24;
static constexpr p_bitboard RANK_5 = RANK_1 << 32;
static constexpr p_bitboard RANK_6 = RANK_1 << 40;
static constexpr p_bitboard RANK_7 = RANK_1 << 48;
static constexpr p_bitboard RANK_8 = RANK_1 << 56;

#define STARTPOS_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#endif

