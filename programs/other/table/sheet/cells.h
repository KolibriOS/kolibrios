#ifndef SHEET_CELLS_H
#define SHEET_CELLS_H

// The cells of the spreadsheet and what they show; no window here.
//
// The spreadsheet: columns A..CZ, rows 1..100. A cell is cells[col][row]
// with col 1..COLS and row 1..ROWS, the numbers the user sees; index 0 is
// unused.

#include <stdlib.h> // its min and max macros, undefined below

// min() and max() of ints as functions: under tcc libc's macros grow every
// use into a lot of code, and evaluate a call in them twice
#undef min
#undef max
int min(int a, int b);
int max(int a, int b);
#define COLS 104
#define ROWS 100

// sizes in pixels with the grid line on the left or top
#define COL_W    80
#define ROW_H    23   // 22 inside: the 10 px capitals of the font centred
#define MIN_SIZE 16
#define MAX_SIZE 1000

#define CELL_MAX 255  // the longest text of a cell

// what a cell shows after table_recalc()
enum {
	VALUE_TEXT,   // its text as it is: no formula, or not a number
	VALUE_NUMBER, // a number: typed, or a formula computed
	VALUE_ERROR   // a formula that cannot be computed
};

typedef struct {
	char *text; // what was typed, NULL for an empty cell
	int kind;   // VALUE_*
	double number;
	int error;  // of VALUE_ERROR: ERR_* of formula.h
} Cell;

extern Cell cells[COLS + 1][ROWS + 1];
extern int col_w[COLS + 1], row_h[ROWS + 1];
extern int table_changed; // its texts, since it was cleared, loaded or saved: undo marks it
extern int table_recalcs;  // how many times table_recalc() ran: a change of the cells

void table_clear(void); // the undo steps stay: undo_forget()
// a copy of text into the cell, at most CELL_MAX characters, control
// characters as spaces; NULL or "" empties it
void cell_set(int x, int y, const char *text);
// a copy of the texts of the cells x0..x1, y0..y1,
// texts[(x - x0) * height + y - y0], or NULL if no memory; texts_free()
// frees its n texts and it
char **texts_copy(int x0, int y0, int x1, int y1);
void texts_free(char **texts, int n);
void table_recalc(void);

// what cell x, y shows: its text as typed, or with show_formulas 0 the
// value of its formula; NULL if empty. buf of 32 for a number, which is
// NUMBER_MAX characters at most: a sign, 15 digits and the point
#define NUMBER_MAX 17
const char *cell_show(int x, int y, int show_formulas, char *buf);

#endif
