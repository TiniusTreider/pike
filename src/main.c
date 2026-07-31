#include "uci.h"
#include "engine.h"
#include "perft.h"
#include "def.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PERFT_OPTION "perft"
#define USAGE \
        VERSION " usage:\n" \
        "\n" \
        "    pike\n" \
        "        run UCI mode\n" \
        "\n" \
        "    pike " PERFT_OPTION " [<FEN> | startpos | kiwipete] <depth>\n" \
        "        run a benchmark with the given position and depth\n" \
        "\n"

int main(int argc, char **argv)
{
        if (argc == 1) {
                p_engine pike = init_engine();

                uci();

                clean_engine(pike);
        } else if (argc == 4) {
                if (strcmp(argv[1], PERFT_OPTION) == 0)
                        perft(argv[2], (size_t)strtoull(argv[3], NULL, 10));
        } else {
                printf(USAGE);
                return 1;
        }

        return 0;
}

