#ifndef THWOMP_H
#define THWOMP_H

//#include "../collision.h"

typedef struct {
	int id;
	double x, y;
	double vsp, grav;
	double imageIndex;
	int type, state, timer, dir;
	int hp, blink;
	int delay;
} Thwomp;

void createThwomp(int x, int y, int type, int offset, int delay, int dir);

#endif