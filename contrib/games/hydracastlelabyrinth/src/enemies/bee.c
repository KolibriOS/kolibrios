#include "bee.h"
#include "../enemy.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>
#include <math.h>

void beeStep(Bee* b);
void beeDraw(Bee* b);

void createBee(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = /*(Enemy*)*/malloc(sizeof *e);
			Bee* b = /*(Bee*)*/malloc(sizeof *b);
			b->id = i;			
			
			b->x = x;
			b->y = y;
			b->xstart = b->x;
			b->ystart = b->y;
			
			b->hsp = 0;
			b->vsp = 0;
			
			b->timer = 0;
			b->imageIndex = 0;
			b->dir = 1;
			b->state = 0;
			
			b->hoverdir = 180;
			
			if (dir == 1) {
				b->hoverdir = 0;
				b->dir = -1;
			}		
			
			e->data = b;
			e->enemyStep = beeStep;
			e->enemyDraw = beeDraw;
			e->type = 24;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void beeStep(Bee* b)
{
	//Animate
	{
		b->imageIndex += 0.33;
		if (b->imageIndex >= 3) {
			b->imageIndex -= 3;
		}
	}
	
	//Mindless hovering
	if (b->state == 0)
	{
		b->hoverdir += 2.6;
		if (b->hoverdir >= 360) {
			b->hoverdir -= 360;
		}
		
		b->dir = 1;
		if (b->hoverdir <= 180) {
			b->dir = -1;
		}
		
		b->x = b->xstart + (20 * cos(b->hoverdir * 3.14159 /180));
		
		//If player is within range
		Mask area;
		area.unused = area.circle = 0;
		area.x = b->x - 80;
		area.y = b->y;
		area.w = 200;
		area.h = 100;
		
		if (checkCollision(area, getHeroMask())) {
			b->state = 1;
			
			b->dir = 1;
			if (b->x + 20 > herox) {
				b->dir = -1;
			}
			
			b->hsp = -5.5 * b->dir;
			
			PHL_PlaySound(sounds[sndBee01], CHN_ENEMIES);
		}
	}
	//Fly backwards
	else if (b->state == 1)
	{
		b->hsp += 0.25 * b->dir;
		
		if ((b->dir == 1 && b->hsp >= 0) || (b->dir == -1 && b->hsp <= 0)) {
			b->hsp = 0;
			b->state = 2;
			b->vsp = 3.75;
		}
	}
	//Fly downwards
	else if (b->state == 2)
	{
		b->vsp -= 0.1;
		if (b->vsp <= 0) {
			b->state = 3;
			b->vsp = 0;
			
			b->dir = 1;
			if (b->x + 20 > herox) {
				b->dir = -1;
			}
			b->hsp = 3 * b->dir;
		}
	}
	//Fly diaganal
	else if (b->state == 3)
	{
		b->vsp -= 0.1;
		
		if (b->vsp < -3) {
			b->vsp = -3;
		}

		if (b->y <= b->ystart) {
			b->state = 4;
			
			b->vsp = 0;
			b->y = b->ystart;
			
			if (b->x < b->xstart) {
				b->dir = 1;
			}else{
				b->dir = -1;
			}
			b->hsp = b->dir;
		}
	}
	//Fly back to start
	else if (b->state == 4)
	{
		if ((b->dir == 1 && b->x >= b->xstart) || (b->dir == -1 && b->x <= b->xstart)) {
			b->state = 0;
			b->hsp = 0;
			
			b->hoverdir = 0;
		}
	}
	
	//Movement
	{
		b->x += b->hsp;
		b->y += b->vsp;
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 24;
		mask.h = 32;
		mask.y = b->y + 6;
		mask.x = b->x + 14;
		if (b->dir == -1) {
			mask.x = b->x + 2;
		}
	}
	
	//Hit Player
	{
		if (checkCollision(mask, getHeroMask())) {
			heroHit(15, mask.x + (mask.w / 2));
		}
	}
	
	//Weapon Collision
	{
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (checkCollision(mask, weapons[i]->weaponMask)) {
					weaponHit(weapons[i]);
					
					createEffect(2, b->x - 12, b->y - 6);
					spawnCollectable(b->x + 20, b->y);
					enemyDestroy(b->id);
					
					i = MAX_WEAPONS;
				}
			}	
		}
	}
	
}

void beeDraw(Bee* b)
{
	int cropx = 280;
	
	if (b->dir == -1) {
		cropx += 120;
	}
	
	cropx += (int)b->imageIndex * 40;
	
	PHL_DrawSurfacePart(b->x, b->y, cropx, 480, 40, 40, images[imgEnemies]);
}