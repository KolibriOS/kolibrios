#include "wizard.h"
#include "../enemy.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>

void createWizard(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Wizard* w = malloc(sizeof *w);
			w->id = i;
			
			w->x = x;
			w->y = y;
			
			w->imageIndex = 0;
			
			w->state = 0;
			w->timer = 50;
			w->visible = 1;
			
			w->mask.circle = w->mask.unused = 0;
			w->mask.w = 24;
			w->mask.h = 38;
			w->mask.x = w->x + 8;
			w->mask.y = w->y + 2;
			
			e->data = w;
			e->enemyStep = wizardStep;
			e->enemyDraw = wizardDraw;
			e->type = 21;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void wizardStep(Wizard* w)
{
	w->imageIndex += 0.3;
	if (w->imageIndex >= 3) {
		w->imageIndex -= 3;
	}
	
	//Stand still
	if (w->state == 0) {
		w->timer -= 1;
		
		if (w->timer <= 0) {
			PHL_PlaySound(sounds[sndPi10], CHN_ENEMIES);
			w->state = 1;
			w->timer = 15;
		}		
	}
	//Flash
	else if (w->state == 1 || w->state == 3) {
		if (w->visible == 0) {
			w->visible = 1;
		}else{
			w->visible = 0;
		}
		
		w->timer -= 1;
		if (w->timer <= 0) {
			if (w->state == 1) {
				w->state = 2;
				w->timer = 60;
			}
			else if (w->state == 3) {
				w->visible = 1;
				w->state = 0;
				w->timer = 50;
			}
		}
	}
	//Invisible
	else if (w->state == 2) {
		w->visible = 0;
		
		w->timer -= 1;
		if (w->timer <= 0) {
			PHL_PlaySound(sounds[sndPi03], CHN_ENEMIES);
			w->state = 3;
			w->timer = 15;
			
			//Horizontal Jump
			int gridX = w->x / 40,
				gridY = w->y / 40,
				lastGridX = gridX;
				
			do {
				gridX = (rand() % 16) + 1;
			} while (collisionTiles[gridX][gridY] != 0 ||
					 collisionTiles[gridX][gridY+1] != 1 ||
					 gridX == lastGridX);
					 
			w->x = gridX * 40;
			w->mask.x = w->x + 8;
		}
	}
	
	if (w->state == 0 || w->state == 3) {
		//Hit Player
		if (checkCollision(w->mask, getHeroMask())) {
			heroHit(15, w->x + 20);
		}
		
		//Weapon Collision
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (checkCollision(w->mask, weapons[i]->weaponMask)) {
					weaponHit(weapons[i]);					
					createEffect(2, w->x - 12, w->y - 6);
					spawnCollectable(w->x + 20, w->y);
					enemyDestroy(w->id);
					i = MAX_WEAPONS;
				}
			}	
		}
	}
}

void wizardDraw(Wizard* w)
{
	if (w->visible == 1) {			
		PHL_DrawSurfacePart(w->x, w->y, 520 + (((int)w->imageIndex) * 40), 480, 40, 40, images[imgEnemies]);
	}
}