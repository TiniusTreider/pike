#include "board.h"
#include "structboard.h"
#include "macros.h"
#include "tables.h"
#include "error.h"
#include "memory.h"

#include <stdlib.h>
#include <string.h>

static constexpr p_index BEHIND[2] = { (p_index)-8, 8 };

static constexpr p_index LONG_CASTLE_ROOK_END[2] = { 3, 59 };
static constexpr p_index SHORT_CASTLE_ROOK_END[2] = { 5, 61 };
static constexpr p_index LONG_CASTLE_ROOK_START[2] = { 0, 56 };
static constexpr p_index SHORT_CASTLE_ROOK_START[2] = { 7, 63 };


static inline void make_special_move(
        p_board board, p_move move, p_piece from, p_bitboard *friendly, p_bitboard *enemy
) {
        switch (move.flags) {
                case MOVE_PROMOTION:
                        {
                                const p_bitboard mask = BIT_MASK(move.to);
                                const p_piece piece = PIECE_WITH(move.promotion, board->player);

                                BITBOARD_REMOVE_MASK(board->bitboards[from], mask);
                                BITBOARD_ADD_MASK(board->bitboards[piece], mask);
                                board->mailbox[move.to] = piece;
                        } break;
                case MOVE_EN_PASSANT:
                        {
                                const p_piece pawn = PIECE_WITH(PAWN, !board->player);
                                const p_index square = board->ep_square + BEHIND[board->player];

                                BITBOARD_REMOVE_BIT(board->bitboards[pawn], square);
                                board->mailbox[square] = EMPTY;
                                BITBOARD_REMOVE_BIT(board->all_pieces, square);
                                BITBOARD_REMOVE_BIT(*enemy, square);
                        } break;
                case MOVE_LONG_CASTLE:
                        {
                                const p_index rook_start = LONG_CASTLE_ROOK_START[board->player];
                                const p_index rook_end = LONG_CASTLE_ROOK_END[board->player];
                                const p_piece rook = PIECE_WITH(ROOK, board->player);

                                BITBOARD_REMOVE_BIT(board->bitboards[rook], rook_start);
                                board->mailbox[rook_start] = EMPTY;
                                BITBOARD_REMOVE_BIT(board->all_pieces, rook_start);
                                BITBOARD_REMOVE_BIT(*friendly, rook_start);

                                BITBOARD_ADD_BIT(board->bitboards[rook], rook_end);
                                board->mailbox[rook_end] = rook;
                                BITBOARD_ADD_BIT(board->all_pieces, rook_end);
                                BITBOARD_ADD_BIT(*friendly, rook_end);

                        } break;
                case MOVE_SHORT_CASTLE:
                        {
                                const p_index rook_start = SHORT_CASTLE_ROOK_START[board->player];
                                const p_index rook_end = SHORT_CASTLE_ROOK_END[board->player];
                                const p_piece rook = PIECE_WITH(ROOK, board->player);

                                BITBOARD_REMOVE_BIT(board->bitboards[rook], rook_start);
                                board->mailbox[rook_start] = EMPTY;
                                BITBOARD_REMOVE_BIT(board->all_pieces, rook_start);
                                BITBOARD_REMOVE_BIT(*friendly, rook_start);

                                BITBOARD_ADD_BIT(board->bitboards[rook], rook_end);
                                board->mailbox[rook_end] = rook;
                                BITBOARD_ADD_BIT(board->all_pieces, rook_end);
                                BITBOARD_ADD_BIT(*friendly, rook_end);
                        } break;

                default:
                        errorf("invalid move flag %u", move.flags);
        }
}

p_unmake make_move(p_board board, p_move move)
{
        const p_piece from = board->mailbox[move.from];
        const p_piece to = board->mailbox[move.to];

        p_bitboard *friendly = board->player ?
                &board->black_pieces :
                &board->white_pieces;
        p_bitboard *enemy = board->player ?
                &board->white_pieces :
                &board->black_pieces;

        // save auxillary information

        const p_unmake data = (p_unmake){
                .captured_piece = to,
                .castling_rights = board->castling_rights,
                .ep_square = board->ep_square
        };

        // update board state

        BITBOARD_REMOVE_BIT(board->bitboards[from], move.from);
        board->mailbox[move.from] = EMPTY;
        BITBOARD_REMOVE_BIT(board->all_pieces, move.from);
        BITBOARD_REMOVE_BIT(*friendly, move.from);

        BITBOARD_REMOVE_BIT(board->bitboards[to], move.to);
        BITBOARD_ADD_BIT(board->bitboards[from], move.to);
        board->mailbox[move.to] = from;
        BITBOARD_ADD_BIT(board->all_pieces, move.to);
        BITBOARD_REMOVE_BIT(*enemy, move.to);
        BITBOARD_ADD_BIT(*friendly, move.to);

        if (move.flags)
                make_special_move(board, move, from, friendly, enemy);

        // update auxillary information

        if (PIECE_OF(from) == PAWN && DIFFERENCE(move.to, move.from) == 16)
                board->ep_square = move.from + -16 * board->player + 8;
        else
                board->ep_square = NO_SQUARE;

        board->castling_rights &= CASTLING_RIGHTS_TABLE[move.to] & CASTLING_RIGHTS_TABLE[move.from];

        FLIP_BOOL(board->player);

        return data;
}

