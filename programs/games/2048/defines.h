#ifndef DEFINES_H
#define DEFINES_H

#include <string.h>
#include <stdlib.h>
#include <keys.h>
#include <menuet/gui.h>

inline void enable_scancode();
inline void clear_key_buffer();
inline void vsync();

inline __u32 text_length_px(__u32 len);

#define false   (0)
#define true    (1)

#define FONT_WIDTH      (5)
#define FONT_HEIGHT     (9)

#define ANI_DELAY       (2)             // time between animation redraw
#define ANI_APPEAR_STEP (15)            // default step for appear animation
#define ANI_MOVE_STEP   (25)            // default step for move animation

#define START_COUNT     (2)             // tiles count for new game

#define WND_WIDTH       (400)           // main window width
#define WND_HEIGHT      (400)           // main window height

#define GAME_BORDER     (30)            // minimum border size around board
#define GAME_BG_COLOR   (0xFAF8EF)      // main window background color

#define SCORE_HEIGHT    (21)            // minimum height for score text

#define BOARD_SPACING   (10)            // spacing between cells
#define BOARD_COUNT     (4)             // row and column count
#define BOARD_MAP_SIZE  (16)            // cells total count (row * column)
#define BOARD_BG_COLOR  (0xBBADA0)      // board color

#define CELL_COLOR      (0xCDC0B4)      // cell color

#endif // DEFINES_H
