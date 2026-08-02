#include "movegen.h"
#include "structboard.h"
#include "macros.h"
#include "tables.h"

#include <immintrin.h>
#include <string.h>
#include <assert.h>

static inline p_index pop_bit(p_bitboard *bitboard)
{
        const p_index pos = CTZ(*bitboard);

        *bitboard &= *bitboard - 1;

        return pos;
}

#define PROMOTION_PIECES_X \
        X(KNIGHT) \
        X(BISHOP) \
        X(ROOK) \
        X(QUEEN)

static inline void push_promotions(p_move *buffer, size_t *move_count, p_index from, p_index to)
{
#define X(PIECE) \
        buffer[(*move_count)++] = (p_move){ \
                .from = from, \
                .to = to, \
                .promotion = PIECE, \
                .flags = MOVE_PROMOTION \
        };

        PROMOTION_PIECES_X

#undef X
}

static inline p_bitboard is_square_attacked(p_board board, p_index square)
{
        // pawn

        const p_bitboard target = BIT_MASK(square);
        const p_bitboard left_target = target & ~FILE_H;
        const p_bitboard right_target = target & ~FILE_A;
        const p_bitboard pawns = board->bitboards[PIECE_WITH(PAWN, !board->player)];
        const p_bitboard checking_pawns = pawns & (board->player ?
                (right_target >> 9) | (left_target >> 7) :
                (right_target << 7) | (left_target << 9)
        );
        if (checking_pawns)
                return checking_pawns;

        // knight

        const p_bitboard checking_knights = (
                KNIGHT_MOVE_TABLE[square] &
                board->bitboards[PIECE_WITH(KNIGHT, !board->player)]
        );
        if (checking_knights)
                return checking_knights;

        // bishop

        const p_bitboard king = board->bitboards[PIECE_WITH(KING, board->player)];

        const p_bitboard bishops = (
                board->bitboards[PIECE_WITH(BISHOP, !board->player)] |
                board->bitboards[PIECE_WITH(QUEEN, !board->player)]
        );
        const size_t bishop_index = _pext_u64(board->all_pieces ^ king, BISHOP_MOVE_TABLE[square]);
        const p_bitboard checking_bishops = bishops & BISHOP_PEXT_TABLE[square][bishop_index];
        if (checking_bishops)
                return checking_bishops;

        // rook

        const p_bitboard rooks = (
                board->bitboards[PIECE_WITH(ROOK, !board->player)] |
                board->bitboards[PIECE_WITH(QUEEN, !board->player)]
        );
        const size_t rook_index = _pext_u64(board->all_pieces ^ king, ROOK_MOVE_TABLE[square]);
        const p_bitboard checking_rooks = rooks & ROOK_PEXT_TABLE[square][rook_index];
        if (checking_rooks)
                return checking_rooks;

        // king

        const p_bitboard other_king = board->bitboards[PIECE_WITH(KING, !board->player)];
        if (other_king & KING_MOVE_TABLE[square])
                return other_king;

        return 0ULL;
}


#define GEN_PINS(LOWER, UPPER) do { \
        const p_bitboard friendly = board->player ? \
                board->black_pieces : \
                board->white_pieces; \
        const p_bitboard enemy = board->player ? \
                board->white_pieces : \
                board->black_pieces; \
 \
        const p_index king_pos = CTZ(board->bitboards[PIECE_WITH(KING, board->player)]); \
        const p_bitboard LOWER ## s = ( \
                board->bitboards[PIECE_WITH(UPPER, !board->player)] | \
                board->bitboards[PIECE_WITH(QUEEN, !board->player)] \
        ); \
 \
        const size_t index = _pext_u64(LOWER ## s, UPPER ## _MOVE_TABLE[king_pos]); \
        const p_bitboard mask = UPPER ## _PEXT_TABLE[king_pos][index]; \
 \
        p_bitboard pins = 0ULL; \
        p_bitboard attackers = LOWER ## s & mask; \
 \
        while (attackers) \
        { \
                const p_index pos = pop_bit(&attackers); \
 \
                const p_bitboard pos_mask = BIT_MASK(pos); \
                const p_bitboard ray = BETWEEN_TABLE[pos][king_pos] | pos_mask; \
 \
                if (POP(enemy & ray) != 1) \
                        continue; \
 \
                const p_bitboard blockers = friendly & ray; \
                const p_index population = POP(blockers); \
                if (population == 1) { \
                        const p_index pin = CTZ(blockers); \
                        rays[pin] = ray; \
                        BITBOARD_ADD_BIT(pins, pin); \
                } else if (population == 0) { \
                        *checkers |= pos_mask; \
                } \
        } \
 \
        return pins; \
\
} while (false)