static inline void unmake_special_move(
        p_board board, p_move move, p_bitboard *friendly, p_bitboard *enemy
) {
        switch (move.flags) {
                case MOVE_PROMOTION:
                        {
                                const p_bitboard mask = BIT_MASK(move.to);
                                const p_piece pawn = PIECE_WITH(PAWN, board->player);
                                const p_piece piece = PIECE_WITH(move.promotion, board->player);

                                BITBOARD_ADD_MASK(board->bitboards[pawn], mask);
                                BITBOARD_REMOVE_MASK(board->bitboards[piece], mask);
                                board->mailbox[move.to] = pawn;

                        } break;
                case MOVE_EN_PASSANT:
                        {
                                const p_piece pawn = PIECE_WITH(PAWN, !board->player);
                                const p_index square = board->ep_square + BEHIND[board->player];

                                BITBOARD_ADD_BIT(board->bitboards[pawn], square);
                                board->mailbox[square] = pawn;
                                BITBOARD_ADD_BIT(board->all_pieces, square);
                                BITBOARD_ADD_BIT(*enemy, square);
                        } break;
                case MOVE_LONG_CASTLE:
                        {
                                const p_index rook_start = LONG_CASTLE_ROOK_START[board->player];
                                const p_index rook_end = LONG_CASTLE_ROOK_END[board->player];
                                const p_piece rook = PIECE_WITH(ROOK, board->player);

                                BITBOARD_ADD_BIT(board->bitboards[rook], rook_start);
                                board->mailbox[rook_start] = rook;
                                BITBOARD_ADD_BIT(board->all_pieces, rook_start);
                                BITBOARD_ADD_BIT(*friendly, rook_start);

                                BITBOARD_REMOVE_BIT(board->bitboards[rook], rook_end);
                                board->mailbox[rook_end] = EMPTY;
                                BITBOARD_REMOVE_BIT(board->all_pieces, rook_end);
                                BITBOARD_REMOVE_BIT(*friendly, rook_end);
                        } break;
                case MOVE_SHORT_CASTLE:
                        {
                                const p_index rook_start = SHORT_CASTLE_ROOK_START[board->player];
                                const p_index rook_end = SHORT_CASTLE_ROOK_END[board->player];
                                const p_piece rook = PIECE_WITH(ROOK, board->player);

                                BITBOARD_ADD_BIT(board->bitboards[rook], rook_start);
                                board->mailbox[rook_start] = rook;
                                BITBOARD_ADD_BIT(board->all_pieces, rook_start);
                                BITBOARD_ADD_BIT(*friendly, rook_start);

                                BITBOARD_REMOVE_BIT(board->bitboards[rook], rook_end);
                                board->mailbox[rook_end] = EMPTY;
                                BITBOARD_REMOVE_BIT(board->all_pieces, rook_end);
                                BITBOARD_REMOVE_BIT(*friendly, rook_end);
                        } break;

                default:
                        errorf("invalid move flag %u", move.flags);
        }
}

void unmake_move(p_board board, p_move move, p_unmake data)
{
        // restore auxillary information

        FLIP_BOOL(board->player);

        board->castling_rights = data.castling_rights;
        board->ep_square = data.ep_square;

        // restore board state

        p_bitboard *friendly = board->player ?
                &board->black_pieces :
                &board->white_pieces;
        p_bitboard *enemy = board->player ?
                &board->white_pieces :
                &board->black_pieces;

        if (move.flags)
                unmake_special_move(board, move, friendly, enemy);

        const p_piece from = board->mailbox[move.to];
        const p_piece to = data.captured_piece;

        BITBOARD_ADD_BIT(board->bitboards[to], move.to);
        BITBOARD_REMOVE_BIT(board->bitboards[from], move.to);
        board->mailbox[move.to] = to;
        if (to == EMPTY)
                BITBOARD_REMOVE_BIT(board->all_pieces, move.to);
        else
                BITBOARD_ADD_BIT(*enemy, move.to);
        BITBOARD_REMOVE_BIT(*friendly, move.to);

        BITBOARD_ADD_BIT(board->bitboards[from], move.from);
        board->mailbox[move.from] = from;
        BITBOARD_ADD_BIT(board->all_pieces, move.from);
        BITBOARD_ADD_BIT(*friendly, move.from);
}

