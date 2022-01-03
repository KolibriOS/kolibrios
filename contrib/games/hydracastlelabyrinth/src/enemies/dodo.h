#ifndef DODO_H
#define DODO_H

#include "../collision.h"

typedef struct {
	int id;
	double x, y;
	double vsp, hsp, grav;
	int dir, onground;
	double imageIndex;
	int state, timer, hp;
	int blink;
	int tojump, jumptoggle;
	int flag;
	
	//Mask mask;
} Dodo;

void createDodo(int x, int y, int flag);

#endif