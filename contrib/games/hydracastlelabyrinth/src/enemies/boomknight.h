#ifndef BOOMKNIGHT_H
#define BOOMKNIGHT_H

typedef struct {
	int id;
	int hp;
	int blink;
	double x, y;
	int dir;
	double imageIndex;
	int state, timer;
} Boomknight;

void createBoomknight(int x, int y);

typedef struct {
	int id;
	int dir;
	double x, y;
	double hsp;
	double imageIndex;
	int timer;
} Boom;

void createBoom(int x, int y, int dir);

#endif