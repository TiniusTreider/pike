#define _POSIX_C_SOURCE 200809L

#include "board.h"
#include "structboard.h"
#include "macros.h"
#include "tables.h"
#include "error.h"
#include "memory.h"
#include "debug.h"
#include "zobrist.h"

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

                                board->zobrist ^= (
                                        piece_hash[move.to][from] ^ piece_hash[move.to][piece]
                                );
                        } break;
                case MOVE_EN_PASSANT:
                        {
                                const p_piece pawn = PIECE_WITH(PAWN, !board->player);
                                const p_index square = board->ep_square + BEHIND[board->player];

                                BITBOARD_REMOVE_BIT(board->bitboards[pawn], square);
                                board->mailbox[square] = EMPTY;
                                BITBOARD_REMOVE_BIT(board->all_pieces, square);
                                BITBOARD_REMOVE_BIT(*enemy, square);

                                board->zobrist ^= piece_hash[square][pawn];
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

                                board->zobrist ^= (
                                        piece_hash[rook_start][rook] ^ piece_hash[rook_end][rook]
                                );

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

                                board->zobrist ^= (
                                        piece_hash[rook_start][rook] ^ piece_hash[rook_end][rook]
                                );
                        } break;

                default:
                        errorf("invalid move flag %u", move.flags);
        }
}

p_unmake make_move(p_board board, p_move move)
{
        // update board state

        const p_piece from = board->mailbox[move.from];
        const p_piece to = board->mailbox[move.to];

        board->mailbox[move.to] = from;
        board->mailbox[move.from] = EMPTY;

        BITBOARD_REMOVE_BIT(board->bitboards[from], move.from);
        BITBOARD_ADD_BIT(board->bitboards[from], move.to);
        BITBOARD_REMOVE_BIT(board->bitboards[to], move.to);

        BITBOARD_REMOVE_BIT(board->all_pieces, move.from);
        BITBOARD_ADD_BIT(board->all_pieces, move.to);

        p_bitboard *friendly = board->player ?
                &board->black_pieces :
                &board->white_pieces;
        p_bitboard *enemy = board->player ?
                &board->white_pieces :
                &board->black_pieces;

        BITBOARD_REMOVE_BIT(*friendly, move.from);
        BITBOARD_ADD_BIT(*friendly, move.to);

        BITBOARD_REMOVE_BIT(*enemy, move.to);

        // save auxillary information

        board->history_end = (board->history_end + 1) % 100;

        const p_unmake data = (p_unmake){
                .captured_piece = to,
                .castling_rights = board->castling_rights,
                .ep_square = board->ep_square,
                .zobrist = board->zobrist,
                .history = board->history[board->history_end],
                .history_start = board->history_start
        };

        if (move.flags)
                make_special_move(board, move, from, friendly, enemy);

        // update auxillary information

        board->zobrist ^= (
                piece_hash[move.from][from] ^
                piece_hash[move.to][to] ^
                piece_hash[move.to][from] ^
                black_hash ^
                (board->ep_square == NO_SQUARE ? 0 : ep_hash[FILE_OF(board->ep_square)]) ^
                castling_hash[board->castling_rights]
        );

        if (PIECE_OF(from) == PAWN && DIFFERENCE(move.to, move.from) == 16) {
                board->ep_square = move.from + -16 * board->player + 8;
                board->zobrist ^= ep_hash[FILE_OF(board->ep_square)];
        } else {
                board->ep_square = NO_SQUARE;
        }

        board->castling_rights &= CASTLING_RIGHTS_TABLE[move.to] & CASTLING_RIGHTS_TABLE[move.from];
        board->zobrist ^= castling_hash[board->castling_rights];

        FLIP_BOOL(board->player);

        board->history[board->history_end] = board->zobrist;
        if (PIECE_OF(from) == PAWN || to != EMPTY)
                board->history_start = board->history_end;

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
        board->zobrist = data.zobrist;
        board->history_start = data.history_start;
        board->history[board->history_end] = data.history;

        board->history_end = (board->history_end + 99) % 100;

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

        board->mailbox[move.from] = from;
        board->mailbox[move.to] = to;

        BITBOARD_REMOVE_BIT(board->bitboards[from], move.to);
        BITBOARD_ADD_BIT(board->bitboards[from], move.from);
        BITBOARD_ADD_BIT(board->bitboards[to], move.to);

        BITBOARD_REMOVE_BIT(*friendly, move.to);
        BITBOARD_ADD_BIT(*friendly, move.from);

        if (to == EMPTY) {
                BITBOARD_REMOVE_BIT(board->all_pieces, move.to);
        } else {
                BITBOARD_ADD_BIT(*enemy, move.to);
        }
        BITBOARD_ADD_BIT(board->all_pieces, move.from);
}

