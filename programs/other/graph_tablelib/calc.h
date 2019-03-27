
#pragma once

#include "KosSyst.h"

extern int cf_x0, cf_x1, cf_y0, cf_y1;

void calculate_values();
int get_x(int x);
int get_y(int y);
char *make_col_cap(int i);
char *make_row_cap(int i);
void init();
void reinit();
int SaveFile(char *fname);
int LoadFile(char *fname);
int SaveCSV(char *fname);
int LoadCSV(char *fname);
void fill_cells(int sel_x, int sel_y, int sel_end_x, int sel_end_y, int old_end_x, int old_end_y);
int parse_cell_name(char *str, int *px, int *py, int *xd = NULL, int *yd = NULL);
char *make_cell_name(int x, int y, int xd, int yd);
char *change_formula(char *name, int sx, int sy);

void freeBuffer();