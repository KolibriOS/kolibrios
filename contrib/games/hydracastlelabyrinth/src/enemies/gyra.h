#ifndef GYRA_H
#define GYRA_H

typedef struct {
	int id;
	int hp;
	double x, y;
	double xrecord[144];
	double yrecord[144];
	int state, timer, counter;
	int targx, targy;
	int invincible;
	double dir;
	double imageIndex;
} Gyra;

void createGyra(int x, int y);

#endif