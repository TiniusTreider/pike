#include "zobrist.h"

#include <stdint.h>

#define SEED 18012011

static uint64_t state;
static inline uint64_t random_uint64()
{
    uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

uint64_t piece_hash[64][13];
uint64_t black_hash;
uint64_t castling_hash[16];
uint64_t ep_hash[8];

void init_zobrist(void)
{
        state = SEED;

        for (int i = 0; i < 64; i++)
        {
                for (int j = 0; j < 12; j++)
                {
                        piece_hash[i][j] = random_uint64();
                }

                piece_hash[i][12] = 0ULL;
        }

        black_hash = random_uint64();

        for (int i = 0; i < 16; i++)
        {
                castling_hash[i] = random_uint64();
        }

        for (int i = 0; i < 8; i++)
        {
                ep_hash[i] = random_uint64();
        }
}