p_board init_board(char *fen)
{
        p_board board = smalloc(sizeof(struct board_struct));

        set_board(board, fen);

        return board;
}

#define DELIM " \t"
#define FEN_BUFFER_SIZE 128

void set_board(p_board board, char *fen_string)
{
        LOG("parsing fen");

        char fen[FEN_BUFFER_SIZE] = "";
        const size_t size = strlen(fen_string);
        if (size + 1 > FEN_BUFFER_SIZE) {
                LOG("fen too big for buffer");
                return;
        }
        memcpy(fen, fen_string, size + 1);

        char *strtok_ptr = NULL;
        char *fen_position = strtok_r(fen, DELIM, &strtok_ptr);
        if (fen_position == NULL || *fen_position == '\0') {
                LOG("no first fen chunk");
                return;
        }

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

        LOG("parsed first fen chunk");

        char *fen_stm = strtok_r(NULL, DELIM, &strtok_ptr);
        if (fen_stm == NULL || *fen_stm == '\0') {
                LOG("no second fen chunk");
                return;
        }

        if (*fen_stm == 'w') {
                board->player = WHITE;
        } else if (*fen_stm == 'b') {
                board->player = BLACK;
        } else {
                LOG("unknown color to move in fen");
                return;
        }

        LOG("parsed second fen chunk");

        char *fen_castling_rights = strtok_r(NULL, DELIM, &strtok_ptr);
        if (fen_castling_rights == NULL || *fen_castling_rights == '\0') {
                LOG("no third fen chunk");
                return;
        }

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
                                LOG("unknown castling rights");
                                return;
                }
        } while (*++fen_castling_rights);

        LOG("parsed third fen chunk");

        char *fen_ep_square = strtok_r(NULL, DELIM, &strtok_ptr);
        if (fen_ep_square == NULL || *fen_ep_square == '\0') {
                LOG("no fourth fen chunk");
                return;
        }

        if (*fen_ep_square == '-') {
                board->ep_square = NO_SQUARE;
        } else {
                const p_index file = fen_ep_square[0] - 'a';
                const p_index rank = fen_ep_square[1] - '1';

                if (file > 7 || rank > 7) {
                        LOG("unknown ep square in fen");
                        return;
                }

                board->ep_square = SQUARE_WITH(file, rank);
        }

        LOG("parsed fourth fen chunk");
        LOG("finished parsing fen");

        board->zobrist = 0;

        for (size_t i = 0; i < 64; i++)
        {
                board->zobrist ^= piece_hash[i][board->mailbox[i]];
        }

        board->zobrist ^= board->player ? 0 : black_hash;
        board->zobrist ^= castling_hash[board->castling_rights];
        board->zobrist ^= board->ep_square == NO_SQUARE ? 0 : ep_hash[FILE_OF(board->ep_square)];

        board->history_start = 0;
        board->history_end = 0;
        board->history[0] = board->zobrist;
}

void clean_board(p_board board)
{
        free(board);
}

bool is_repeated(p_board board)
{
        if ((board->history_end + 1) % 100 == board->history_start)
                return true;

        size_t count = 0;
        for (
                size_t i = (board->history_end + 1) % 100;
                i != board->history_start;
                i = (i + 100 + 99) % 100
        ) {
                if (board->history[i] == board->zobrist) {
                        if (count)
                                return true;

                        count++;
                }
        }

        return false;
}

