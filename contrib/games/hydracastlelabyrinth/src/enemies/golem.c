#include "golem.h"
#include "../PHL.h"
#include "../hero.h"
#include "../game.h"
#include <stdlib.h>

void golemStep(Golem* g);
void golemDraw(Golem* g);

void createGolem(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Golem* g = malloc(sizeof *g);
			g->id = i;
			
			g->x = x;
			g->y = y;
			
			g->hp = 4;
			
			g->dir = 1;
			if (dir == 1) {
				g->dir = -1;
			}
			
			g->imageIndex = 0;
			g->state = 0;
			g->blink = 0;			
			
			e->data = g;
			e->enemyStep = golemStep;
			e->enemyDraw = golemDraw;
			e->type = 28;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void golemStep(Golem* g)
{
	double imageSpeed = 0.2;
	
	//Timers
	{
		if (g->blink > 0) {
			g->blink -= 1;
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.unused = mask.circle = 0;
		mask.w = 36;
		mask.h = 36;
		mask.x = g->x + ((40 - mask.w) / 2);
		mask.y = g->y + (40 - mask.h);
	}
	
	//Rolling
	if (g->state == 0)
	{
		//Animate
		{
			g->imageIndex += imageSpeed * g->dir;
			
			if (g->imageIndex >= 8) {
				g->imageIndex -= 8;
			}
			
			if (g->imageIndex < 0) {
				g->imageIndex += 8;
			}
		}
		
		//Movement
		double hsp = 1;
		{			
			g->x += hsp * g->dir;
			mask.x = g->x + ((40 - mask.w) / 2);
		}
		
		char nextState = 0;
		
		//Check on ledge
		{
			mask.x += 30 * g->dir;
			mask.y += 10;
			
			if (checkTileCollision(1, mask) == 0 && checkTileCollision(3, mask) == 0) {
				nextState = 1;
			}
			
			mask.x = g->x + ((40 - mask.w) / 2);
			mask.y = g->y + (40 - mask.h);
		}
		
		//Collide with wall
		{
			mask.x += hsp * g->dir;
			
			if (checkTileCollision(1, mask) == 1) {
				nextState = 1;
			}
			
			mask.x = g->x + ((40 - mask.w) / 2);
		}
		
		if (nextState == 1) {
			PHL_PlaySound(sounds[sndPi10], CHN_ENEMIES);
			g->state = 1;
			g->imageIndex = 0;
		}
	}
	
	//Forming
	else if (g->state == 1)
	{
		//Animate
		{
			g->imageIndex += imageSpeed;
			
			if (g->imageIndex >= 12) {
				g->imageIndex = 0;
				g->state = 0;
				g->dir *= -1;
			}
		}
		
	}
	
	//Hero Collision
	{
		if (checkCollision(mask, getHeroMask())) {
			heroHit(15, mask.x + (mask.w / 2));
		}
	}
	
	//Weapon collision
	{
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (weapons[i]->cooldown == 0) {
					if (checkCollision(mask, weapons[i]->weaponMask)) {
						weaponHit(weapons[i]);
						
						//Tink
						if (g->state == 0) {
							PHL_PlaySound(sounds[sndHit03], CHN_WEAPONS);
						}else{
							g->hp -= 1;
							g->blink = 15;
						}
						
						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}	

	//Death
	{
		if (g->hp <= 0) {
			createRockSmash(mask.x + (mask.w / 2), mask.y + (mask.h / 2));
			spawnCollectable(g->x + 20, g->y);
			enemyDestroy(g->id);
		}
	}
}

void golemDraw(Golem* g)
{
	if (g->blink % 2 == 0) {
		int cropX = 320,
			cropY = 160;
			
		int drawY = g->y;
			
		if (g->state == 0) {
			cropX += (int)g->imageIndex * 40;
			drawY += 2;
		}else{
			cropY = 280;
			cropX = 240;
			
			int animation[12] = {0, 1, 2, 3, 3, 3, 3, 3, 3, 2, 1, 0};
			cropX += animation[(int)g->imageIndex] * 40;
		}
		
		PHL_DrawSurfacePart(g->x, drawY, cropX, cropY, 40, 40, images[imgEnemies]);
	}
}