#include "refs.h"
#include "../formula/formula.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

void col_name(int x, char *buf)
{
	// A..Z, then AA..CZ
	if (x > 26)
		*buf++ = 'A' + (x - 1) / 26 - 1;
	*buf++ = 'A' + (x - 1) % 26;
	*buf = '\0';
}

void cell_name(int x, int y, char *buf)
{
	col_name(x, buf);
	sprintf(buf + strlen(buf), "%d", y);
}

// a cell reference at s, [$]letters[$]digits in any case: its length, or 0
// if s starts with none; the cell into *x, *y, whether each part is
// absolute ($) into *abs_x, *abs_y
static int ref_parse(const char *s, int *x, int *y, int *abs_x, int *abs_y)
{
	const char *p = s;
	int col = 0, row = 0;

	*abs_x = *p == '$';
	if (*abs_x)
		p++;
	if (!isalpha((unsigned char)*p))
		return 0;
	while (isalpha((unsigned char)*p)) {
		col = col * 26 + tolower((unsigned char)*p) - 'a' + 1;
		if (col > COLS)
			return 0;
		p++;
	}
	*abs_y = *p == '$';
	if (*abs_y)
		p++;
	if (!isdigit((unsigned char)*p))
		return 0;
	while (isdigit((unsigned char)*p)) {
		row = row * 10 + *p - '0';
		if (row > ROWS)
			return 0;
		p++;
	}
	if (row == 0)
		return 0;
	*x = col;
	*y = row;
	return p - s;
}

int ref_cell(const char *name, int *x, int *y)
{
	int abs_x, abs_y, len = ref_parse(name, x, y, &abs_x, &abs_y);

	return len != 0 && name[len] == '\0';
}

// a reference moved; 0 if its cell is gone, or a fill or a paste moves it
// out of the table, as in Excel. end: 0 for a reference alone;
// for an end of a range -1 the low one, 1 the high one on the axis of the
// insertion or deletion. Deleted, an end moves in to the first column or
// row after or the last one before; pushed past the table, the high end
// stays at its edge
static int move_ref(const Move *m, int *x, int *y, int abs_x, int abs_y, int end)
{
	int *v, last = m->cols ? COLS : ROWS;

	if (!m->n) {
		if (!abs_x)
			*x += m->dx;
		if (!abs_y)
			*y += m->dy;
		return *x >= 1 && *x <= COLS && *y >= 1 && *y <= ROWS;
	}
	v = m->cols ? x : y;
	if (*v < m->at)
		return 1;
	if (m->n < 0 && *v < m->at - m->n) { // deleted
		if (!end)
			return 0;
		*v = end < 0 ? m->at : m->at - 1;
		return 1;
	}
	*v += m->n;
	if (end > 0 && *v > last)
		*v = last;
	return *v <= last;
}

int formula_move(const char *s, const Move *m, char *out)
{
	char *o = out;

	while (*s) {
		// a whole name or number at a time, never a reference inside one
		int x[2], y[2], abs_x[2], abs_y[2], *v = m->cols ? x : y;
		int run = word_len(s), next, ends = 1, low = 0, i;

		if (run == 0) {
			*o++ = *s++;
			continue;
		}
		if (ref_parse(s, &x[0], &y[0], &abs_x[0], &abs_y[0]) != run) {
			memcpy(o, s, run);
			o += run;
			s += run;
			continue;
		}
		// a range, A1:B3, its ends in any order: both at once, low the
		// index of the one first on the axis of a move
		next = s[run] == ':' ? word_len(s + run + 1) : 0;
		if (next && ref_parse(s + run + 1, &x[1], &y[1], &abs_x[1], &abs_y[1]) == next) {
			ends = 2;
			low = v[1] < v[0];
			run += 1 + next;
		}
		for (i = 0; i < ends; i++) {
			char name[3];
			int end = 0;
			if (ends == 2)
				end = i == low ? -1 : 1;
			if (!move_ref(m, &x[i], &y[i], abs_x[i], abs_y[i], end))
				return 0;
			col_name(x[i], name);
			o += sprintf(o, "%s%s%s%s%d", i ? ":" : "", abs_x[i] ? "$" : "", name, abs_y[i] ? "$" : "", y[i]);
		}
		if (ends == 2 && m->n < 0 && v[low] > v[1 - low]) // all of it deleted
			return 0;
		s += run;
	}
	*o = '\0';
	return 1;
}
