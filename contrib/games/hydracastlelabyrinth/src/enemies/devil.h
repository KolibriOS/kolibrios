#ifndef DEVIL_H
#define DEVIL_H

typedef struct {
	int id;
	double x, y;
	double ystart, newystart;
	double hsp;
	int hp;
	int state, timer;
	int blink;
	int boblen, bobspd;
	double tailangle, bobcounter, rotcounter, rotspd;
	double imageIndex;
} Devil;

void createDevil(int x, int y);

typedef struct {
	int id;
	double x, y;
	double dir;
	double imageIndex;
} Orb;

void createOrb(int x, int y, double dir);

#endif