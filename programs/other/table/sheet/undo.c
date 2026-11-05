#include "cells.h"
#include "texts.h"
#include "undo.h"

#include <stdlib.h>
#include <string.h>

#define UNDO_MAX 64 // the steps kept each way

// a rectangle of cells and sizes of columns or rows as they were: a step
// changes the one, the other or both
typedef struct {
	int x0, y0, x1, y1; // x1 < x0: no cells
	char **texts;       // texts[(x - x0) * height + y - y0]
	int *sizes;         // col_w or row_h, NULL: no sizes
	int first, count;   // sizes[first..first + count - 1]
	int *saved;         // their values, NULL if no memory
} Snapshot;

static Snapshot undo_list[UNDO_MAX], redo_list[UNDO_MAX];
static int undo_count, redo_count;

// 1 if it holds all it is asked for, 0 if there was no memory for some
static int snapshot_take(Snapshot *s, int x0, int y0, int x1, int y1, int *sizes, int first, int count)
{
	int has_cells = x1 >= x0;

	s->x0 = x0;
	s->y0 = y0;
	s->x1 = x1;
	s->y1 = y1;
	s->texts = has_cells ? texts_copy(x0, y0, x1, y1) : NULL;
	if (!s->texts)
		s->x1 = x0 - 1; // no memory, or no cells
	s->sizes = sizes;
	s->first = first;
	s->count = count;
	s->saved = sizes ? malloc(count * sizeof(int)) : NULL;
	if (s->saved)
		memcpy(s->saved, sizes + first, count * sizeof(int));
	return (!has_cells || s->texts) && (!sizes || s->saved);
}

static void snapshot_free(Snapshot *s)
{
	int n = s->x1 >= s->x0 ? (s->x1 - s->x0 + 1) * (s->y1 - s->y0 + 1) : 0;

	texts_free(s->texts, n);
	free(s->saved);
}

// the table as the snapshot has it; its texts go to the cells
static void snapshot_restore(Snapshot *s)
{
	int x, y, i = 0;

	for (x = s->x0; x <= s->x1; x++)
		for (y = s->y0; y <= s->y1; y++, i++) { // i: (x - x0) * height + y - y0
			text_free(cells[x][y].text);
			cells[x][y].text = s->texts[i];
		}
	free(s->texts);
	if (s->saved)
		memcpy(s->sizes + s->first, s->saved, s->count * sizeof(int));
	free(s->saved);
}

// a new snapshot on a list, the oldest dropped when it is full
static Snapshot *list_push(Snapshot *list, int *n)
{
	if (*n == UNDO_MAX) {
		snapshot_free(&list[0]);
		memmove(&list[0], &list[1], (UNDO_MAX - 1) * sizeof(Snapshot));
		(*n)--;
	}
	return &list[(*n)++];
}

static void list_clear(Snapshot *list, int *n)
{
	while (*n)
		snapshot_free(&list[--*n]);
}

// a new snapshot on a list of the cells and sizes as they are; 0 and none
// if there is no memory for all of it (a full list has lost its oldest
// one all the same)
static int list_save(Snapshot *list, int *n, int x0, int y0, int x1, int y1, int *sizes, int first, int count)
{
	Snapshot *s = list_push(list, n);

	if (snapshot_take(s, x0, y0, x1, y1, sizes, first, count))
		return 1;
	snapshot_free(s);
	(*n)--;
	return 0;
}

// the table changes: what was undone cannot be redone on it; its cells
// too, or only the sizes, which its file does not keep
static void table_changes(int cells)
{
	list_clear(redo_list, &redo_count);
	if (cells)
		table_changed = 1;
}

void undo_save(int x0, int y0, int x1, int y1)
{
	list_save(undo_list, &undo_count, x0, y0, x1, y1, NULL, 0, 0); // no memory: the change goes on without its step
	table_changes(1);
}

void undo_save_size(int *sizes, int index)
{
	list_save(undo_list, &undo_count, 1, 1, 0, 0, sizes, index, 1); // no cells
	table_changes(0);
}

int undo_save_table(int *sizes, int count)
{
	if (!list_save(undo_list, &undo_count, 1, 1, COLS, ROWS, sizes, 1, count))
		return 0; // no memory: the table stays as it is
	table_changes(1);
	return 1;
}

// the last step of from given back to the table; the table as it was
// saved on to, to give it back in turn. 0 for no step, or no memory to
// save the table: the step stays
static int step_back(Snapshot *from, int *from_n, Snapshot *to, int *to_n,
                     int *x0, int *y0, int *x1, int *y1)
{
	Snapshot *s;

	if (!*from_n)
		return 0;
	s = &from[*from_n - 1];
	if (!list_save(to, to_n, s->x0, s->y0, s->x1, s->y1, s->sizes, s->first, s->count))
		return 0;
	(*from_n)--;
	*x0 = s->x0;
	*y0 = s->y0;
	*x1 = s->x1;
	*y1 = s->y1;
	if (s->x1 >= s->x0) // its cells: the sizes alone are not in the file
		table_changed = 1;
	snapshot_restore(s);
	table_recalc();
	return 1;
}

int table_undo(int redo, int *x0, int *y0, int *x1, int *y1)
{
	if (redo)
		return step_back(redo_list, &redo_count, undo_list, &undo_count, x0, y0, x1, y1);
	return step_back(undo_list, &undo_count, redo_list, &redo_count, x0, y0, x1, y1);
}

int undo_steps(void) { return undo_count; }
int redo_steps(void) { return redo_count; }

void undo_forget(void)
{
	list_clear(undo_list, &undo_count);
	list_clear(redo_list, &redo_count);
}

