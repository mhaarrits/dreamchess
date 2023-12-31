/*  DreamChess
**  Copyright (C) 2003-2005  The DreamChess project
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#include <errno.h>

#include "board.h"
#include "history.h"
#include "ui.h"
#include "comm.h"
#include "dir.h"
#include "dreamchess.h"
#include "debug.h"
#include "svn_version.h"
#include "libmenu.h"

#ifdef HAVE_GETOPT_LONG
#define OPTION_TEXT(L, S, T) "  " L "\t" S "\t" T "\n"
#else
#define OPTION_TEXT(L, S, T) "  " S "\t" T "\n"
#endif

/* FIXME */
int pgn_parse_file(char *filename);

typedef struct move_list
{
    char **move;
    int entries, view, max_entries;
}
move_list_t;

static ui_driver_t *ui;
static config_t *config;
static move_list_t san_list, fan_list, fullalg_list;
static history_t *history;
static int in_game;

static void move_list_play(move_list_t *list, char *move)
{
    if (list->entries == list->max_entries)
    {
        list->max_entries *= 2;
        list->move = realloc(list->move, list->max_entries * sizeof(char *));
    }
    list->move[list->entries++] = strdup(move);
    list->view = list->entries - 1;
}

static void move_list_undo(move_list_t *list)
{
    if (list->entries > 0)
    {
        list->entries--;
        free(list->move[list->entries]);
        list->view = list->entries - 1;
    }
}

static void move_list_init(move_list_t *list)
{
    list->max_entries = 20;
    list->move = malloc(list->max_entries * sizeof(char *));
    list->entries = 0;
    list->view = -1;
}

static void move_list_exit(move_list_t *list)
{
    while (list->entries > 0)
        move_list_undo(list);
    free(list->move);
}

static void move_list_view_next(move_list_t *list)
{
    if (list->view < list->entries - 1)
        list->view++;
}

static void move_list_view_prev(move_list_t *list)
{
    if (list->view >= 0)
        list->view--;
}

void game_view_next()
{
    history_view_next(history);
    move_list_view_next(&fullalg_list);
    move_list_view_next(&san_list);
    move_list_view_next(&fan_list);
    ui->update(history->view->board, NULL);
}

void game_view_prev()
{
    history_view_prev(history);
    move_list_view_prev(&fullalg_list);
    move_list_view_prev(&san_list);
    move_list_view_prev(&fan_list);
    ui->update(history->view->board, NULL);
}

void game_undo()
{
    history_undo(history);
    move_list_undo(&fullalg_list);
    move_list_undo(&san_list);
    move_list_undo(&fan_list);
    if (history->result)
    {
        free(history->result->reason);
        free(history->result);
        history->result = NULL;
    }
    ui->update(history->view->board, NULL);
}

void game_retract_move()
{
    /* Make sure a user is on move and we can undo two moves. */
    if (config->player[history->last->board->turn] != PLAYER_UI)
        return;
    if (!history->last->prev || !history->last->prev->prev)
        return;

    game_undo();
    game_undo();
    comm_send("remove\n");
}

void game_move_now()
{
    /* Make sure engine is on move. */
    if (config->player[history->last->board->turn] != PLAYER_ENGINE)
        return;

    comm_send("?\n");
}

int game_want_move()
{
    return config->player[history->last->board->turn] == PLAYER_UI
           && history->last == history->view;
}

int game_save( int slot )
{
    int retval;
    char temp[80];

    if (!ch_userdir())
    {
        sprintf( temp, "save%i.pgn", slot );
        retval = history_save_pgn(history, temp);
    }
    else
    {
        DBG_ERROR("failed to enter user directory");
        retval = 1;
    }

    return retval;
}

static int do_move(move_t *move, int ui_update)
{
    char *move_s, *move_f, *move_san;
    board_t new_board;

    if (!move_is_valid(history->last->board, move))
    {
        DBG_WARN("move is illegal");
        return 0;
    }

    move_set_attr(history->last->board, move);
    new_board = *history->last->board;
    move_s = move_to_fullalg(&new_board, move);
    move_list_play(&fullalg_list, move_s);

    move_san = move_to_san(&new_board, move);
    move_f = san_to_fan(&new_board, move_san);

    DBG_LOG("processing move %s (%s)", move_s, move_san);

    move_list_play(&san_list, move_san);
    move_list_play(&fan_list, move_f);

    free(move_san);
    free(move_f);
    free(move_s);

    make_move(&new_board, move);

    if (move->state == MOVE_CHECK)
        new_board.state = BOARD_CHECK;
    else if (move->state == MOVE_CHECKMATE)
        new_board.state = BOARD_CHECKMATE;
    else
        new_board.state = BOARD_NORMAL;

    history_play(history, move, &new_board);

    if (ui_update)
        ui->update(history->view->board, move);

    if (new_board.state == MOVE_CHECKMATE)
    {
        history->result = malloc(sizeof(result_t));

        if (new_board.turn == WHITE)
        {
            history->result->code = RESULT_BLACK_WINS;
            history->result->reason = strdup("Black mates");
        }
        else
        {
            history->result->code = RESULT_WHITE_WINS;
            history->result->reason = strdup("White mates");
        }

        if (ui_update)
            ui->show_result(history->result);
    }
    else if (new_board.state == MOVE_STALEMATE)
    {
        history->result = malloc(sizeof(result_t));

        history->result->code = RESULT_DRAW;
        history->result->reason = strdup("Stalemate");

        if (ui_update)
            ui->show_result(history->result);
    }

    return 1;
}