static inline p_bitboard bishop_pins(p_board board, p_bitboard rays[64], p_bitboard *checkers)
{
        GEN_PINS(bishop, BISHOP);
}

static inline p_bitboard rook_pins(p_board board, p_bitboard rays[64], p_bitboard *checkers)
{
        GEN_PINS(rook, ROOK);
}

static constexpr p_index BEHIND[2] = { (p_index)-8, 8 };

static constexpr p_bitboard LONG_CASTLE_MASK[2] = { 0x000000000000000EULL, 0x0E00000000000000ULL };
static constexpr p_bitboard SHORT_CASTLE_MASK[2] = { 0x0000000000000060ULL, 0x6000000000000000ULL };
static constexpr p_index KING_START_SQUARE[2] = { 4, 60 };
static constexpr p_index LONG_CASTLE_INT_SQUARE[2] = { 3, 59 };
static constexpr p_index LONG_CASTLE_END_SQUARE[2] = { 2, 58 };
static constexpr p_index SHORT_CASTLE_INT_SQUARE[2] = { 5, 61 };
static constexpr p_index SHORT_CASTLE_END_SQUARE[2] = { 6, 62 };

#define A \
        const p_bitboard end_mask = BIT_MASK(endpos);
#define B \
        if (!(end_mask & check_ray)) \
                continue;
#define C \
        const p_bitboard start_mask = BIT_MASK(startpos); \
        if (start_mask & bishop_pin_board) \
                continue; \
        if (start_mask & rook_pin_board && !(end_mask & pin_rays[startpos])) \
                continue;
#define D \
        const p_bitboard start_mask = BIT_MASK(startpos); \
        if (start_mask & bishop_pin_board && !(end_mask & pin_rays[startpos])) \
                continue; \
        if (start_mask & rook_pin_board) \
                continue;
#define E \
        move_board &= check_ray;
#define F
#define G \
        king:
#define H(LABEL) \
        if (!((end_mask | ep_pawn_bit) & check_ray)) \
                goto LABEL;
#define I(LABEL) \
        const p_bitboard end_mask = BIT_MASK(endpos); \
        if (start_mask & bishop_pin_board && !(end_mask & pin_rays[startpos])) \
                goto LABEL;
#define J \
        const p_bitboard ep_pawn_bit = BIT_MASK(ep_pawn);
#define K \
        const p_bitboard mask = BIT_MASK(startpos); \
        if (mask & rook_pin_board) \
                continue;
#define L \
        const p_bitboard mask = BIT_MASK(startpos); \
        if (mask & bishop_pin_board) \
                continue;
#define M \
        if (mask & bishop_pin_board && !(BIT_MASK(endpos) & pin_rays[startpos])) \
                continue;
#define N \
        if (mask & rook_pin_board && !(BIT_MASK(endpos) & pin_rays[startpos])) \
                continue;

#define PUSH_VANILLA_MOVE \
                buffer[move_count++] = (p_move){ \
                        .from = startpos, \
                        .to = endpos \
                };
