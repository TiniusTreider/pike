#include "search.h"
#include "perft.h"
#include "engine.h"
#include "wrappers.h"
#include "debug.h"
#include "movegen.h"
#include "print.h"
#include "structboard.h"
#include "macros.h"
#include "movegen.h"
#include "def.h"
#include "eval.h"

#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <threads.h>

static size_t nodes_searched = 0;
#define NODE_LIMIT_FREQ 0xFFFF

#define MATE_SCORE 15000
#define MATE_THRESHOLD 14000

#define SEARCH_MAX 64
static p_move pv_matrix[SEARCH_MAX][SEARCH_MAX] = {0};

static inline p_eval negamax(size_t depth, size_t ply, size_t *pv_length)
{
        nodes_searched++;

        if (!(nodes_searched & NODE_LIMIT_FREQ)) {
                if (pike->data.nodes && nodes_searched > pike->data.nodes) {
                        pike->stop = true;
                        return 0;
                } if (pike->data.deadline && now_ms() > pike->data.deadline) {
                        pike->stop = true;
                        return 0;
                }
        }

        if (depth == 0)
                return evaluation(pike->data.board);

        p_move buffer[218];
        const size_t move_count = generate_moves(pike->data.board, buffer);

        if (move_count == 0) {
                const p_piece king = PIECE_WITH(KING, pike->data.board->player);
                const p_index king_pos = CTZ(pike->data.board->bitboards[king]);
                if (is_square_attacked(
                                pike->data.board,
                                king_pos
                )) {
                        return -MATE_SCORE;
                } else {
                        return 0;
                }
        }

        p_eval max_eval = EVAL_MIN;
        p_move best_move = NULL_MOVE;
        for (size_t i = 0; i < move_count; i++)
        {
                const p_unmake data = make_move(pike->data.board, buffer[i]);

                size_t copy_size = 0;
                const p_eval eval = -negamax(depth - 1, ply + 1, &copy_size);
                if (eval > max_eval) {
                        max_eval = eval;
                        best_move = buffer[i];

                        if (copy_size)
                                memcpy(
                                        pv_matrix[ply] + 1,
                                        pv_matrix[ply + 1],
                                        copy_size * sizeof(p_move)
                                );

                        *pv_length = copy_size + 1;
                }

                unmake_move(pike->data.board, buffer[i], data);

                if (pike->stop)
                        return 0;
        }

        pv_matrix[ply][0] = best_move;

        if (abs(max_eval) > MATE_THRESHOLD)
                return max_eval > 0 ? max_eval - 1 : max_eval + 1;

        return max_eval;
}

static inline size_t get_search_time(void)
{
        if (pike->data.infinite)
                return 0;

        const double time = pike->data.board->player ? pike->data.btime : pike->data.wtime;
        const double inc = pike->data.board->player ? pike->data.binc : pike->data.winc;

        const double budget = MIN(0.9 * time, 0.05 * time + 0.8 * inc);

        if (pike->data.movetime)
                return MIN(budget, pike->data.movetime);

        return budget;
}

static inline void search(void)
{
        pike->data.deadline = now_ms() + get_search_time();

        p_eval chosen_eval = 0;
        p_move chosen_move = NULL_MOVE;

        p_move buffer[218];
        const size_t move_count = generate_moves(pike->data.board, buffer);

        for (
                size_t depth = 1;
                !pike->stop && (!pike->data.depth || (depth <= pike->data.depth));
                depth++
        ) {
                nodes_searched = 0;

                size_t pv_length = 0;

                p_eval max_eval = EVAL_MIN;
                p_move best_move = NULL_MOVE;
                for (size_t move = 0; move < move_count; move++)
                {
                        const p_unmake data = make_move(pike->data.board, buffer[move]);

                        size_t copy_size = 0;
                        const p_eval eval = -negamax(depth - 1, 1, &copy_size);

                        if (eval > max_eval && !pike->stop) {
                                max_eval = eval;
                                best_move = buffer[move];

                                if (copy_size)
                                        memcpy(
                                                pv_matrix[0] + 1,
                                                pv_matrix[1],
                                                copy_size * sizeof(p_move)
                                        );

                                pv_length = copy_size + 1;
                        }

                        unmake_move(pike->data.board, buffer[move], data);

                        if (pike->stop)
                                break;
                }

                if (pike->stop)
                        break;

                chosen_eval = max_eval;
                chosen_move = best_move;
                pv_matrix[0][0] = best_move;

                char pv_string[SEARCH_MAX * 6] = "";
                char pv_move[6];
                for (size_t i = 0; i < pv_length; i++)
                {
                        print_move(pv_matrix[0][i], pv_move);
                        strcat(pv_string, pv_move);
                        if (i != pv_length - 1)
                                strcat(pv_string, " ");
                }
                printf(
                        "info depth %zu score cp %d nodes %zu pv %s\n",
                        pv_length, chosen_eval, nodes_searched, pv_string
                );
                fflush(stdout);
        }

        char move_string[6];
        print_move(chosen_move, move_string);
        printf("bestmove %s\n", move_string);
        fflush(stdout);
}

static inline void search_parse(void)
{
        if (pike->data.perft) {
                perft(pike->data.perft_depth);
                return;
        }

        search();
}

void *search_wait(void*)
{
        for (;;)
        {
                wait_sem(&pike->task);

                if (pike->kill)
                        break;

                lock_mutex(&pike->lock);

                LOG("search started");

                search_parse();

                LOG("search ended");

                unlock_mutex(&pike->lock);
        }

        return NULL;
}

