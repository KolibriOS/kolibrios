#ifndef PUMPKIN_H
#define PUMPKIN_H

typedef struct {
	int id;
	int hp;
	int blink;
	double x, y;
	int dir;
	double imageIndex;
	int state, timer;
} Pumpkinenemy;

void createPumpkinenemy(int x, int y);

typedef struct {
	int id;
	int dir;
	double x, y;
	double vsp;
	double imageIndex;
	int state, timer;
} Pumpkinhead;

void createPumpkinhead(int x, int y, int dir);

#endif