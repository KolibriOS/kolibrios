#ifndef COLLISION_H
#define COLLISION_H

#include "PHL.h"

typedef struct {
	int circle; //1 if circle, 0 is rectangle
	int x, y;
	int w, h; //width is the radius if it's a circle
	int unused;
} Mask;

void PHL_DrawMask(Mask m);

int checkCollision(Mask m1, Mask m2);

int checkTileCollision(int type, Mask m);
PHL_Rect getTileCollision(int type, Mask m);

int checkCollisionXY(Mask m, int x, int y);

int checkTileCollisionXY(int type, int x, int y);
PHL_Rect getTileCollisionXY(int type, int x, int y);

PHL_Rect getTileCollisionWeapon(int type, Mask m);

#endif