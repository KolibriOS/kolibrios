#ifndef PODOBOO_H
#define PODOBOO_H

typedef struct {
	int id;
	double x, y;
	int ystart;	
	int hp;
	int blink;	
	int rot;
	double yoffset;	
	double vsp, grav;
	double jumpheight;
	double imageIndex;
	int timer, state;
} Podoboo;

void createPodoboo(int x, int y, int offset, int height);

#endif