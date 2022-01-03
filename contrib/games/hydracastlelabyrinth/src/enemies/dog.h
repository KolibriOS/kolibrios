#ifndef DOG_H
#define DOG_H

typedef struct {
	int id;
	int hp;
	int blink;
	double x, y;
	double vsp, hsp;
	double imageIndex;
	int dir;
	int state, timer, counter;
} Dog;

void createDog(int x, int y);

#endif