#define PUSH_EN_PASSANT \
                buffer[move_count++] = (p_move){ \
                        .from = startpos, \
                        .to = endpos, \
                        .flags = MOVE_EN_PASSANT \
                };

#define GEN_CASTLING \
        if ( \
                board->castling_rights & (W_LONG_CASTLE << (board->player * 2)) && \
                !(board->all_pieces & LONG_CASTLE_MASK[board->player]) && \
                !is_square_attacked(board, LONG_CASTLE_INT_SQUARE[board->player]) && \
                !is_square_attacked(board, LONG_CASTLE_END_SQUARE[board->player]) \
        ) { \
                buffer[move_count++] = (p_move){ \
                        .from = KING_START_SQUARE[board->player], \
                        .to = LONG_CASTLE_END_SQUARE[board->player], \
                        .flags = MOVE_LONG_CASTLE \
                }; \
        } \
 \
        if ( \
                board->castling_rights & (W_SHORT_CASTLE << (board->player * 2)) && \
                !(board->all_pieces & SHORT_CASTLE_MASK[board->player]) && \
                !is_square_attacked(board, SHORT_CASTLE_INT_SQUARE[board->player]) && \
                !is_square_attacked(board, SHORT_CASTLE_END_SQUARE[board->player]) \
        ) { \
                buffer[move_count++] = (p_move){ \
                        .from = KING_START_SQUARE[board->player], \
                        .to = SHORT_CASTLE_END_SQUARE[board->player], \
                        .flags = MOVE_SHORT_CASTLE \
                }; \
        }

