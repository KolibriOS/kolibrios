#ifndef PENDULUM_H
#define PENDULUM_H

#include "../collision.h"

typedef struct {
	int id;
	double x, y;
	double rotCounter, angle;
	
	Mask mask;
} Pendulum;

void createPendulum(int x, int y, int side);

void pendulumStep(Pendulum* p);
void pendulumDraw(Pendulum* p);

#endif