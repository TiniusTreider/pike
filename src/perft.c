#define _POSIX_C_SOURCE 200809L

#include "perft.h"
#include "board.h"
#include "movegen.h"
#include "print.h"
#include "engine.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define BULK_COUNT 1

#define STOP_CHECK_FREQ 0xFFFF

static size_t node_count = 0;
static bool stop_flag = 0;

size_t count(size_t depth)
{
        if (stop_flag)
                return 0;

        if (!(node_count++ & STOP_CHECK_FREQ) && pike->stop) {
                stop_flag = 1;
                return 0;
        }

        p_move buffer[218];
        const size_t move_count = generate_moves(pike->data.board, buffer);

        if (depth == 0)
#if BULK_COUNT
                return move_count;
#else
                return 1;
#endif

        size_t sum = 0;
        for (size_t i = 0; i < move_count; i++)
        {
                const p_unmake data = make_move(pike->data.board, buffer[i]);

                sum += count(depth - 1);

                unmake_move(pike->data.board, buffer[i], data);

                if (stop_flag)
                        return 0;
        }

        return sum;
}

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void perft(size_t depth)
{
        if (depth == 0)
                return;

        p_move buffer[218];
        const size_t move_count = generate_moves(pike->data.board, buffer);
        size_t sum = 0;

        const double before = now();

        for (size_t i = 0; i < move_count; i++)
        {
                const p_unmake data = make_move(pike->data.board, buffer[i]);

                size_t nodes;

                if (depth == 1) {
                        nodes = 1;
                } else {
#if BULK_COUNT
                        nodes = count(depth - 2);
#else
                        nodes = count(depth - 1);
#endif
                        node_count = 0;
                }

                unmake_move(pike->data.board, buffer[i], data);

                if (stop_flag) {
                        stop_flag = 0;
                        return;
                }

                print_move(buffer[i]);
                printf(": %zu\n", nodes);

                sum += nodes;
        }

        const double after = now();

        const double time = after - before;
        const double nps = sum / time;

        printf(
                "\n"
                "Nodes searched: %zu\n"
                "\n"
                "info nodes %zu depth %zu time %zu nps %zu\n",
                sum, sum, depth, (size_t)(time * 1000), (size_t)nps
        );
}

