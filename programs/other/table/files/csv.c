#include "../sheet/cells.h"
#include "csv.h"
#include "encoding.h"

#include <stdlib.h>
#include <string.h>

int csv_load(char *p, char sep)
{
	int x = 1, y = 1, whole = 1;

	while (*p && y <= ROWS) {
		char *field = p, *f = p, end; // unquoted in place

		if (*p == '"') {
			for (p++; *p; p++) {
				if (*p == '"' && p[1] != '"')
					break;
				if (*p == '"')
					p++; // "" is one quote
				*f++ = *p;
			}
			if (*p == '"')
				p++;
		} else
			while (*p && *p != sep && *p != '\n')
				*f++ = *p++;
		// on past the rest of the field to its end, kept: the zero of the
		// field may fall on it
		while (*p && *p != sep && *p != '\n')
			p++;
		end = *p;
		*f = '\0';
		if (x <= COLS)
			cell_set(x, y, field);
		if ((x > COLS && *field) || f - field > CELL_MAX)
			whole = 0;
		if (end == sep)
			x++;
		else if (end == '\n') {
			x = 1;
			y++;
		}
		if (end)
			p++;
	}
	// past the last row: empty lines and fields only
	while (*p == sep || *p == '\n')
		p++;
	return whole && !*p;
}

char csv_separator(const char *p)
{
	int quoted = 0;

	for (; *p && (quoted || *p != '\n'); p++) {
		if (*p == '"')
			quoted = !quoted;
		else if (*p == ';' && !quoted)
			return ';';
	}
	return ',';
}

static void out_char(Out *o, char c)
{
	if (o->len == o->size) {
		int size = o->size * 2 + 256; // twice as large and 256 bytes more
		char *grown = realloc(o->data, size);
		if (!grown) {
			o->failed = 1;
			return;
		}
		o->data = grown;
		o->size = size;
	}
	o->data[o->len++] = c;
}

void out_text(Out *o, const char *s)
{
	for (; *s; s++)
		out_char(o, *s);
}

// a character of a cell, in UTF-8 if the file is
static void out_cell_char(Out *o, char c)
{
	char utf8[2];
	int i, n = 1;

	utf8[0] = c;
	if (o->utf8)
		n = cp866_to_utf8(c, utf8);
	for (i = 0; i < n; i++)
		out_char(o, utf8[i]);
}

// the text of a cell; in quotes if it holds a quote or either separator:
// a ',' unquoted in a file of ';' with one column would make it one of ','
static void out_field(Out *o, const char *s)
{
	int quoted = strchr(s, ',') || strchr(s, ';') || strchr(s, '"');

	if (quoted)
		out_char(o, '"');
	for (; *s; s++) {
		if (*s == '"')
			out_char(o, '"');
		out_cell_char(o, *s);
	}
	if (quoted)
		out_char(o, '"');
}

void csv_save(Out *o, char sep, const char *eol)
{
	int x, y, x1 = 0, y1 = 0; // the used cells from A1 to x1, y1

	for (x = 1; x <= COLS; x++)
		for (y = 1; y <= ROWS; y++)
			if (cells[x][y].text) {
				x1 = max(x1, x);
				y1 = max(y1, y);
			}
	for (y = 1; y <= y1; y++) {
		for (x = 1; x <= x1; x++) {
			const char *text = cells[x][y].text;
			if (text)
				out_field(o, text);
			if (x < x1)
				out_char(o, sep);
		}
		out_text(o, eol);
	}
}