#define GEN_MOVES(NAME) do { \
        /* pawn */ \
 \
        p_bitboard single_push = board->player ? \
                pawn >> 8 : \
                pawn << 8; \
        const p_bitboard PROMOTION_RANKS = RANK_1 | RANK_8; \
        single_push &= ~board->all_pieces; \
        p_bitboard single_push_promotion = single_push & PROMOTION_RANKS; \
        p_bitboard double_push = single_push; \
        single_push &= ~PROMOTION_RANKS; \
 \
        while (single_push) \
        { \
                const p_index endpos = pop_bit(&single_push); \
                const p_index startpos = board->player ? endpos + 8 : endpos - 8; \
 \
                A B C \
 \
                PUSH_VANILLA_MOVE \
        } \
 \
        while(single_push_promotion) \
        { \
                const p_index endpos = pop_bit(&single_push_promotion); \
                const p_index startpos = board->player ? endpos + 8 : endpos - 8; \
 \
                A B C \
 \
                push_promotions(buffer, &move_count, startpos, endpos); \
        } \
 \
        double_push = board->player ? \
                (double_push >> 8) & RANK_5 : \
                (double_push << 8) & RANK_4; \
        double_push &= ~board->all_pieces; \
 \
        while (double_push) \
        { \
                const p_index endpos = pop_bit(&double_push); \
                const p_index startpos = board->player ? endpos + 16 : endpos - 16; \
 \
                A B C \
 \
                PUSH_VANILLA_MOVE \
        } \
 \
        p_bitboard capture_left = board->player ? \
                pawn >> 9 : \
                pawn << 7; \
 \
        capture_left &= ~FILE_H; \
        const p_bitboard ep_bit = board->ep_square == NO_SQUARE ? 0ULL : BIT_MASK(board->ep_square); \
        p_bitboard ep_left = capture_left & ep_bit; \
        capture_left &= enemy; \
        p_bitboard capture_left_promotion = capture_left & PROMOTION_RANKS; \
        capture_left &= ~PROMOTION_RANKS; \
 \
        while (capture_left) \
        { \
                const p_index endpos = pop_bit(&capture_left); \
                const p_index startpos = board->player ? endpos + 9 : endpos - 7; \
 \
                A B D \
 \
                PUSH_VANILLA_MOVE \
        } \
 \
        while(capture_left_promotion) \
        { \
                const p_index endpos = pop_bit(&capture_left_promotion); \
                const p_index startpos = board->player ? endpos + 9 : endpos - 7; \
 \
                A B D \
 \
                push_promotions(buffer, &move_count, startpos, endpos); \
        } \
 \
        p_bitboard capture_right = board->player ? \
                pawn >> 7 : \
                pawn << 9; \
        capture_right &= ~FILE_A;p_bitboard ep_right = capture_right & ep_bit; \
        capture_right &= enemy; \
        p_bitboard capture_right_promotion = capture_right & PROMOTION_RANKS; \
        capture_right &= ~PROMOTION_RANKS; \
 \
 \
        while (capture_right) \
        { \
                const p_index endpos = pop_bit(&capture_right); \
                const p_index startpos = board->player ? endpos + 7 : endpos - 9; \
 \
                A B D \
 \
                PUSH_VANILLA_MOVE \
        } \
 \
        while(capture_right_promotion) \
        { \
                const p_index endpos = pop_bit(&capture_right_promotion); \
                const p_index startpos = board->player ? endpos + 7 : endpos - 9; \
 \
                A B D \
 \
                push_promotions(buffer, &move_count, startpos, endpos); \
        } \
 \
        if (ep_bit) \
        { \
                p_bitboard *enemy_ptr = board->player ? \
                        &board->white_pieces : \
                        &board->black_pieces; \
                const p_index ep_pawn = board->ep_square + BEHIND[board->player]; \
                J \
 \
                p_bitboard ep_pin_rays[64]; \
                memset(ep_pin_rays, 0, 64 * sizeof(p_bitboard)); \
                p_bitboard ep_checkers = 0ULL; \
 \
                BITBOARD_REMOVE_BIT(*enemy_ptr, ep_pawn); \
                const p_bitboard ep_rook_pin_board = rook_pins(board, ep_pin_rays, &ep_checkers); \
                BITBOARD_ADD_BIT(*enemy_ptr, ep_pawn); \
 \
                if (ep_left) \
                { \
                        const p_index endpos = CTZ(ep_left); \
                        const p_index startpos = board->player ? endpos + 9 : endpos - 7; \
 \
                        const p_bitboard start_mask = BIT_MASK(startpos); \
 \
                        I(NAME ## _ep_right) H(NAME ## _ep_right) \
 \
                        if (start_mask & ep_rook_pin_board) \
                                goto NAME ## _ep_right; \
 \
                        PUSH_EN_PASSANT \
                } \
 \
NAME ## _ep_right: \
 \
                if (ep_right) \
                { \
                        const p_index endpos = CTZ(ep_right); \
                        const p_index startpos = board->player ? endpos + 7 : endpos - 9; \
 \
                        const p_bitboard start_mask = BIT_MASK(startpos); \
 \
                        I(NAME ## _knight) H(NAME ## _knight) \
 \
                        if (start_mask & ep_rook_pin_board) \
                                goto NAME ## _knight; \
 \
                        PUSH_EN_PASSANT \
 \
                } \
        } \
 \
NAME ## _knight: /* knight */ \
 \
        p_bitboard knight_board = board->bitboards[PIECE_WITH(KNIGHT, board->player)]; \
 \
        while (knight_board) \
        { \
                const p_index startpos = pop_bit(&knight_board); \
 \
                const p_bitboard mask = BIT_MASK(startpos); \
                if (mask & bishop_pin_board || mask & rook_pin_board) \
                        continue; \
 \
                p_bitboard move_board = KNIGHT_MOVE_TABLE[startpos] & ~friendly; \
 \
                E \
 \
                while (move_board) \
                { \
                        const p_index endpos = pop_bit(&move_board); \
 \
                        PUSH_VANILLA_MOVE \
                } \
        } \
 \
        /* bishop */ \
 \
        const p_bitboard queen_board = board->bitboards[PIECE_WITH(QUEEN, board->player)]; \
 \
        p_bitboard bishop_board = board->bitboards[PIECE_WITH(BISHOP, board->player)] | queen_board; \
 \
        while (bishop_board) \
        { \
                const p_index startpos = pop_bit(&bishop_board); \
 \
                const size_t index = _pext_u64(board->all_pieces, BISHOP_MOVE_TABLE[startpos]); \
                p_bitboard move_board = BISHOP_PEXT_TABLE[startpos][index] & ~friendly; \
 \
                E K \
 \
                while (move_board) \
                { \
                        const p_index endpos = pop_bit(&move_board); \
 \
                        M \
 \
                        PUSH_VANILLA_MOVE \
                } \
        } \
 \
        /* rook */ \
 \
        p_bitboard rook_board = board->bitboards[PIECE_WITH(ROOK, board->player)] | queen_board; \
 \
        while (rook_board) \
        { \
                const p_index startpos = pop_bit(&rook_board); \
 \
                const size_t index = _pext_u64(board->all_pieces, ROOK_MOVE_TABLE[startpos]); \
                p_bitboard move_board = ROOK_PEXT_TABLE[startpos][index] & ~friendly; \
 \
                E L \
 \
                while (move_board) \
                { \
                        const p_index endpos = pop_bit(&move_board); \
 \
                        N \
 \
                        PUSH_VANILLA_MOVE \
                } \
        } \
 \
 G /* king */ \
 \
        p_bitboard king_board = board->bitboards[PIECE_WITH(KING, board->player)]; \
 \
        while (king_board) \
        { \
                const p_index startpos = pop_bit(&king_board); \
 \
                p_bitboard move_board = KING_MOVE_TABLE[startpos] & ~friendly; \
 \
                while (move_board) \
                { \
                        const p_index endpos = pop_bit(&move_board); \
 \
                        if (!is_square_attacked(board, endpos)) { \
                                buffer[move_count++] = (p_move){ \
                                        .from = startpos, \
                                        .to = endpos \
                                }; \
                        } \
                } \
        } \
 \
        F \
\
} while (false)

size_t generate_moves(p_board board, p_move buffer[218])
{
        // pins and checks

        p_bitboard pin_rays[64];
        memset(pin_rays, 0, 64 * sizeof(p_bitboard));
        p_bitboard checkers = 0ULL;

        const p_bitboard bishop_pin_board = bishop_pins(board, pin_rays, &checkers);
        const p_bitboard rook_pin_board = rook_pins(board, pin_rays, &checkers);
        const p_index king_pos = CTZ(board->bitboards[PIECE_WITH(KING, board->player)]);
        checkers |= is_square_attacked(board, king_pos);

        const p_bitboard check_ray = POP(checkers) == 1 ?
                BETWEEN_TABLE[king_pos][CTZ(checkers)] | checkers :
                0xFFFFFFFFFFFFFFFFULL;

        const p_bitboard friendly = board->player ?
                board->black_pieces :
                board->white_pieces;
        const p_bitboard enemy = board->player ?
                board->white_pieces :
                board->black_pieces;
        const p_bitboard pawn = board->bitboards[PIECE_WITH(PAWN, board->player)];

        size_t move_count = 0;

        if (POP(checkers) == 2)
                goto king;

        if (checkers) { // check

                GEN_MOVES(normal);

        } else if (bishop_pin_board | rook_pin_board) { // pin

#undef B
#undef E
#undef F
#undef G
#undef H
#undef J
#define B
#define E
#define F GEN_CASTLING
#define G
#define H(LABEL)
#define J

                GEN_MOVES(pin);

        } else { // normal

#undef A
#undef C
#undef D
#undef I
#undef K
#undef L
#undef M
#undef N
#define A
#define C
#define D
#define I(LABEL)
#define K
#define L
#define M
#define N

                GEN_MOVES(check);

        }

        return move_count;
}

