#ifndef BOARD_H
#define BOARD_H

#include "defines.h"
#include "cell.h"

// Draw a new board
void board_init(rect* r);

// Redraw board and all content (animation will started if needed)
void board_redraw();

// Add one tile with 2 or 4 value in a random cell place
// Return true if tile added, false - if no more place for tile
__u8 board_add_random_tile();

// Check for available moves
// Return true if board has moves, false - if not
__u8 board_has_moves();

// Get score
__u32 board_score();

// Try to move all tiles up
// Will return true if something moved or false - if not
__u8 board_up();

// Try to move all tiles down
// Will return true if something moved or false - if not
__u8 board_down();

// Try to move all tiles left
// Will return true if something moved or false - if not
__u8 board_left();

// Try to move all tiles right
// Will return true if something moved or false - if not
__u8 board_right();

#endif // BOARD_H
