#include "changes.h"
#include "cells.h"
#include "texts.h"
#include "refs.h"
#include "undo.h"

#include <string.h>

#define REF_ERROR "=#REF!" // a formula of a reference gone, as Excel leaves it

// text into cell x, y, a formula with its references moved; an error if
// a cell of one is gone, or if it grew longer than a cell holds: cut, it
// would be another formula
static void text_set(int x, int y, const char *text, const Move *m)
{
	char moved[MOVED_MAX + 1];

	if (!text || text[0] != '=')
		cell_set(x, y, text);
	else if (formula_move(text, m, moved) && strlen(moved) <= CELL_MAX)
		cell_set(x, y, moved);
	else
		cell_set(x, y, REF_ERROR);
}

// 1 if the cells x0..x1, y0..y1 have no text
static int range_empty(int x0, int y0, int x1, int y1)
{
	int x, y;

	for (x = x0; x <= x1; x++)
		for (y = y0; y <= y1; y++)
			if (cells[x][y].text)
				return 0;
	return 1;
}

/* ===================== filling and the clipboard ==================== */

// a mod b, never negative
static int wrap(int a, int b)
{
	return ((a % b) + b) % b;
}

// cell x, y inside the rectangle x0..x1, y0..y1
static int inside(int x, int y, int x0, int y0, int x1, int y1)
{
	return x >= x0 && x <= x1 && y >= y0 && y <= y1;
}

void table_fill(int x0, int y0, int x1, int y1, int tx0, int ty0, int tx1, int ty1)
{
	int x, y, w = x1 - x0 + 1, h = y1 - y0 + 1;

	undo_save(min(x0, tx0), min(y0, ty0), max(x1, tx1), max(y1, ty1));
	// the target cells around the source first, read from it; then the
	// source cells the target left
	for (x = tx0; x <= tx1; x++)
		for (y = ty0; y <= ty1; y++)
			if (!inside(x, y, x0, y0, x1, y1)) {
				int sx = x0 + wrap(x - x0, w), sy = y0 + wrap(y - y0, h);
				Move m = { x - sx, y - sy };
				text_set(x, y, cells[sx][sy].text, &m);
			}
	for (x = x0; x <= x1; x++)
		for (y = y0; y <= y1; y++)
			if (!inside(x, y, tx0, ty0, tx1, ty1))
				cell_set(x, y, NULL);
	table_recalc();
}

void table_erase(int x0, int y0, int x1, int y1)
{
	int x, y;

	if (range_empty(x0, y0, x1, y1))
		return; // nothing to erase: no undo step, the table unchanged
	undo_save(x0, y0, x1, y1);
	for (x = x0; x <= x1; x++)
		for (y = y0; y <= y1; y++)
			cell_set(x, y, NULL);
	table_recalc();
}

static char **clip;        // the copied texts, as texts_copy() gives them
static int clip_x, clip_y; // where they came from
static int clip_w, clip_h;

void clip_copy(int x0, int y0, int x1, int y1, int cut)
{
	texts_free(clip, clip_w * clip_h);
	clip_x = x0;
	clip_y = y0;
	clip_w = x1 - x0 + 1;
	clip_h = y1 - y0 + 1;
	clip = texts_copy(x0, y0, x1, y1);
	if (!clip) {
		clip_w = clip_h = 0;
		return;
	}
	if (cut)
		table_erase(x0, y0, x1, y1);
}

int clip_cells(void)
{
	return clip_w * clip_h;
}

void clip_paste(int x, int y)
{
	// the target, cut at the edges of the table
	int x1 = min(x + clip_w - 1, COLS), y1 = min(y + clip_h - 1, ROWS), tx, ty;
	Move m = { x - clip_x, y - clip_y };

	if (!clip_cells())
		return;
	undo_save(x, y, x1, y1);
	for (tx = x; tx <= x1; tx++)
		for (ty = y; ty <= y1; ty++)
			text_set(tx, ty, clip[(tx - x) * clip_h + ty - y], &m);
	table_recalc();
}

/* ====================== inserting and deleting ===================== */

// cell k of line i: of column i, or of row i
static Cell *line_cell(int cols, int i, int k)
{
	return cols ? &cells[i][k] : &cells[k][i];
}

// line from with its texts and size to line to
static void line_move(int cols, int to, int from)
{
	int k, across = cols ? ROWS : COLS, *sizes = cols ? col_w : row_h;

	for (k = 1; k <= across; k++)
		line_cell(cols, to, k)->text = line_cell(cols, from, k)->text;
	sizes[to] = sizes[from];
}

int table_room(int cols, int n)
{
	if (cols)
		return range_empty(COLS - n + 1, 1, COLS, ROWS);
	return range_empty(1, ROWS - n + 1, COLS, ROWS);
}

void table_insert(int cols, int at, int n)
{
	int count = cols ? COLS : ROWS, across = cols ? ROWS : COLS;
	int *sizes = cols ? col_w : row_h, k, x, y, step;
	Move m = { 0, 0, cols, at, n };

	if (!undo_save_table(sizes, count))
		return; // no memory to undo it: the table as it is
	// each line from at on takes the one n before it, or is new where that
	// is past an end; from the last one for an insertion, so that none is
	// overwritten before it moves. A deleted line's texts are freed first
	for (step = 0; step <= count - at; step++) {
		int to = n > 0 ? count - step : at + step, from = to - n;
		if (to < at - n)
			for (k = 1; k <= across; k++)
				text_free(line_cell(cols, to, k)->text);
		if (from >= at && from <= count) {
			line_move(cols, to, from);
			continue;
		}
		for (k = 1; k <= across; k++)
			line_cell(cols, to, k)->text = NULL;
		sizes[to] = cols ? COL_W : ROW_H;
	}

	for (x = 1; x <= COLS; x++)
		for (y = 1; y <= ROWS; y++) {
			const char *text = cells[x][y].text;
			if (text && text[0] == '=') // only the formulas change
				text_set(x, y, text, &m);
		}
	table_recalc();
}
