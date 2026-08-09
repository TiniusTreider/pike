#include "eval.h"
#include "structboard.h"
#include "macros.h"

#include <stddef.h>

static constexpr p_eval PIECE_VALUES[6] = { 100, 315, 337, 512, 975, 0 };

p_eval evaluation(p_board board)
{
        p_eval white = 0;
        p_eval black = 0;
        for (p_color color = 0; color < 2; color++)
        {
                for (p_piece_type piece = 0; piece < 6; piece++)
                {
                        p_eval *side = color ? &black : &white;
                        const size_t population = POP(board->bitboards[PIECE_WITH(piece, color)]);
                        const p_eval value = PIECE_VALUES[piece];
                        *side += value * population;
                }
        }

        return board->player ? black - white : white - black;
}

