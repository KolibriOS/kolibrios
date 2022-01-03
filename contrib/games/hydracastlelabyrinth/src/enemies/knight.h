#ifndef KNIGHT_H
#define KNIGHT_H

#include "../collision.h"

typedef struct {
	int id, type;
	double x, y,
		   vsp, grav;
	int dir, state, timer;
	double imageIndex;
	int hp, invincible;
	int shieldhit;
	
	Mask mask;
} Knight;

void createKnight(int x, int y, int type);

void knightStep(Knight* k);
void knightDraw(Knight* k);

#endif