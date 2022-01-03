#ifndef GARM_H
#define GARM_H

typedef struct {
	int id;
	int hp;
	double x, y;
	double hsp, vsp;
	int dir;
	double imageIndex;
	int state, timer, blink, counter;
	int wallcounter, substate;
	int targx;
} Garm;

void createGarm(int x, int y);

typedef struct {
	int id;
	int hp;
	double x, y;
	double vsp, hsp;
	double imageIndex;
	int counter;
	int blink;
	int inwall;
} Garmrock;

void createGarmrock(int x, int y, double hsp, double vsp);

#endif