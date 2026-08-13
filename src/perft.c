#include "perft.h"
#include "board.h"
#include "movegen.h"
#include "print.h"
#include "engine.h"
#include "wrappers.h"

#include <stdio.h>
#include <string.h>

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
        size_t move_count = generate_capture_moves(pike->data.board, buffer);
        move_count += generate_quiet_moves(pike->data.board, buffer + move_count);

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

void perft(size_t depth)
{
        if (depth == 0)
                return;

        p_move buffer[218];
        size_t move_count = generate_capture_moves(pike->data.board, buffer);
        move_count += generate_quiet_moves(pike->data.board, buffer + move_count);
        size_t sum = 0;

        const size_t before = now_ms();

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

                char move_string[6];
                print_move(buffer[i], move_string);
                printf("%s: %zu\n", move_string, nodes);

                sum += nodes;
        }

        const size_t after = now_ms();

        const size_t time = after - before;
        const size_t nps = sum / (time + 1) * 1000;

        printf(
                "\n"
                "Nodes searched: %zu\n"
                "\n"
                "info nodes %zu depth %zu time %zu nps %zu\n",
                sum, sum, depth, time, nps
        );
}

