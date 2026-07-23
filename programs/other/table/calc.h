#pragma once

// formula-relative rectangle: change_formula() shifts only refs inside it
extern int cf_x0, cf_x1, cf_y0, cf_y1;

void init(void);
void reinit(void);
void calculate_values(void);
int SaveFile(char *fname);
int LoadFile(char *fname);
void fill_cells(int sel_x, int sel_y, int sel_end_x, int sel_end_y, int old_end_x, int old_end_y);
char *change_formula(char *name, int sx, int sy);
void freeBuffer(void);