p_board init_board(char *fen)
{
        p_board board = smalloc(sizeof(struct board));

        set_board(board, fen);

        return board;
}

static constexpr p_piece_type ALGEBRAIC['R' - 'B' + 1] = {
        BISHOP, 0, 0, 0, 0, 0, 0, 0, 0,
        KING, 0, 0,
        KNIGHT, 0,
        PAWN,
        QUEEN,
        ROOK
};

void set_board(p_board board, char *fen_string)
{
        char fen[128];
        const size_t size = strlen(fen_string);
        memcpy(fen, fen_string, size + 1);

        char *fen_position = strtok(fen, " ");
        if (*fen_position == '\0' || fen_position == NULL)
                errorf("invalid 1st segment of FEN \"%s\"", fen);

        memset(board->mailbox, EMPTY, 64 * sizeof(p_piece));
        memset(board->bitboards, 0, 12 * sizeof(p_bitboard));
        board->all_pieces = 0;
        board->white_pieces = 0;
        board->black_pieces = 0;
        p_index square = 0;
        do {
                const char letter = *fen_position;
                if (letter == '\0')
                        break;
                if (letter == '/')
                        continue;

                if (letter >= '1' && letter <= '8') {
                        square += letter - '1' + 1;
                        continue;
                }

                const p_color color = letter >= 'a' ? BLACK : WHITE;
                const p_piece_type type = ALGEBRAIC[(letter - 'B') % 32];
                const p_piece piece = PIECE_WITH(type, color);

                const p_index index = SQUARE_WITH(FILE_OF(square), (7 - RANK_OF(square)));

                BITBOARD_ADD_BIT(board->bitboards[piece], index);
                board->mailbox[index] = piece;
                BITBOARD_ADD_BIT(board->all_pieces, index);
                if (color == WHITE) {
                        BITBOARD_ADD_BIT(board->white_pieces, index);
                } else {
                        BITBOARD_ADD_BIT(board->black_pieces, index);
                }

                square++;
        } while (*++fen_position);

        char *fen_stm = strtok(NULL, " ");
        if (*fen_stm == '\0' || fen_stm == NULL)
                errorf("invalid 2nd segment of FEN \"%s\"", fen);

        if (*fen_stm == 'w') {
                board->player = WHITE;
        } else if (*fen_stm == 'b') {
                board->player = BLACK;
        } else {
                errorf("invalid char '%c' in 2nd letter of FEN \"%s\"", *fen_stm, fen);
        }

        char *fen_castling_rights = strtok(NULL, " ");
        if (*fen_castling_rights == '\0' || fen_castling_rights == NULL)
                errorf("invalid 3rd segment of FEN \"%s\"", fen);

        board->castling_rights = 0;
        do {
                const char letter = *fen_castling_rights;
                switch (letter) {
                        case '-':
                                fen_castling_rights[1] = '\0'; break;
                        case 'K':
                                board->castling_rights |= W_SHORT_CASTLE; break;
                        case 'Q':
                                board->castling_rights |= W_LONG_CASTLE; break;
                        case 'k':
                                board->castling_rights |= B_SHORT_CASTLE; break;
                        case 'q':
                                board->castling_rights |= B_LONG_CASTLE; break;

                        default:
                                errorf(
                                        "invalid character '%c' in 3rd segment of FEN \"%s\"",
                                        letter, fen
                                );
                }
        } while (*++fen_castling_rights);

        char *fen_ep_square = strtok(NULL, " ");
        if (*fen_ep_square == '\0' || fen_ep_square == NULL)
                errorf("invalid 4th segment of FEN \"%s\"", fen);

        if (*fen_ep_square == '-') {
                board->ep_square = NO_SQUARE;
        } else {
                const p_index file = fen_ep_square[0];
                const p_index rank = fen_ep_square[1];

                if (file > 7 || rank > 7)
                        errorf("invalid 4th segment of FEN \"%s\"", fen);

                board->ep_square = SQUARE_WITH(file, rank);
        }
}

void clean_board(p_board board)
{
        free(board);
}

