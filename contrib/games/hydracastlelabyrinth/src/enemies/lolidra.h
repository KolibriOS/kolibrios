#ifndef LOLIDRA_H
#define LOLIDRA_H

#include "../collision.h"

typedef struct {
	int id;
	double x, y;
	double positionY;
	double imageIndex, hoverRot;
	int hp, state, invincible,
		visible, timer, counter;
	
	Mask mask;
} Lolidra;

void createLolidra(int x, int y);

void lolidraStep(Lolidra* l);
void lolidraDraw(Lolidra* l);

//Minion
typedef struct {
	int id;
	int state;
	int timer;
	double x, y;
	double positionY;
	double imageIndex;
	double dir, spd;
	
	Mask mask;
} Minion;

void createMinion(int x, int y);

void minionStep(Minion* m);
void minionDraw(Minion* m);

#endif