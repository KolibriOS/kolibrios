#include "jellyfish.h"
#include "../enemy.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>
#include <math.h>

void jellyfishStep(Jellyfish* j);
void jellyfishDraw(Jellyfish* j);

void createJellyfish(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Jellyfish* j = malloc(sizeof *j);
			j->id = i;
			
			j->x = x;
			j->y = j->ystart = y;
			j->ystart += 20;
			
			j->spd = 0;
			j->angle = 0;
			
			j->state = 0;
			j->imageIndex = 0;
			
			e->data = j;
			e->enemyStep = jellyfishStep;
			e->enemyDraw = jellyfishDraw;
			e->type = 20;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void jellyfishStep(Jellyfish* j)
{
	Mask mask;
	mask.unused = mask.circle = 0;
	mask.w = mask.h = 30;
	mask.x = j->x + 20 - (mask.w / 2);
	mask.y = j->y + 20 - (mask.h / 2);
	
	//Idle float
	if (j->state == 0)
	{
		//Animate
		j->imageIndex += 0.06;
		if (j->imageIndex >= 4) {
			j->imageIndex -= 4;
		}
		
		//Movement
		j->angle += 2.5;
		if (j->angle >= 360) { j->angle -= 360; }
		j->y = j->ystart + (20 * sin(j->angle * 3.14159 / 180));

		//Update mask
		mask.y = j->y + 20 - (mask.h / 2);
		
		//if player is close enough
		Mask area;
		area.unused = area.circle = 0;
		area.w = area.h = 160;
		area.x = j->x - 60;
		area.y = j->y - 60;
		
		if (checkCollision(area, getHeroMask()) == 1) {
			j->state = 1;
			j->spd = 0;
		}
	}
	//Attack
	if (j->state == 1)
	{
		//Setup
		if (j->spd == 0) {
			PHL_PlaySound(sounds[sndPi02], CHN_ENEMIES);
			j->spd = 3;
			
			//Move Right
			if (herox > j->x + 20) {
				//Move Up
				if (heroy < j->y) {
					j->angle = 135;
				}
				//Move Down
				else {
					j->angle = 45;
				}
			}
			//Move Left
			else{
				//Move Up
				if (heroy < j->y) {
					j->angle = 225;
				}
				//Move Down
				else {
					j->angle = 315;
				}
			}
		}
		
		//Movement
		j->x += (j->spd) * sin(j->angle * 3.14159 / 180);
		j->y += (j->spd) * cos(j->angle * 3.14159 / 180);
		
		//Slow down
		j->spd -= 0.075;
		if (j->spd <= 0) {
			j->spd = 0;
			j->state = 2;
		}
	}
	//Stablize
	if (j->state == 2)
	{
		//Setup
		if (j->spd == 0) {
			j->spd = 1;
			j->ystart = j->y - 20;
			j->angle = 80;
		}
		
		//Movement
		j->angle += 2.5;
		if (j->angle >= 360) { j->angle -= 360; }
		j->y = j->ystart + (20 * sin(j->angle * 3.14159 / 180));
		
		
		if (j->angle >= 180) {
			j->state = 0;
			j->ystart = j->y - 20;
			j->angle = 100;
		}
	}
	
	//Update Mask
	mask.x = j->x + 20 - (mask.w / 2);
	mask.y = j->y + 20 - (mask.h / 2);
	
	//Collide with hero
	if (checkCollision(mask, getHeroMask())) {
		heroHit(15, j->x + 20);
	}
	
	//Sword collision
	int i;
	for (i = 0; i < MAX_WEAPONS; i++) {
		if (weapons[i] != NULL) {
			if (checkCollision(mask, weapons[i]->weaponMask)) {
				spawnCollectable(j->x + 20, j->y);				
				weaponHit(weapons[i]);
				
				createEffect(2, j->x - 12, j->y - 12);
				enemyDestroy(j->id);
				
				i = MAX_WEAPONS;
			}
		}	
	}
}

void jellyfishDraw(Jellyfish* j)
{
	int frame = 0;
	
	//if (j->state == 0) {
		int animation[4] = { 0, 1, 0, 2};
		frame = animation[(int)j->imageIndex];
	//}
	
	if (j->state == 1) {
		if (j->angle == 135) {
			frame = 3;
		}
		else if (j->angle == 225) {
			frame = 4;
		}
		else if (j->angle == 315) {
			frame = 5;
		}
		else {
			frame = 6;
		}
	}
	
	PHL_DrawSurfacePart(j->x, j->y, frame * 40, 520, 40, 40, images[imgEnemies]);
}