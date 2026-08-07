#include "search.h"
#include "perft.h"
#include "engine.h"
#include "wrappers.h"
#include "debug.h"
#include "movegen.h"
#include "print.h"

#include <stddef.h>
#include <stdio.h>

static inline p_eval negamax(size_t depth)
{
        return 0;
}

static inline void search(void)
{
        p_move chosen_move = NULL_MOVE;

        p_move buffer[218];
        const size_t move_count = generate_moves(pike->data.board, buffer);
        for (size_t depth = 0; !pike->stop; depth++)
        {
                p_eval max_eval;
                p_move best_move = NULL_MOVE;
                for (size_t move = 0; move < move_count; move++)
                {
                        const p_unmake data = make_move(pike->data.board, buffer[move]);

                        const p_eval eval = negamax(depth);
                        if (eval > max_eval) {
                                max_eval = eval;
                                best_move = buffer[move];
                        }

                        unmake_move(pike->data.board, buffer[move], data);
                }

                if (!pike->stop)
                        chosen_move = best_move;
        }

        char move_string[6];
        print_move(chosen_move, move_string);
        printf("bestmove %s\n", move_string);
}

static inline void search_mate(void)
{
        // TODO search for mate in pike->mate
}

static inline void search_parse(void)
{
        if (pike->data.perft) {
                perft(pike->data.perft_depth);
                return;
        }

        if (pike->data.mate) {
                search_mate();
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

