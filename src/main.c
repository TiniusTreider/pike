// #include "uci.h"
#include "perft.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
        if (argc != 3 && argc != 4)
                return 1;

        // uci();
        perft(atoi(argv[1]), argv[2], argc == 4 ? argv[3] : NULL);

        return 0;
}

