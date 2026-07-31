#include "uci.h"
#include "error.h"
#include "macros.h"
#include "def.h"

#include <stdio.h>
#include <string.h>

bool should_continue = true;

#define READ_ERROR "failed to read input from stdin"
#define DELIM " \t\r\n"

// engine to gui

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
        char *token = strtok(NULL, DELIM);
        if (!token)
                return;

        if (strcmp(token, "startpos") == 0) {
                // set global board to STARTPOS_FEN
                strtok(NULL, DELIM);
        } else if (strcmp(token, "kiwipete") == 0) {
                // set global board to KIWIPETE_FEN
                strtok(NULL, DELIM);
        } else if (strcmp(token, "fen") == 0) {
                token = strtok(NULL, DELIM);
                if (!token)
                        return;

                // set global board to FEN

                for (int i = 0; i < 6; i++)
                {
                        if (!(token = strtok(NULL, DELIM)))
                                return;
                }
        }

        if (strcmp(token, "moves") != 0)
                return;

        token = strtok(NULL, DELIM);
        while (token)
        {
                // apply token as move

                token = strtok(NULL, DELIM);
        }
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

struct pair { char *string; void (*function)(void); };
static const struct pair functions[] = {
        { "uci", uci_c },
        { "debug", debug_c },
        { "isready", isready_c },
        { "setoption", setoption_c },
        { "register", register_c },
        { "ucinewgame", ucinewgame_c },
        { "position", position_c },
        { "go", go_c },
        { "stop", stop_c },
        { "ponderhit", ponderhit_c },
        { "quit", quit_c }
};

void uci(void)
{
        char string[16384];
        while (should_continue)
        {
                errorif(fgets(string, sizeof(string), stdin) == NULL, READ_ERROR);

                char *token = strtok(string, DELIM);
                while (token)
                {
                        for (size_t i = 0; i < ELEMENTS_OF(functions); i++)
                        {
                                if (strcmp(token, functions[i].string) == 0) {
                                        functions[i].function();
                                        goto end;
                                }
                        }

                        token = strtok(NULL, DELIM);
                }

end:
        }
}

