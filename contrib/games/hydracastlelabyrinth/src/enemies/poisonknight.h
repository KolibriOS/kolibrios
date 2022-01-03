#ifndef POISONKNIGHT_H
#define POISONKNIGHT_H

typedef struct {
	int id;
	int hp;
	double x, y;
	double imageIndex;
	int dir;
	int blink;
	int timer;
	int state;
} Poisonknight;

void createPoisonknight(int x, int y);

typedef struct {
	int id;
	double x, y;
	int dir;
	double imageIndex;
} Goop;

void createGoop(int x, int y, int dir);

#endif