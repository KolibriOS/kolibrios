#ifndef SLIME_H
#define SLIME_H

typedef struct {
	int id;	
	double x, y;
	int type; //0 = blue | 1 = red | 2 = yellow
	int offset;
	double vsp, hsp, grav;
	double imageIndex;
	int counter, timer, state;
	int hp;	
} Slime;

void createSlime(int x, int y, int type, int offset);

#endif