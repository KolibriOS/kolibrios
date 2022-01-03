#ifndef BOAR_H
#define BOAR_H

typedef struct {
	int id;
	int hp;
	double x, y;
	double hsp;
	double imageIndex;
	int blink;
	int dir;
	int state;
	int timer;
} Boar;

void createBoar(int x, int y);

#endif