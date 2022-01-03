#ifndef WATERJUMPER_H
#define WATERJUMPER_H

#include "../enemy.h"

typedef struct {
	int id, type;
	
	double x, y;
	int hp;
	int blink;
	int ystart, rot;
	double yoffset;
	double hsp, vsp, grav;
	
	double imageIndex;	
	int state, timer;
	int height;
} WaterJumper;

void createWaterJumper(int x, int y, int type, int offset, int height);

#endif