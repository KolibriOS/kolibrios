#ifndef SHEET_REFS_H
#define SHEET_REFS_H

#include "cells.h"

// The names of the cells, A1 .. CZ100, in formulas: [$]letters[$]digits
// in any case, '$' keeping its part where it is when the formula moves.

// "A".."CZ" for column x; buf of 3
void col_name(int x, char *buf);
// "A1".."CZ100" for cell x, y; buf of 6
void cell_name(int x, int y, char *buf);
// the cell a name refers to; 0 if it is no reference, or more follows it
int ref_cell(const char *name, int *x, int *y);

// how the references of a formula move: the relative ones by dx, dy, for a
// fill or a paste; or, with n, all past the columns (cols) or rows from at
// on inserted (n > 0) or deleted (n < 0)
typedef struct {
	int dx, dy;
	int cols, at, n;
} Move;

// a formula of CELL_MAX characters moved: a reference of 2 characters
// grows to 5 at most ("A1" -> "CZ100")
#define MOVED_MAX (CELL_MAX * 5 / 2)

// a formula with its references moved into out, of MOVED_MAX + 1; 0 if
// a cell of one is gone, or all of a range
int formula_move(const char *s, const Move *m, char *out);

#endif
