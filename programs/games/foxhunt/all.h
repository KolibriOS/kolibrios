
#include "system/boolean.h"
#include "system/kolibri.h"
#include "system/stdlib.h"
#include "system/string.h"
#include "system/ctype.h"

#include "globals.h"


void init_board();
void init_grid_sizes();
void wnd_draw();
void grid_to_pos(unsigned gx, unsigned gy, unsigned* x, unsigned* y);
int pos_to_grid(unsigned x, unsigned y, int* gx, int* gy);
int check();
