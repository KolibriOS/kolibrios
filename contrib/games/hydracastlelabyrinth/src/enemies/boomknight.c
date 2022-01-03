#include "boomknight.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>

void boomknightStep(Boomknight* b);
void boomknightDraw(Boomknight* b);

void boomStep(Boom* b);
void boomDraw(Boom* b);

void createBoomknight(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Boomknight* b = malloc(sizeof *b);
			b->id = i;
			
			b->hp = 2;
			b->blink = 0;
			
			b->x = x;
			b->y = y;
			
			b->dir = 1;
			if (herox < b->x + 20) {
				b->dir = -1;
			}
			
			b->imageIndex = 0;
			
			b->state = 0;
			b->timer = 0;
			
			e->data = b;
			e->enemyStep = boomknightStep;
			e->enemyDraw = boomknightDraw;
			e->type = 31;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void boomknightStep(Boomknight* b)
{
	//Animate
	{
		b->imageIndex += 0.1;
		if (b->imageIndex >= 2) {
			b->imageIndex -= 2;
		}
		
		if (b->blink > 0) {
			b->blink -= 1;
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 30;
		mask.h = 32;
		mask.x = b->x + ((40 - mask.w) / 2);
		mask.y = b->y + (40 - mask.h);
	}
	
	//Walk
	if (b->state == 0) {
		//Movement
		{
			double hsp = 0.5;		
			b->x += hsp * b->dir;
			mask.x = b->x + ((40 - mask.w) / 2);
		}
		
		//Hit wall
		{			
			if (checkTileCollision(1, mask) == 1) {
				b->dir *= -1;
			}
		}
		
		//On edge
		{
			mask.x += mask.w * b->dir;
			mask.y += 20;
			
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			
			if (collide.x == -1) {
				b->dir *= -1;
			}
		}
		
		//Player is close
		{
			if (b->timer <= 0) {
				Mask area;
				{
					area.circle = area.unused = 0;
					area.w = 120;
					area.h = 40;
					area.x = b->x + 20;
					if (b->dir == -1) {
						area.x -= area.w;
					}
					area.y = b->y;
				}
				if (checkCollision(area, getHeroMask()) == 1) {
					b->state = 1;
					b->timer = 0;
				}
				
			}else{
				b->timer -= 1;
			}
		}
		
	}
	
	//Throw
	else if (b->state == 1) {
		//Animate
		{
			b->imageIndex = 0;
			if (b->timer >= 15) {
				b->imageIndex = 2;
			}
		}
		
		b->timer += 1;
		if (b->timer == 15) {
			createBoom(b->x, b->y, b->dir);
			PHL_PlaySound(sounds[sndPi05], CHN_ENEMIES);
		}
		
		if (b->timer >= 110) {
			b->state = 0;
			b->imageIndex = 0;
			b->timer = 120;
		}
	}
	
	//Update Mask
	mask.x = b->x + ((40 - mask.w) / 2);
	mask.y = b->y + (40 - mask.h);
	
	//Hero Collision
	{
		if (checkCollision(mask, getHeroMask()) == 1) {
			heroHit(15, mask.x + (mask.w / 2));
		}
	}
	
	//Weapon Collision
	{
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (weapons[i]->cooldown == 0) {
					if (checkCollision(mask, weapons[i]->weaponMask)) {
						weaponHit(weapons[i]);
						
						b->hp -= 1;
						b->blink = 15;
						
						//Death
						if (b->hp <= 0) {					
							createEffect(2, b->x - 12, b->y - 6);
							spawnCollectable(b->x + 20, b->y);
							enemyDestroy(b->id);
						}

						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}
	
}

void boomknightDraw(Boomknight* b)
{
	if (b->blink % 2 == 0) {
		int cropX = 400 + ((int)b->imageIndex * 40);
		
		if (b->dir == -1) {
			cropX += 120;
		}
		
		PHL_DrawSurfacePart(b->x, b->y, cropX, 400, 40, 40, images[imgEnemies]);
	}
}


//Enemy boomerang
void createBoom(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Boom* b = malloc(sizeof *b);
			b->id = i;
			
			b->dir = dir;
			b->x = x;
			b->y = y;
			
			b->hsp = 6 * b->dir;
			b->imageIndex = 0;
			
			b->timer = 90;
			
			e->data = b;
			e->enemyStep = boomStep;
			e->enemyDraw = boomDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void boomStep(Boom* b)
{
	//Animate
	{
		b->imageIndex += 0.33;
		if (b->imageIndex >= 8) {
			b->imageIndex -= 8;
		}
	}
	
	//Movement
	{
		b->x += b->hsp;
		
		double fric = 0.125;
		b->hsp -= fric * b->dir;
	}
	
	//Hero collision
	{
		Mask mask;
		{
			mask.circle = mask.unused = 0;
			mask.w = 24;
			mask.h = 24;
			mask.x = b->x + ((40 - mask.w) / 2);
			mask.y = b->y + ((40 - mask.h) / 2);
		}
		
		if (checkCollision(mask, getHeroMask()) == 1) {
			heroHit(10, mask.x + (mask.w / 2));
		}
	}
	
	b->timer -= 1;
	if (b->timer <= 0) {
		createEffectExtra(5, b->x + 20, b->y + 20, 0, 0, 0);
		enemyDestroy(b->id);
	}
}

void boomDraw(Boom* b)
{
	int cropX = (int)b->imageIndex * 40;
	
	if (b->dir == -1) {
		cropX += 320;
	}
	
	PHL_DrawSurfacePart(b->x, b->y, cropX, 360, 40, 40, images[imgMisc20]);
}