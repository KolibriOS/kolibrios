#ifndef FIREWHEEL_H
#define FIREWHEEL_H

typedef struct {
	int id;
	double x, y;
	double imageIndex;
	int hp;
	int blink;
	int dir;
	int hsp, vsp;
	int wallx, wally;
	int timer;
} Firewheel;

void createFirewheel(int x, int y, int dir);

#endif