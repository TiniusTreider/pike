#define _POSIX_C_SOURCE 200809L

#include "uci.h"
#include "error.h"
#include "macros.h"
#include "def.h"
#include "board.h"
#include "engine.h"
#include "print.h"
#include "debug.h"

#include <stdio.h>
#include <string.h>

bool should_continue = true;

#define READ_ERROR "failed to read input from stdin"
#define DELIM " \t\r\n"

// engine to gui

static char *strtok_ptr = NULL;

#define SEND(MESSAGE) do { printf(MESSAGE "\n"); fflush(stdout); } while (false)

#define AUTHOR "Tinius Treider"

void id_c(void)
{
        SEND("id name " VERSION);
        SEND("id author " AUTHOR);
}

void option_c(void)
{
        return;
}

void uciok_c(void)
{
        SEND("uciok");
}

// gui to engine

void uci_c(void)
{
        id_c();
        option_c();
        uciok_c();
}

void debug_c(void)
{
        // TODO
}

void isready_c(void)
{
        SEND("readyok");
}

void setoption_c(void)
{
        return;
}

void register_c(void)
{
        return;
}

void ucinewgame_c(void)
{
        return;
}

void position_c(void)
{
        char *token = strtok_r(NULL, DELIM, &strtok_ptr);
        if (!token)
                return;

        pthread_mutex_lock(&pike->lock);

        if (strcmp(token, "startpos") == 0) {
                pike->board = init_board(STARTPOS_FEN);
                token = strtok_r(NULL, DELIM, &strtok_ptr);
        } else if (strcmp(token, "kiwipete") == 0) {
                pike->board = init_board(KIWIPETE_FEN);
                token = strtok_r(NULL, DELIM, &strtok_ptr);
        } else if (strcmp(token, "fen") == 0) {
                token = strtok_r(NULL, DELIM, &strtok_ptr);

                char fen[128] = "";

                for (int i = 0; i < 6; i++)
                {
                        if (i != 0)
                                strcat(fen, " ");
                        strcat(fen, token);

                        token = strtok_r(NULL, DELIM, &strtok_ptr);
                        if (!token)
                                return;
                }

                LOG(fen);

                pike->board = init_board(fen);
        }

        if (!token || *token == '\0')
                return;

        LOG("parsing move list");

        if (strcmp(token, "moves") != 0) {
                LOG("unrecognized argument");
                return;
        }

        token = strtok_r(NULL, DELIM, &strtok_ptr);
        while (token)
        {
                const p_move move = parse_move(pike->board, token);
                if (IS_NULL_MOVE(move)) {
                        LOG("invalid move");
                        return;
                }
                (void)make_move(pike->board, move);

                token = strtok_r(NULL, DELIM, &strtok_ptr);
        }

        pthread_mutex_unlock(&pike->lock);

        LOG("finished parsing move list");
}

void go_c(void)
{
        // TODO
}

void stop_c(void)
{
        // TODO
}

void ponderhit_c(void)
{
        // TODO
}

void quit_c(void)
{
        should_continue = false;
}

void d_c(void)
{
        pthread_mutex_lock(&pike->lock);

        print_board(pike->board);

        pthread_mutex_unlock(&pike->lock);
}

struct pair { char *string; void (*function)(void); };

#define COMMANDS \
        X(uci) \
        X(debug) \
        X(isready) \
        X(setoption) \
        X(register) \
        X(ucinewgame) \
        X(position) \
        X(go) \
        X(stop) \
        X(ponderhit) \
        X(quit) \
        X(d)

#define X(COMMAND) { #COMMAND, COMMAND##_c },

static const struct pair functions[] = {
        COMMANDS
};

#undef X

void uci(void)
{
        LOG("parsing uci");

        char string[16384];
        while (should_continue)
        {
                errorif(fgets(string, sizeof(string), stdin) == NULL, READ_ERROR);

                LOG("parsing a line");

                char *token = strtok_r(string, DELIM, &strtok_ptr);
                while (token)
                {
                        for (size_t i = 0; i < ELEMENTS_OF(functions); i++)
                        {
                                if (strcmp(token, functions[i].string) == 0) {
                                        functions[i].function();
                                        goto end;
                                }
                        }

                        token = strtok_r(NULL, DELIM, &strtok_ptr);
                }

end:

                LOG("finished parsing line");
        }

        LOG("finished parsing uci");
}

