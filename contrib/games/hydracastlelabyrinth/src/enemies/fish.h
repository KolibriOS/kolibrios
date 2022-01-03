#ifndef FISH_H
#define FISH_H

#include "../enemy.h"
#include "../collision.h"

typedef struct {
	int id;
	
	double x, y;
	int xstart;
	double imageIndex;
	double spd;	
	int dir, turning;
	
	Mask mask;
} Fish;

void createFish(int x, int y, int dir);

void fishStep(Fish* f);
void fishDraw(Fish* f);

#endif