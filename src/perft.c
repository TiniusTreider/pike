#include "perft.h"
#include "movegen.h"
#include "structboard.h"
#include "error.h"
#include "print.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static inline size_t count(p_board board, size_t depth)
{
        if (depth == 0)
                return 1;

        p_move buffer[218];
        const size_t size = generate_moves(board, buffer);

        size_t sum = 0;
        for (size_t i = 0; i < size; i++)
        {
                const p_unmake data = make_move(board, buffer[i]);

                sum += count(board, depth - 1);

                unmake_move(board, buffer[i], data);
        }

        return sum;
}

static inline p_index parse_square(const char *s)
{
    int file = s[0] - 'a';
    int rank = s[1] - '1';

    return rank * 8 + file;
}

static int parse_uci_move(p_board board, const char *uci, p_move *result)
{
    p_move buffer[218];
    size_t count = generate_moves(board, buffer);

    p_index from = parse_square(uci);
    p_index to   = parse_square(uci + 2);

    int promotion = 0;

    if (uci[4])
    {
        switch (uci[4])
        {
            case 'n': promotion = KNIGHT; break;
            case 'b': promotion = BISHOP; break;
            case 'r': promotion = ROOK;   break;
            case 'q': promotion = QUEEN;  break;
        }
    }

    for (size_t i = 0; i < count; i++)
    {
        if (buffer[i].from != from)
            continue;

        if (buffer[i].to != to)
            continue;

        if (promotion && buffer[i].promotion != promotion)
            continue;

        if (!promotion && (buffer[i].flags & MOVE_PROMOTION))
            continue;

        *result = buffer[i];
        return 1;
    }

    return 0;
}

void parse_move_list(p_board board, char *move_list)
{
    char *token = strtok(move_list, " ");

    while (token)
    {
        p_move move;

        if (!parse_uci_move(board, token, &move))
        {
            printf("Invalid move: %s\n", token);
            return;
        }

        (void)make_move(board, move);

        token = strtok(NULL, " ");
    }
}

struct bitboards {
        p_bitboard *bitboards;
        p_bitboard all_pieces;
        p_bitboard white_pieces;
        p_bitboard black_pieces;
};

void perft(size_t depth, char *fen, char *move_list)
{
        p_board board = init_board(fen);

        if (move_list != NULL)
                parse_move_list(board, move_list);

        p_move buffer[218];
        const size_t move_count = generate_moves(board, buffer);
        size_t sum = 0;
        for (size_t i = 0; i < move_count; i++)
        {
                print_move(buffer[i]);

                const struct bitboards test = (struct bitboards){
                        .bitboards = board->bitboards,
                        .all_pieces = board->all_pieces,
                        .white_pieces = board->white_pieces,
                        .black_pieces = board->black_pieces
                };

                const p_unmake data = make_move(board, buffer[i]);
                const size_t size = count(board, depth - 1);
                unmake_move(board, buffer[i], data);

                if (memcmp(test.bitboards, board->bitboards, 12 * sizeof(p_bitboard)) != 0)
                        error("\nbitboards dont match after unmake");
                if (board->all_pieces != test.all_pieces)
                        error("\nall_pieces doesnt match after unmake");
                if (board->white_pieces != test.white_pieces)
                        error("\nwhite_pieces doesnt match after unmake");
                if (board->black_pieces != test.black_pieces)
                        error("\nblack_pieces doesnt match after unmake");

                printf(" %zu\n", size);
                sum += size;
        }

        printf("\n%zu\n", sum);

        clean_board(board);
}