void game_make_move(move_t *move, int ui_update)
{
    if (do_move(move, ui_update)){
        comm_send("%s\n", fullalg_list.move[fullalg_list.entries-1]);
    }
    else
    {
        char *move_str = move_to_fullalg(history->last->board, move);
        DBG_WARN("ignoring illegal move %s", move_str);
        free(move_str);
    }
}

void game_quit()
{
    in_game = 0;
}

int game_load( int slot )
{
    int retval;
    char temp[80];
    board_t *board;

    if (ch_userdir())
    {
        DBG_ERROR("failed to enter user directory");
        return 1;
    }

    comm_send("force\n");

    sprintf( temp, "save%i.pgn", slot );
    retval = pgn_parse_file(temp);

    if (retval)
    {
        DBG_ERROR("failed to parse PGN file '%s'", temp);
        return 1;
    }

    board = history->last->board;

    ui->update(board, NULL);

    if (config->player[board->turn] == PLAYER_ENGINE)
        comm_send("go\n");
    else if (config->player[OPPONENT(board->turn)] == PLAYER_ENGINE)
    {
        if (board->turn == WHITE)
            comm_send("white\n");
        else
            comm_send("black\n");
    }

    return retval;
}

void game_make_move_str(char *move_str, int ui_update)
{
    board_t new_board = *history->last->board;
    move_t *engine_move;

    DBG_LOG("parsing move string '%s'", move_str);

    engine_move = san_to_move(&new_board, move_str);

    if (!engine_move)
        engine_move = fullalg_to_move(&new_board, move_str);
    if (engine_move)
    {
        game_make_move(engine_move, ui_update);
        free(engine_move);
    }
    else
        DBG_ERROR("failed to parse move string '%s'", move_str);
}

void game_get_move_list(char ***list, int *total, int *view)
{
    *list = fan_list.move;
    *total = fan_list.entries;
    *view = fan_list.view;
}

int use_ui_fullscreen=0;

#ifdef __BEOS__
int use_ui_width=480;
int use_ui_height=360;
#else
int use_ui_width=640;
int use_ui_height=480;
#endif

#ifndef _arch_dreamcast
static void parse_options(int argc, char **argv, ui_driver_t **ui_driver, char **engine)
{
    int c;

#ifdef HAVE_GETOPT_LONG

    int optindex;

    struct option options[] =
        {
            {"help", no_argument, NULL, 'h'
            },
            {"list-drivers", no_argument, NULL, 'l'},
            {"ui", required_argument, NULL, 'u'},
            {"fullscreen", no_argument, NULL, 'f'},
            {"width", required_argument, NULL, 'W'},
            {"height", required_argument, NULL, 'H'},
            {"1st-engine", required_argument, NULL, '1'},
            {"verbose", required_argument, NULL, 'v'},
            {0, 0, 0, 0}
        };

    while ((c = getopt_long(argc, argv, "1:fhlu:v:W:H:", options, &optindex)) > -1)
#else

    while ((c = getopt(argc, argv, "1:fhlu:v:W:H:")) > -1)
#endif /* HAVE_GETOPT_LONG */

        switch (c)
        {
        case 'h':
            printf("Usage: dreamchess [options]\n\n"
                   "An xboard-compatible chess interface.\n\n"
                   "Options:\n"
                   OPTION_TEXT("--help\t", "-h\t", "Show help.")
                   OPTION_TEXT("--list-drivers", "-l\t", "List all available drivers.")
                   OPTION_TEXT("--ui <drv>\t", "-u<drv>\t", "Use user interface driver <drv>.")
                   OPTION_TEXT("--fullscreen\t", "-f\t", "Run fullscreen")
                   OPTION_TEXT("--width\t", "-W<num>\t", "Set screen width")
                   OPTION_TEXT("--height\t", "-H<num>\t", "Set screen height")
                   OPTION_TEXT("--1st-engine <eng>", "-1<eng>\t", "Use <eng> as first chess engine.\n\t\t\t\t\t  Defaults to 'dreamer'.")
                   OPTION_TEXT("--verbose <level>", "-v<level>", "Set verbosity to <level>.\n\t\t\t\t\t  Verbosity levels:\n\t\t\t\t\t  0 - Silent\n\t\t\t\t\t  1 - Errors only\n\t\t\t\t\t  2 - Errors and warnings only\n\t\t\t\t\t  3 - All\n\t\t\t\t\t  Defaults to 1")
                  );
            exit(0);
        case 'l':
            printf("Available drivers:\n\n");
            ui_list_drivers();
            exit(0);
        case 'u':
            if (!(*ui_driver = ui_find_driver(optarg)))
            {
                DBG_ERROR("could not find user interface driver '%s'", optarg);
                exit(1);
            }
            break;
        case '1':
            *engine = optarg;
            break;
        case 'f':
            use_ui_fullscreen=1;
            break;
        case 'W':
            use_ui_width=atoi(optarg);
            break;
        case 'H':
            use_ui_height=atoi(optarg);
            break;
        case 'v':
            {
                int level;
                char *endptr;

                errno = 0;
                level = strtol(optarg, &endptr, 10);

                if (errno || (optarg == endptr) || (level < 0) || (level > 3))
                {
                    DBG_ERROR("illegal verbosity level specified");
                    exit(1);
                }

                dbg_set_level(level);
            }
        }
}
#endif

