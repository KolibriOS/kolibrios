#ifndef GAS_H
#define GAS_H

//#include "../enemy.h"
//#include "../collision.h"

typedef struct {
	int id;
	int x, y;
	int state, timer;
	double imageIndex;
	
	//Mask mask;
} Gas;

void createGas(int x, int y, int temp);

#endif