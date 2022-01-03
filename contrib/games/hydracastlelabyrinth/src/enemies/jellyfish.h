#ifndef JELLYFISH_H
#define JELLYFISH_H

typedef struct {
	int id;
	double x, y;
	int ystart;
	double spd;
	double angle;
	int state;
	double imageIndex;
} Jellyfish;

void createJellyfish(int x, int y);

#endif