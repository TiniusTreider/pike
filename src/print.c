#include "print.h"
#include "macros.h"
#include "structboard.h"
#include "tables.h"

#include <stdio.h>
#include <string.h>

static constexpr char PIECE_CHARS[6] = { 'p', 'n', 'b', 'r', 'q', 'k' };

void print_move(p_move move, char string[6])
{
        if (IS_NULL_MOVE(move)) {
                sprintf(string, "0000");
                return;
        }

        sprintf(
                string,
                "%c%c%c%c",
                'a' + FILE_OF(move.from),
                '1' + RANK_OF(move.from),
                'a' + FILE_OF(move.to),
                '1' + RANK_OF(move.to)
        );

        if (move.flags & MOVE_PROMOTION)
        {
                string[4] = PIECE_CHARS[PIECE_OF(move.promotion)];
                string[5] = '\0';
        }
}

static const char *piece_strings[6] = {
        "♟ ", "♞ ", "♝ ", "♜ ", "♛ ", "♚ "
};

#define ASSERT(CONDITION) do { if (!(CONDITION)) return RETURN; } while (false)
#define RETURN NO_SQUARE

static inline p_index parse_square(char *string)
{
        const char file = string[0];
        const char rank = string[1];
        ASSERT(file >= 'a' && file <= 'h' && rank >= '1' && rank <= '8');

        return file - 'a' + (rank - '1') * 8;
}

#undef RETURN
#define RETURN NULL_MOVE

p_move parse_move(p_board board, char *string)
{
        size_t length = strlen(string);
        ASSERT(length == 4 || length == 5);

        p_move move;

        ASSERT((move.from = parse_square(string)) != NO_SQUARE);
        ASSERT((move.to = parse_square(string + 2)) != NO_SQUARE);

        if (length == 5)
                move.promotion = ALGEBRAIC[string[4] - 'b'];

        move.flags = 0;

        const p_piece_type piece = PIECE_OF(board->mailbox[move.from]);
        if (piece == PAWN) {
                if (FILE_OF(move.from) != FILE_OF(move.to) && board->mailbox[move.to] == EMPTY)
                        move.flags = MOVE_EN_PASSANT;
                else if (RANK_OF(move.to) == 0 || RANK_OF(move.to) == 7)
                        move.flags = MOVE_PROMOTION;
        } else if (piece == KING) {
                const p_index file = FILE_OF(move.to);
                if (file - 3 > 2) {
                        move.flags = file > 4 ? MOVE_SHORT_CASTLE : MOVE_LONG_CASTLE;
                }
        }

        return move;
}

#undef RETURN
#undef ASSERT_PARSE

#define LIGHT_SQUARE "\033[48;5;179m"
#define DARK_SQUARE "\033[48;5;130m"
#define LIGHT_PIECE "\033[38;5;253m"
#define DARK_PIECE "\033[38;5;234m"

static inline void draw_square(p_piece piece, p_color square_color)
{
        square_color ? printf(DARK_SQUARE) : printf(LIGHT_SQUARE);
        COLOR_OF(piece) ? printf(DARK_PIECE) : printf(LIGHT_PIECE);

        if (piece == EMPTY) {
                printf("  \033[0m");
                return;
        }

        printf("%s\033[0m", piece_strings[PIECE_OF(piece)]);
}

void print_board(p_board board)
{
        for (int j = 0; j < 64; j++)
        {
                const int i = board->player == WHITE ? j : 63 - j;

                if (j % 8 == 0)
                        printf("%d ", 8 - RANK_OF(i));

                const p_index index = FILE_OF(i) + (7 - RANK_OF(i)) * 8;
                p_piece piece = board->mailbox[index];
                p_color square_color = (FILE_OF(i) + RANK_OF(i)) % 2;
                draw_square(piece, square_color);

                if (j % 8 == 7)
                        printf("\n");
        }

        printf(board->player == WHITE ? "WM a b c d e f g h\n" : "BM h g f e d c b a\n");

}

void print_bitboard(p_bitboard board)
{
        for (int i = 0; i < 8; i++)
        {
                const p_bitboard mask = 255ULL << (i * 8);

                const uint8_t row = (board & mask) >> (i * 8);

                printf("%08b\n", row);
        }
}

