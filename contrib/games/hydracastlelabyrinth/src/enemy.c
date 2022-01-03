#include "enemy.h"
#include "game.h"
#include <stdlib.h>

void enemyDestroy(int id)
{
	if (enemies[id] != NULL) {
		if (enemies[id]->data != NULL) {
			free(enemies[id]->data);
		}
		enemies[id]->data = NULL;
	
		free(enemies[id]);
	}	
	enemies[id] = NULL;
}
