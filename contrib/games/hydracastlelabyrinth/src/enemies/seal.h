#ifndef SEAL_H
#define SEAL_H

typedef struct {
	int id, hp;
	double x, y;
	double imageIndex;
	int dir, state, timer;
	int invincible;
} Seal;

void createSeal(int x, int y);

#endif