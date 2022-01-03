#include "fish.h"
#include "../game.h"
#include "../enemy.h"
#include "../PHL.h"
#include "../collision.h"
#include "../hero.h"
#include <stdlib.h>

void createFish(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Fish* f = malloc(sizeof *f);
			f->id = i;
			
			f->x = f->xstart = x;
			f->y = y;
			
			f->imageIndex = 0;
			
			f->spd = 1;
			
			f->turning = 0;
			f->dir = 1;
			if (dir == 1) {
				f->dir = -1;
				f->spd = -1;
			}
			
			f->mask.circle = f->mask.unused = 0;
			f->mask.x = x + 3;
			f->mask.y = y + 6;
			f->mask.w = 34;
			f->mask.h = 32;
			
			e->data = f;
			e->enemyStep = fishStep;
			e->enemyDraw = fishDraw;
			e->type = 13;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}	
	}
}

void fishStep(Fish* f)
{
	double fric = 0.02;
	
	f->x += f->spd;
	f->mask.x = f->x + 3;
	
	if (f->turning == 0) {
		f->imageIndex += 0.1;
		if (f->imageIndex >= 2) {
			f->imageIndex -= 2;
		}
	}else{
		f->imageIndex += 0.25;
		if (f->imageIndex >= 3) {
			f->turning = 0;
		}
	}
	
	if (f->dir == 1) {		
		if (f->x > f->xstart + 25) {
			f->spd -= fric;
			
			if (f->spd < 0) {
				f->dir = -1;
				f->turning = 1;
				f->imageIndex = 0;
			}
		}else{
			f->spd += fric;
			if (f->spd > 1) {
				f->spd = 1;
			}
		}
	}else if (f->dir == -1) {		
		if (f->x < f->xstart - 25) {
			f->spd += fric;
			
			if (f->spd > 0) {
				f->dir = 1;
				f->turning = 1;
				f->imageIndex = 0;
			}
		}else{
			f->spd -= fric;
			if (f->spd < -1) {
				f->spd = -1;
			}
		}
	}
	
	if (checkCollision(f->mask, getHeroMask())) {
		heroHit(15, f->x + 20);
	}
	
	//Weapon collision
	int i;
	for (i = 0; i < MAX_WEAPONS; i++) {
		if (weapons[i] != NULL) {
			if (checkCollision(f->mask, weapons[i]->weaponMask)) {
				weaponHit(weapons[i]);
				createEffect(2, f->x - 12, f->y - 12);
				spawnCollectable(f->x + 20, f->y);
				enemyDestroy(f->id);
				
				i = MAX_WEAPONS;
			}
		}	
	}
}

void fishDraw(Fish* f)
{
	int thisImage = 0;
	if (f->turning == 1) {
		if (f->dir == -1) {
			int animation[3] = {4, 6, 5};
			thisImage = animation[(int)f->imageIndex];
		}else{
			int animation[3] = {5, 6, 4};
			thisImage = animation[(int)f->imageIndex];
		}
	}else{
		thisImage = f->imageIndex;
		if (f->spd < 0) {
			thisImage += 2;
		}
	}
	
	PHL_DrawSurfacePart(f->x, f->y, 360 + (thisImage * 40), 360, 40, 40, images[imgEnemies]);
}