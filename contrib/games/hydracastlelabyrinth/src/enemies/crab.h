#ifndef CRAB_H
#define CRAB_H

#include "../collision.h"

typedef struct {
	int id;
	int hp, invincible;
	double x, y;
	double hsp, vsp;
	double imageIndex;
	int state, timer, counter;
	
	Mask mask;
} Crab;

void createCrab(int x, int y);

typedef struct {
	int id;
	double x, y;
	double angle;
	double imageIndex;
	
	Mask mask;
} Electricity;

void createElectricity(int x, int y, double angle, int minid);

#endif