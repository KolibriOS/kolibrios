#ifndef PLATFORM_H
#define PLATFORM_H

#include "collision.h"

typedef struct {
	int id, type; //0 = moving platform
	double x, y;
	int xstart, ystart;
	int xend, yend;
	int state;
	double spd;
	int timer;
	int secret, visible;
	
	Mask mask;
} Platform;

void createPlatform(int type, int xstart, int ystart, int xend, int yend, int spd, int secret);

void platformStep(Platform* p);
void platformDraw(Platform* p);

void platformDestroy(int id);

#endif