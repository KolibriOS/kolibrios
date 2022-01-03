#ifndef SKULL_H
#define SKULL_H

typedef struct {
	int id, state, timer;
	double x, y;
	double yoffset;
	int rot;
	double dir;
	double imageIndex;
} Skull;

void createSkull(int x, int y);

#endif