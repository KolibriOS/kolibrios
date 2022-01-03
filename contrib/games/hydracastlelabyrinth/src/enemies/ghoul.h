#ifndef GHOUL_H
#define GHOUL_H

#include "../collision.h"

typedef struct {
	int id;
	int hp;
	double x, y;
	double vsp, grav;
	int type;
	int onground;
	int dir;
	int state, timer, invincible;
	double imageIndex;
	
	Mask mask;
} Ghoul;

void createGhoul(int x, int y, int type);

#endif