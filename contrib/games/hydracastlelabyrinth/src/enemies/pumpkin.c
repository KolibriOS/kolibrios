#include "pumpkin.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>

void pumpkinenemyStep(Pumpkinenemy* p);
void pumpkinenemyDraw(Pumpkinenemy* p);

void pumpkinheadStep(Pumpkinhead* p);
void pumpkinheadDraw(Pumpkinhead* p);

void createPumpkinenemy(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Pumpkinenemy* p = malloc(sizeof *p);
			
			p->id = i;
			
			p->hp = 3;
			p->blink = 0;
			
			p->x = x;
			p->y = y;
			
			p->dir = 1;
			if (herox < p->x + 20) {
				p->dir = -1;
			}
			
			p->imageIndex = 0;
			
			p->state = 0;
			p->timer = 0;
			
			e->data = p;
			e->enemyStep = pumpkinenemyStep;
			e->enemyDraw = pumpkinenemyDraw;
			e->type = 32;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void pumpkinenemyStep(Pumpkinenemy* p)
{
	//Setup Mask
	Mask mask;
	{
		mask.unused = mask.circle = 0;
		mask.w = 20;
		mask.h = 38;
		mask.x = p->x + ((40 - mask.w) / 2);
		mask.y = p->y + (40 - mask.h);
	}
	
	//Animate
	{
		p->imageIndex += 0.1;
		if (p->imageIndex >= 2) {
			p->imageIndex -= 2;
		}
		
		if (p->blink > 0) {
			p->blink -= 1;
		}
	}
	
	//Walking
	if (p->state == 0)
	{
		double hsp = 0.5;		
		p->x += hsp * p->dir;
		mask.x = p->x + ((40 - mask.w) / 2);
		
		//Hit wall
		{			
			if (checkTileCollision(1, mask) == 1 || mask.x > 640 || mask.x + mask.w < 0) {
				p->dir *= -1;
			}
		}
		
		//On edge
		{
			mask.x += mask.w * p->dir;
			mask.y += 20;
			
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			
			if (collide.x == -1) {
				p->dir *= -1;
			}
		}
		
		//Player is close
		{
			if (p->timer <= 0) {
				Mask area;
				{
					area.circle = area.unused = 0;
					area.w = 240;
					area.h = 80;
					area.x = p->x - 100;
					area.y = p->y - 40;
				}
				if (checkCollision(area, getHeroMask()) == 1) {
					p->state = 1;
					p->timer = 0;
					p->dir = 1;
					if (herox < p->x + 20) {
						p->dir = -1;
					}
				}
				
			}else{
				p->timer -= 1;
			}
		}
		
	}
	
	//Deheaded
	else if (p->state == 1) {
		//Animate
		{
			p->imageIndex = 0;
			if (p->timer >= 15) {
				p->imageIndex = 2;
			}
		}
		
		p->timer += 1;
		if (p->timer == 15) {
			createPumpkinhead(p->x, p->y - 6, p->dir);
			PHL_PlaySound(sounds[sndPi05], CHN_ENEMIES);
		}
		
		if (p->timer >= 40) {
			p->state = 0;
			p->imageIndex = 0;
			p->timer = 300;
		}
	}
	
	//Update Mask
	mask.x = p->x + ((40 - mask.w) / 2);
	mask.y = p->y + (40 - mask.h);
	
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
						
						p->hp -= 1;
						p->blink = 15;
						
						//Death
						if (p->hp <= 0) {					
							createEffect(2, p->x - 12, p->y - 6);
							spawnCollectable(p->x + 20, p->y);
							enemyDestroy(p->id);
						}

						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}
	
}

void pumpkinenemyDraw(Pumpkinenemy* p)
{
	if (p->blink % 2 == 0) {
		int cropX = (int)p->imageIndex * 40;
		
		if (p->dir == -1) {
			cropX += 120;
		}
		
		PHL_DrawSurfacePart(p->x, p->y, cropX, 560, 40, 40, images[imgEnemies]);
	}
}


//Pumpkin bomb head
void createPumpkinhead(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Pumpkinhead* p = malloc(sizeof *p);
			p->id = i;
			
			p->dir = dir;
			
			p->x = x;
			p->y = y;
			
			p->vsp = -2;
			
			p->imageIndex = 0;

			p->state = 0;
			p->timer = 0;
			
			e->data = p;
			e->enemyStep = pumpkinheadStep;
			e->enemyDraw = pumpkinheadDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void pumpkinheadStep(Pumpkinhead* p)
{
	char dead = 0;
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 20;
		mask.h = 22;
		mask.x = p->x + ((40 - mask.w) / 2);
		mask.y = p->y + ((40 - mask.h) / 2);
	}
	
	//Pumpkin head
	if (p->state == 0)
	{
		char explode = 0;
		
		//Animate
		{
			p->imageIndex += 0.1;
			if (p->imageIndex >= 2) {
				p->imageIndex -= 2;
			}
		}
			
		//Movement
		{
			int hsp = 3;
			p->x += hsp * p->dir;
			mask.x = p->x + ((40 - mask.w) / 2);
			
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x != -1) {
				p->x = collide.x + 20 - ((20 + (mask.w / 2)) * p->dir) - 20;
				mask.x = p->x + ((40 - mask.w) / 2);
				p->dir *= -1;
			}
			
			double grav = 0.15;
			p->y += p->vsp;
			p->vsp += grav;
			mask.y = p->y + ((40 - mask.h) / 2);
			
			collide = getTileCollision(1, mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			
			if (collide.x != -1) {
				p->y = collide.y - 40;
				explode = 1;
			}
		}
		
		//Update Mask
		mask.x = p->x + ((40 - mask.w) / 2);
		mask.y = p->y + ((40 - mask.h) / 2);
	
		//Explode
		{
			if (explode == 1) {
				PHL_PlaySound(sounds[sndBom03], CHN_ENEMIES);
				p->state = 1;
				p->imageIndex = 0;
				p->timer = 0;
			}
		}
		
		//Outside of room
		{
			if (mask.y > 480 || mask.x > 640 || mask.x + mask.w < 0) {
				dead = 1;
			}
		}
		
	}
	
	//Explosion
	else if (p->state == 1)
	{
		//Update Mask
		{
			mask.w = 68;
			mask.h = 66;
			mask.x = p->x - 44 + 64 - (mask.w / 2);
			mask.y = p->y - 44 + (84 - mask.h);
		}
		
		//Animate
		{
			p->imageIndex += 0.33;
			if (p->imageIndex >= 12) {
				dead = 1;
			}
		}
		
		//Hero Collision
		{
			if (checkCollision(mask, getHeroMask()) == 1) {
				heroHit(40, mask.x + (mask.w / 2));
			}
		}
	}
	
	//Destroy object
	{
		if (dead == 1) {
			enemyDestroy(p->id);
		}
	}
}

void pumpkinheadDraw(Pumpkinhead* p)
{
	if (p->state == 0) {
		int cropX = (int)p->imageIndex * 40;
			
		if (p->dir == -1) {
			cropX += 80;
		}
		
		PHL_DrawSurfacePart(p->x, p->y, cropX, 240, 40, 40, images[imgEnemies]);
	}
	
	if (p->state == 1) {
		int cropX = (int)p->imageIndex * 128;
		int cropY = 0;
			
		while (cropX >= 640) {
			cropX -= 640;
			cropY += 96;
		}
		
		PHL_DrawSurfacePart(p->x - 44, p->y - 44, cropX, cropY, 128, 96, images[imgExplosion]);
	}
}