#include "gas.h"
#include "../PHL.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>

void gasStep(Gas* g);
void gasDraw(Gas* g);

void createGas(int x, int y, int temp)
{
	if (temp == 0 || hasKey[7] == 0) {
		int i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i] == NULL) {
				Enemy* e = malloc(sizeof *e);
				Gas* g = malloc(sizeof *g);
				g->id = i;
				
				g->x = x;
				g->y = y;
				
				g->state = 0;
				g->timer = 0;
				g->imageIndex = 0;
				
				/*
				g->mask.unused = g->mask.circle = 0;
				g->mask.w = g->mask.h = 24;
				g->mask.x = x + 20 - (g->mask.w / 2);
				g->mask.y = y + 40 - g->mask.h;
				*/
				
				e->data = g;
				e->enemyStep = gasStep;
				e->enemyDraw = gasDraw;
				e->type = -1;
				
				enemies[i] = e;
				i = MAX_ENEMIES;
			}
		}
	}
}

void gasStep(Gas* g)
{	
	if (g->state != 0) {
		g->imageIndex += 0.2;
	}
	
	if (g->state == 0) { //Wait
		Mask tempMask;
		tempMask.circle = tempMask.unused = 0;
		tempMask.x = g->x - 100;
		tempMask.y = g->y - 20;
		tempMask.w = 240;
		tempMask.h = 60;
		
		if (checkCollisionXY(tempMask, herox, heroy + 20)) {
			g->state = 1;
			g->imageIndex = 3;
			g->timer = 32;
			PHL_PlaySound(sounds[sndGas01], CHN_ENEMIES);
		}
	}
	else if (g->state == 1 || g->state == 3) { //Small puff		
		if (g->imageIndex >= 5) {
			g->imageIndex -= 2;
		}
		
		g->timer -= 1;
		if (g->timer <= 0) {
			if (g->state == 3) {
				g->state = 0;
			}else{
				g->state = 2;
				g->imageIndex = 0;
				g->timer = 175;
			}
		}
	}
	else if (g->state == 2) { //Big puff
		if (g->imageIndex >= 3) {
			g->imageIndex -= 3;
		}
		
		g->timer -= 1;
		if (g->timer <= 0) {
			g->state = 3;
			g->timer = 120;
			g->imageIndex = 3;
		}
		
		if (hasItem[7] != 1) { //Does not have gas mask
			Mask mask;
			mask.unused = mask.circle = 0;
			mask.w = mask.h = 24;
			mask.x = g->x + 20 - (mask.w / 2);
			mask.y = g->y + 40 - mask.h;
		
			if (checkCollision(getHeroMask(), mask)) {
				if (heroHit(15, g->x + 20)) {
					heroPoison();
				}
			}
		}
	}
}

void gasDraw(Gas* g)
{
	if (g->state != 0) {
		PHL_DrawSurfacePart(g->x, g->y, (int)g->imageIndex * 40, 400, 40, 40, images[imgEnemies]);
	}
}