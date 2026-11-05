#ifndef WINDOW_WINDOW_H
#define WINDOW_WINDOW_H

// The main window: what its modules share. window.c draws it and its
// toolbar, grid.c lays out and draws the grid and keeps the selection,
// edit.c the editing of a cell and the formula bar, input.c the mouse and
// the keys, commands.c what the toolbar, the keys and the context menu do,
// New, Open and Save among them.
//
// From the top: the toolbar, the formula bar, the grid of the cells with
// the headers of the columns on the top and of the rows on the left, the
// scrollbars on the right and bottom.

#include "../dialogs/dialogs.h"
#include "../dialogs/menu.h"
#include "../sheet/cells.h"

#include <clayer/boxlib.h>

#define TOOLBAR_H    (MENU_ITEM_H + 2 * MENU_FRAME) // the toolbar: menus of one item
#define BAR_Y        (GAP + TOOLBAR_H + GAP) // the formula bar, under the toolbar
#define BAR_H        (ROW_H + 1) // a row of the grid with both its lines
#define NAME_W       64 // the name of the active cell, "CZ100", with its left line
#define GRID_X       EDGE // the top left of the grid, its outer lines
#define GRID_Y       (BAR_Y + BAR_H + GAP)
#define HEADER_W     40 // the row numbers, "100", with the left line
#define CELLS_X      (GRID_X + HEADER_W) // the top left of the cells
#define CELLS_Y      (GRID_Y + ROW_H)
#define SCROLL_SIZE  24 // with its lines, the inner one the outer line of the grid
#define ARROW_STEP   24 // pixels a scrollbar arrow scrolls
#define VIEW_W       (grid_right - CELLS_X) // the cells shown, in pixels
#define VIEW_H       (grid_bottom - CELLS_Y)
#define CELL_PAD     4  // text to the cell edge
#define HANDLE       5  // the size of the fill handle at the corner of the selection

#define SHIFT_KEYS   (KSYS_CONTROL_LSHIFT | KSYS_CONTROL_RSHIFT)
#define CTRL_KEYS    (KSYS_CONTROL_LCTRL | KSYS_CONTROL_RCTRL)

/* ============================= window ============================= */

void place_window(void); // where it opens, once
void draw_window(void);
void toolbar_update(void); // Undo and Redo greyed when there is nothing to undo or redo
void process_button(void);
void toolbar_mouse(int x, int y); // the tool under the mouse in work_light

/* =============================== grid ============================= */

extern uint32_t mid_color;
extern int grid_right, grid_bottom;      // the lines of the scrollbars that end the grid
extern int sel_x, sel_y, end_x, end_y;   // the active cell and the other corner
extern int show_formulas;
extern int scroll_x, scroll_y;           // pixel scroll of the cells
extern int first_x, last_x, first_y, last_y; // the columns and rows shown
extern int col_x[COLS + 1], row_y[ROWS + 1]; // where, the first may be under the headers
extern scrollbar scroll_v, scroll_h;

int clamp(int v, int lo, int hi);
int fit_width(int x); // a column for the widest text it shows
void layout_grid(void);
void scroll_to(int x, int y);
void cell_under(int mx, int my, int *x, int *y);
void sel_rect(int *x0, int *y0, int *x1, int *y1);
int col_line(int x), row_line(int y); // on the screen
int handle_shown(int right, int bottom);
void cell_bar(int x, int y, int w, int h, uint32_t color);
void draw_centred(int x, int y, int w, int h, const char *s, int left, uint32_t color);
void read_colors(void);
void draw_grid(void);
void draw_frame(void); // of the selection
// after the selection or the fill moved: only the cells it changes, unless
// the grid scrolled or the cells changed
void draw_selection(void);

/* =============================== edit ============================= */

extern edit_box cell_box, bar_box;
extern edit_box *editor; // the box with the focus, NULL when not editing

void edit_draw(void); // the formula bar, and the box over the cell when editing
void set_editor(edit_box *box);
void start_edit(int keep, edit_box *box);
void edit_follow(void);
void stop_edit(void);
void cancel_edit(void);
void draw_box_lines(void);
int in_box(edit_box *box, int x, int y);

/* ============================== input ============================= */

// what the left mouse button drags
enum { DRAG_NONE, DRAG_SELECT, DRAG_FILL, DRAG_COL, DRAG_ROW };
extern int drag;

void mouse_init(void);
void fill_target(int *tx0, int *ty0, int *tx1, int *ty1);
void process_mouse(void);
void drag_tick(void);
// the inverted line of a column or row resized: drawn again, it is gone
void draw_drag_line(void);
void process_key(void);

/* ============================ commands ============================ */

enum { CMD_CUT, CMD_COPY, CMD_PASTE, CMD_CLEAR, CMD_INSERT_BEFORE, CMD_INSERT_AFTER, CMD_DELETE };

// what the buttons of the toolbar, their hotkeys and the close button of
// the window do
enum { TOOL_NEW, TOOL_OPEN, TOOL_SAVE, TOOL_UNDO, TOOL_REDO, TOOL_HELP, TOOL_CLOSE };
void tool_command(int tool);

// the file of the table: the dialog and the name of the program once,
// the title, loading it
void files_init(void);
void set_title(void); // of the window: the name of the file after the name of the program
void load_file(const char *name);

void select_range(int x0, int y0, int x1, int y1);
void select_cell(int x, int y);
void run_command(int cmd);
// what a point of the grid is on: the cells, or the headers of the
// columns or the rows; the menu of the right button on it, at a point of
// the screen
enum { ON_CELLS, ON_COLS, ON_ROWS };
void context_menu(int target, int x, int y);

#endif
