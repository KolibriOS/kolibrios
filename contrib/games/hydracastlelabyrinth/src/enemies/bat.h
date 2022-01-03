#ifndef BAT_H
#define BAT_H

typedef struct {
	int id;	
	double x, y;
	int xstart, ystart;
	int type; //0 = gray | 1 = red
	int dir;
	double imageIndex;
	int counter, timer, state;
} Bat;

void createBat(int x, int y, int type);

#endif