#ifndef BATBOSS_H
#define BATBOSS_H

typedef struct {
	int id;
	double x, y;
	double hsp, vsp, grav;
	double imageIndex;
	double ypos;
	double rot;
	int hp;
	int state, timer, mode;
	int invincible;
} Batboss;

void createBatboss(int x, int y);

#endif