int dreamchess(void *data)
{
    char *engine = "dreamer";
#ifndef _arch_dreamcast

    arguments_t *arg = data;
#endif

    ui = ui_driver[0];

    printf( "DreamChess " "v" PACKAGE_VERSION " (r" SVN_VERSION ")\n" );

#ifdef _arch_dreamcast
    goat_init();
#endif

#ifndef _arch_dreamcast

    parse_options(arg->argc, arg->argv, &ui, &engine);
#endif

    if (!ui)
    {
        DBG_ERROR("failed to find a user interface driver");
        exit(1);
    }

    comm_init(engine);
    comm_send("xboard\n");

    ui->init(use_ui_width,use_ui_height,use_ui_fullscreen);
    while (1)
    {
        board_t board;
        int pgn_slot;

        if (!(config = ui->config(&pgn_slot)))
            break;

        comm_send("new\n");
        comm_send("random\n");

        comm_send("sd %i\n", config->cpu_level);
        comm_send("depth %i\n", config->cpu_level);
        if (config->difficulty == 0)
            comm_send("noquiesce\n");

        if (config->player[WHITE] == PLAYER_UI
                && config->player[BLACK] == PLAYER_UI)
            comm_send("force\n");

        if (config->player[WHITE] == PLAYER_ENGINE)
            comm_send("go\n");

        in_game = 1;
        board_setup(&board);
        history = history_init(&board);
        move_list_init(&san_list);
        move_list_init(&fan_list);
        move_list_init(&fullalg_list);

        if (pgn_slot >= 0)
            if (game_load(pgn_slot))
            {
                 DBG_ERROR("failed to load savegame in slot %i", pgn_slot);
                 exit(1);
            }

        ui->update(history->view->board, NULL);
        while (in_game)
        {
            char *s;

            if ((s = comm_poll()))
            {
                DBG_LOG("message from engine: '%s'", s);
                if  (!history->result)
                {
                    if ((!strncmp(s, "move ", 4) || strstr(s, "... ")) && config->player[history->last->board->turn] == PLAYER_ENGINE)
                    {
                        char *move_str = strrchr(s, ' ') + 1;
                        board_t new_board = *history->last->board;
                        move_t *engine_move;

                        DBG_LOG("parsing move string '%s'", move_str);

                        engine_move = san_to_move(&new_board, move_str);
                        if (!engine_move)
                            engine_move = fullalg_to_move(&new_board, move_str);
                        if (engine_move)
                        {
                            do_move(engine_move, 1);
                            free(engine_move);
                        }
                        else
                            DBG_ERROR("failed to parse move string '%s'", move_str);
                    }
                    else if (strstr(s, "llegal move"))
                        game_undo();
                    /* Ignore result message if we've already determined a result ourselves. */
                    else
                    {
                        char *start = strchr(s, '{');
                        char *end = strchr(s, '}');

                        if (start && end && end > start)
                        {
                            char *comment = malloc(end - start);
                            history->result = malloc(sizeof(result_t));
                            strncpy(comment, start + 1, end - start - 1);
                            comment[end - start - 1] = '\0';
                            history->result->reason = comment;
                            if (strstr(s, "1-0"))
                            {
                                history->result->code = RESULT_WHITE_WINS;
                                ui->show_result(history->result);
                            }
                            else if (strstr(s, "1/2-1/2"))
                            {
                                history->result->code = RESULT_DRAW;
                                ui->show_result(history->result);
                            }
                            else if (strstr(s, "0-1"))
                            {
                                history->result->code = RESULT_BLACK_WINS;
                                ui->show_result(history->result);
                            }
                            else
                            {
                                free(history->result->reason);
                                free(history->result);
                                history->result = NULL;
                            }
                        }
                    }
                }

                free(s);
            }
            ui->poll();
        }
        history_exit(history);
        move_list_exit(&san_list);
        move_list_exit(&fan_list);
        move_list_exit(&fullalg_list);
    }
    comm_send("quit\n");
    comm_exit();
    ui->exit();

#ifdef _arch_dreamcast
    goat_exit();
#endif

    return 0;
}
