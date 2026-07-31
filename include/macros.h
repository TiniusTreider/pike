#ifndef MACROS_H
#define MACROS_H

#define BIT_MASK(BIT) (1ULL << (BIT))

#define BITBOARD_REMOVE_MASK(BITBOARD, MASK) do { BITBOARD &= ~(MASK); } while (false)
#define BITBOARD_ADD_MASK(BITBOARD, MASK) do { BITBOARD |= MASK; } while (false)
#define BITBOARD_REMOVE_BIT(BITBOARD, BIT) BITBOARD_REMOVE_MASK(BITBOARD, BIT_MASK(BIT))
#define BITBOARD_ADD_BIT(BITBOARD, BIT) BITBOARD_ADD_MASK(BITBOARD, BIT_MASK(BIT))

#define FLIP_BOOL(BOOL) do { BOOL ^= 1; } while (false)

#define DIFFERENCE(A, B) (A > B ? (A) - (B) : (B) - (A))

#define COLOR_OF(PIECE) (p_color)((PIECE) / 6)
#define PIECE_OF(PIECE) (p_piece_type)((PIECE) % 6)
#define PIECE_WITH(PIECE, COLOR) (p_piece)((PIECE) + (COLOR) * 6)

#define RANK_OF(SQUARE) ((SQUARE) / 8)
#define FILE_OF(SQUARE) ((SQUARE) % 8)
#define SQUARE_WITH(FILE, RANK) ((FILE) + (RANK) * 8)

#define POP(BB) __builtin_popcountll(BB)
#define CTZ(BB) __builtin_ctzll(BB)

#define ELEMENTS_OF(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

#endif

