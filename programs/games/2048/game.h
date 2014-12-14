#ifndef GAME_H
#define GAME_H

#include "defines.h"
#include "board.h"

static const char header[] = "2048";
static struct process_table_entry proc_info = {0};

#define NEW_GAME_BUTTON (0xFF)

// Start a new game
void game_init();

// Exit game
void game_exit();

// Redraw game content
void game_redraw();

// Move Up
void game_move_up();

// Move Down
void game_move_down();

// Move Left
void game_move_left();

// Move Right
void game_move_right();

#endif // GAME_H
