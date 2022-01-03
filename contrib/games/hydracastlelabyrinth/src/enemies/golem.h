#ifndef GOLEM_H
#define GOLEM_H

typedef struct {
	int id;
	double x, y;
	double imageIndex;
	int hp;
	int dir;
	int state;
	int blink;
} Golem;

void createGolem(int x, int y, int dir);

#endif