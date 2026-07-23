#pragma once

// The spreadsheet model. Row/column 0 hold the headers, so the usable range
// is 1..cols-1 by 1..rows-1 (columns A..CZ, rows 1..100).
typedef struct {
	int cols, rows; // dimensions, including the header index 0
	int *w, *h;     // column widths / row heights
	int *x, *y;     // on-screen position of each column/row (set every redraw)
	char ***cells;  // cell source text
	char ***values; // computed cell values
} Table;

extern Table tbl;

// formula-relative rectangle: table_shift_formula() shifts only refs inside it
extern int cf_x0, cf_x1, cf_y0, cf_y1;

void table_init(void);
void table_reset(void);
void table_recalc(void);
int table_save(char *fname);
int table_load(char *fname);
void table_fill(int sel_x, int sel_y, int sel_end_x, int sel_end_y, int old_end_x, int old_end_y);
char *table_shift_formula(char *name, int sx, int sy);
void freeBuffer(void);
