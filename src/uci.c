#define _POSIX_C_SOURCE 200809L

#include "uci.h"
#include "error.h"
#include "macros.h"
#include "def.h"
#include "board.h"
#include "engine.h"
#include "print.h"
#include "debug.h"
#include "wrappers.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
        const char *token = strtok_r(NULL, DELIM, &strtok_ptr);

        lock_mutex(&pike->lock);

        if (strcmp(token, "on") == 0)
                pike->data.debug = true;
        else if (strcmp(token, "off") == 0)
                pike->data.debug = false;

        unlock_mutex(&pike->lock);
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

        lock_mutex(&pike->lock);

        if (strcmp(token, "startpos") == 0) {
                set_board(pike->data.board, STARTPOS_FEN);
                token = strtok_r(NULL, DELIM, &strtok_ptr);
        } else if (strcmp(token, "kiwipete") == 0) {
                set_board(pike->data.board, KIWIPETE_FEN);
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
                        if (!token && i != 5) {
                                LOG("incomplete fen");
                                goto position_cleanup;
                        }
                }

                LOG(fen);

                set_board(pike->data.board, fen);
        }

        if (!token || *token == '\0')
                goto position_cleanup;

        LOG("parsing move list");

        if (strcmp(token, "moves") != 0) {
                LOG("unrecognized argument");
                goto position_cleanup;
        }

        token = strtok_r(NULL, DELIM, &strtok_ptr);
        while (token)
        {
                const p_move move = parse_move(pike->data.board, token);
                if (IS_NULL_MOVE(move)) {
                        LOG("invalid move");
                        goto position_cleanup;
                }
                (void)make_move(pike->data.board, move);

                token = strtok_r(NULL, DELIM, &strtok_ptr);
        }

        LOG("finished parsing move list");

position_cleanup:

        unlock_mutex(&pike->lock);
}

struct pair { char *string; void (*function)(void); };

#define GO_COMMANDS \
        X(searchmoves) \
        X(ponder) \
        X(wtime) \
        X(btime) \
        X(winc) \
        X(binc) \
        X(movestogo) \
        X(depth) \
        X(nodes) \
        X(movetime) \
        X(infinite) \
        X(perft) \
        X(mate)

#define X(COMMAND) void COMMAND##_c(void);
GO_COMMANDS
#undef X

#define X(COMMAND) { #COMMAND, COMMAND##_c },

static const struct pair go_functions[] = {
        GO_COMMANDS
};

#undef X

#define DO_GO_COMMAND do {\
        for (size_t i = 0; i < ELEMENTS_OF(go_functions); i++) \
        { \
                if (strcmp(token, go_functions[i].string) == 0) { \
                        LOG("recognized go subcommand"); \
 \
                        go_functions[i].function(); \
                        break; \
                } \
        } \
} while (false)

void searchmoves_c(void)
{
        char *token = strtok_r(NULL, DELIM, &strtok_ptr);
        while (token)
        {
                const p_move move = parse_move(pike->data.board, token);
                if (IS_NULL_MOVE(move)) {
                        DO_GO_COMMAND;
                }
                pike->data.searchmoves[pike->data.searchmoves_size++] = move;

                token = strtok_r(NULL, DELIM, &strtok_ptr);
        }
}
void ponder_c(void)
{
        pike->data.ponder = true;
}

#define ADVANCE_TOKEN \
        char *token = strtok_r(NULL, DELIM, &strtok_ptr); \
        if (!token) \
                return;

void wtime_c(void)
{
        ADVANCE_TOKEN
        pike->data.wtime = strtoull(token, NULL, 10);
}
void btime_c(void)
{
        ADVANCE_TOKEN
        pike->data.btime = strtoull(token, NULL, 10);

}
void winc_c(void)
{
        char *token = strtok_r(NULL, DELIM, &strtok_ptr);
        pike->data.winc = strtoull(token, NULL, 10);
}
void binc_c(void)
{
        ADVANCE_TOKEN
        pike->data.binc = strtoull(token, NULL, 10);
}
void movestogo_c(void)
{
        ADVANCE_TOKEN
        pike->data.movestogo = strtoull(token, NULL, 10);
}
void depth_c(void)
{
        ADVANCE_TOKEN
        pike->data.depth = strtoull(token, NULL, 10);
}
void nodes_c(void)
{
        ADVANCE_TOKEN
        pike->data.nodes = strtoull(token, NULL, 10);
}
void movetime_c(void)
{
        ADVANCE_TOKEN
        pike->data.movetime = strtoull(token, NULL, 10);
}
void infinite_c(void)
{
        pike->data.infinite = true;
}
void perft_c(void)
{
        ADVANCE_TOKEN
        pike->data.perft_depth = strtoull(token, NULL, 10);
        pike->data.perft = true;
}
void mate_c(void)
{
        ADVANCE_TOKEN
        pike->data.depth = strtoull(token, NULL, 10) * 2 - 1;
}

void stop_c(void);

void go_c(void)
{
        pike->stop = false;

        lock_mutex(&pike->lock);

        pike->data.searchmoves_size = 0;
        pike->data.ponder = false;
        pike->data.wtime = 600000;
        pike->data.btime = 600000;
        pike->data.winc = 600000;
        pike->data.binc = 600000;
        pike->data.movestogo = 0;
        pike->data.depth = 0;
        pike->data.nodes = 0;
        pike->data.movetime = 0;
        pike->data.infinite = false;
        pike->data.perft = false;

        char *token = strtok_r(NULL, DELIM, &strtok_ptr);
        while (token)
        {
                DO_GO_COMMAND;

                token = strtok_r(NULL, DELIM, &strtok_ptr);
        }

        unlock_mutex(&pike->lock);

        post_sem(&pike->task);
}

void stop_c(void)
{
        pike->stop = true;
}

void ponderhit_c(void)
{
        // TODO
}

void quit_c(void)
{
        pike->stop = true;
        pike->kill = true;
        post_sem(&pike->task);
        should_continue = false;
}

void d_c(void)
{
        lock_mutex(&pike->lock);

        print_board(pike->data.board);

        unlock_mutex(&pike->lock);
}

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

        char string[16384] = "";
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
                                        LOG("recognized command");
                                        functions[i].function();

                                        goto end;
                                }
                        }

                        token = strtok_r(NULL, DELIM, &strtok_ptr);
                }

                LOG("did not recognize command");

end:

                LOG("finished parsing line");
        }

        LOG("finished parsing uci");
}

