#define _POSIX_C_SOURCE 200809L

#include "perft.h"
#include "board.h"
#include "movegen.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define BULK_COUNT 0

size_t count(p_board board, size_t depth)
{
        p_move buffer[218];
        const size_t move_count = generate_moves(board, buffer);

        if (depth == 0)
#if BULK_COUNT
                return move_count;
#else
                return 1;
#endif

        size_t sum = 0;
        for (size_t i = 0; i < move_count; i++)
        {
                const p_unmake data = make_move(board, buffer[i]);

                sum += count(board, depth - 1);

                unmake_move(board, buffer[i], data);
        }

        return sum;
}

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void perft(char *fen, size_t depth)
{
        if (depth == 0) {
                printf("perft depth cannot be 0\n");
                return;
        }

        p_board board;

        if (strcmp(fen, "startpos") == 0) {
                board = init_board(STARTPOS_FEN);
        } else if (strcmp(fen, "kiwipete") == 0) {
                board = init_board(KIWIPETE_FEN);
        } else {
                board = init_board(fen);
        }

        for (size_t i = 0; i < depth; i++)
        {
                const double before = now();

#if BULK_COUNT
                const size_t nodes = count(board, i);
#else
                const size_t nodes = count(board, i + 1);
#endif

                const double after = now();

                const double time = after - before;
                const double mnps = (nodes / 1e6) / time;

                printf("nodes at depth %zu: %zu (%.3lfMnps, %.6lfs)\n", i + 1, nodes, mnps, time);
        }

        clean_board(board);
}

