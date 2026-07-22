#include "print.h"
#include "macros.h"
#include "structboard.h"

#include <stdio.h>

static constexpr char PIECE_CHARS[6] = { 'p', 'b', 'n', 'r', 'k', 'q' };

void print_move(p_move move)
{
        printf(
                "%c%d%c%d",
                'a' + FILE_OF(move.from),
                1 + RANK_OF(move.from),
                'a' + FILE_OF(move.to),
                1 + RANK_OF(move.to)
        );

        if (move.flags & MOVE_PROMOTION)
        {
                printf("%c", PIECE_CHARS[PIECE_OF(move.promotion)]);
        }
}

static const char *piece_strings[6] = {
        "♟ ", "♞ ", "♝ ", "♜ ", "♛ ", "♚ "
};

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

void print_board(struct board *board)
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

        printf(board->player == WHITE ? "[W]a b c d e f g h" : "[B]h g f e d c b a");

        printf("\n\n");
}

