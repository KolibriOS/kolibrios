#ifndef BEE_H
#define BEE_H

typedef struct {
	int id;
	double x, y;
	int xstart, ystart;
	double hsp, vsp;
	double imageIndex;
	int dir, state, timer;
	double hoverdir;
} Bee;

void createBee(int x, int y, int dir);

#endif