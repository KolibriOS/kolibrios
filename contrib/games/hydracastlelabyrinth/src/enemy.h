#ifndef ENEMY_H
#define ENEMY_H

typedef struct {	
	void* data; //Specific enemy struct
	void (*enemyStep)();
	void (*enemyDraw)();
	int type;
} Enemy;

void enemyDestroy(int id);

#endif