#include "cells.h"
#include "texts.h"
#include "../formula/formula.h"
#include "refs.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

Cell cells[COLS + 1][ROWS + 1];
int col_w[COLS + 1], row_h[ROWS + 1];
int table_changed, table_recalcs;

// a formula cell inside table_recalc(), not computed yet
#define VALUE_PENDING (VALUE_ERROR + 1)

int min(int a, int b) { return a < b ? a : b; }
int max(int a, int b) { return a > b ? a : b; }

/* ============================== cells ============================= */

void cell_set(int x, int y, const char *text)
{
	int i, n = text ? min((int)strlen(text), CELL_MAX) : 0;
	char *t = n ? text_alloc(n + 1) : NULL;

	if (t) {
		for (i = 0; i < n; i++) // a line end would break a line of the file
			t[i] = (unsigned char)text[i] < ' ' ? ' ' : text[i];
		t[n] = '\0';
	}
	text_free(cells[x][y].text);
	cells[x][y].text = t;
}

char **texts_copy(int x0, int y0, int x1, int y1)
{
	int x, y, i = 0;
	char **texts = malloc((x1 - x0 + 1) * (y1 - y0 + 1) * sizeof(char *));

	if (!texts)
		return NULL;
	for (x = x0; x <= x1; x++)
		for (y = y0; y <= y1; y++, i++) { // i: (x - x0) * height + y - y0
			const char *text = cells[x][y].text;
			texts[i] = text ? text_copy(text) : NULL;
			if (text && !texts[i]) { // no memory: none
				texts_free(texts, i);
				return NULL;
			}
		}
	return texts;
}

void texts_free(char **texts, int n)
{
	int i;

	for (i = 0; i < n; i++)
		text_free(texts[i]);
	free(texts);
}

void table_clear(void)
{
	int x, y;

	table_changed = 0;
	for (x = 1; x <= COLS; x++) {
		for (y = 1; y <= ROWS; y++)
			cell_set(x, y, NULL);
		col_w[x] = COL_W;
	}
	for (y = 1; y <= ROWS; y++)
		row_h[y] = ROW_H;
	table_recalc();
}

/* =========================== computing ============================ */

// set when a formula meets one not computed yet: it waits for the next pass
static int deferred;

// the value of a cell reference, for compute(): its number, 0 for an
// empty cell, or an error: ERR_VALUE for a text, the error of its formula,
// ERR_NAME for no reference
static int ref_value(const char *name, double *value)
{
	int x, y;
	Cell *c;

	if (!ref_cell(name, &x, &y))
		return ERR_NAME;
	c = &cells[x][y];
	if (c->kind == VALUE_PENDING)
		deferred = 1;
	*value = c->text ? c->number : 0.0;
	if (c->text && c->kind == VALUE_TEXT)
		return ERR_VALUE;
	return c->kind == VALUE_ERROR ? c->error : ERR_NONE;
}

// the numbers of the cells from..to, its ends in any order, for compute():
// texts and empty cells skipped; the first error of a cell, ERR_NAME if an
// end is no reference
static int range_values(const char *from, const char *to, List *list)
{
	int x, y, x0, y0, x1, y1;

	if (!ref_cell(from, &x0, &y0) || !ref_cell(to, &x1, &y1))
		return ERR_NAME;
	for (x = min(x0, x1); x <= max(x0, x1); x++)
		for (y = min(y0, y1); y <= max(y0, y1); y++) {
			Cell *c = &cells[x][y];
			if (c->kind == VALUE_PENDING)
				deferred = 1;
			if (c->kind == VALUE_ERROR)
				return c->error;
			if (c->kind == VALUE_NUMBER)
				list_add(list, c->number);
		}
	return ERR_NONE;
}

// the formula cells table_recalc() found, in the order of the table
static Cell *formula_cells[COLS * ROWS];

void table_recalc(void)
{
	int x, y, i, len, e, n = 0, pass, pending, done;

	// a plain cell is a number or a text; a formula waits
	for (x = 1; x <= COLS; x++)
		for (y = 1; y <= ROWS; y++) {
			Cell *c = &cells[x][y];
			c->kind = VALUE_TEXT;
			if (!c->text)
				continue;
			if (c->text[0] == '=') {
				c->kind = VALUE_PENDING;
				formula_cells[n++] = c;
				continue;
			}
			c->number = parse_number(c->text, &len, 1);
			if (len && c->text[len] == '\0')
				c->kind = VALUE_NUMBER;
		}

	pending = n;
	table_recalcs++;

	// the formulas pass after pass, forward and backward in turn, so that a
	// chain of references either way is done in two: one that meets a
	// formula not computed yet waits for the next pass; a pass that
	// computes none leaves cycles
	for (pass = 0; pending; pass++) {
		done = 0;
		for (i = 0; i < n; i++) {
			Cell *c = formula_cells[pass % 2 ? n - 1 - i : i];
			if (c->kind != VALUE_PENDING)
				continue;
			deferred = 0;
			e = compute(c->text + 1, &c->number, ref_value, range_values); // past the '='
			if (deferred)
				continue;
			c->kind = e == ERR_NONE ? VALUE_NUMBER : VALUE_ERROR;
			c->error = e;
			done++;
		}
		if (!done)
			break;
		pending -= done;
	}

	// what is left depends on itself: #REF!, as Google Sheets shows it
	for (i = 0; i < n; i++) {
		Cell *c = formula_cells[i];
		if (c->kind == VALUE_PENDING) {
			c->kind = VALUE_ERROR;
			c->error = ERR_REF;
		}
	}
}

// a number with the 15 digits a double holds, 6 decimals at most, without
// the zeros it does not need: 10, 3.5, 0.333333, 1234567890.25
static void format_number(double d, char *buf)
{
	double a = fabs(d), scale = 1e6, limit;
	int decimals = 6;
	char *p;

	if (a >= 1e15) {
		sprintf(buf, "%g", d);
		return;
	}
	// a decimal less for each whole digit past nine
	for (limit = 1e9; limit <= a; limit *= 10) {
		scale /= 10;
		decimals--;
	}
	a = round(a * scale) / scale;
	sprintf(buf, "%.*f", decimals, d < 0.0 && a != 0.0 ? -a : a); // -0.0000001 is 0, not -0
	if (decimals) {
		for (p = buf + strlen(buf) - 1; *p == '0'; p--)
			*p = '\0';
		if (*p == '.')
			*p = '\0';
	}
}

const char *cell_show(int x, int y, int show_formulas, char *buf)
{
	Cell *c = &cells[x][y];

	// an empty cell is a VALUE_TEXT one
	if (show_formulas || c->kind == VALUE_TEXT || c->text[0] != '=')
		return c->text; // as typed
	if (c->kind == VALUE_ERROR)
		return error_text(c->error);
	format_number(c->number, buf);
	return buf;
}
