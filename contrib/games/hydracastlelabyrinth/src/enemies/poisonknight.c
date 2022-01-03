#include "poisonknight.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>

void poisonknightStep(Poisonknight* p);
void poisonknightDraw(Poisonknight* p);

void goopStep(Goop* g);
void goopDraw(Goop* g);

void createPoisonknight(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Poisonknight* p = malloc(sizeof *p);
			p->id = i;
			p->hp = 2;
			
			p->x = x;
			p->y = y;
			
			p->imageIndex = 0;
			
			p->dir = 1;
			if (herox < p->x) {
				p->dir = -1;
			}
			
			p->blink = 0;
			p->timer = 0;
			p->state = 0;
			
			e->data = p;
			e->enemyStep = poisonknightStep;
			e->enemyDraw = poisonknightDraw;
			e->type = 29;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void poisonknightStep(Poisonknight* p)
{
	char dead = 0;
	
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
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 38;
		mask.h = 36;
		mask.x = p->x + ((40 - mask.w) / 2);
		mask.y = p->y + (40 - mask.h);
	}
	
	//Walk
	if (p->state == 0) {
		double hsp = 1;
		
		p->x += hsp * p->dir;
		mask.x = p->x + ((40 - mask.w) / 2);
		
		//Hit wall
		if (checkTileCollision(1, mask) == 1) {
			p->dir *= -1;
		}
		
		//On wall edge
		else {
			mask.x += mask.w * p->dir;
			mask.y += 10;
			
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			if (collide.x == -1) {
				p->dir *= -1;
			}
		}
		
		//Hero is close enough
		{
			if (p->timer <= 0) {
				Mask area;
				area.circle = area.unused = 0;
				area.x = p->x - 110;
				area.y = p->y;
				area.w = 260;
				area.h = 40;
				
				if (checkCollision(area, getHeroMask()) == 1) {
					p->dir = 1;
					if (herox < p->x + 20) {
						p->dir = -1;
					}
					p->imageIndex = 1;
					p->timer = 0;
					p->state = 1;
				}
				
			}else{
				p->timer -= 1;
			}
		}
	}
	
	//*beat*
	else if (p->state == 1)
	{
		//Animate
		p->imageIndex = 1;
		
		p->timer += 1;
		if (p->timer >= 15) {
			p->state = 2;
			p->timer = 0;
			p->imageIndex = 2;
		}
	}
	
	//Shoot goop
	else if (p->state == 2)
	{
		//Shoot goop
		if (p->timer == 0) {
			PHL_PlaySound(sounds[sndPi05], CHN_ENEMIES);
			createGoop(p->x + (20 * p->dir), p->y - 2, p->dir);
		}
		
		//Animate
		p->imageIndex = 2;
		
		p->timer += 1;
		if (p->timer >= 25) {
			p->state = 0;
			p->timer = 240;
		}
	}
	
	//Update Mask
	mask.x = p->x + ((40 - mask.w) / 2);
	mask.y = p->y + (40 - mask.h);
	
	
	//Collide with hero
	{
		if (checkCollision(mask, getHeroMask()) == 1) {
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
						
						p->hp -= 1;
						p->blink = 15;

						if (p->hp <= 0) {
							dead = 1;
							createEffect(2, p->x - 12, p->y - 6);
							spawnCollectable(p->x + 20, p->y);
						}
					}
				}
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

void poisonknightDraw(Poisonknight* p)
{
	if (p->blink % 2 == 0) {
		int cropX = (int)p->imageIndex * 40;
		
		if (p->dir == -1) {
			cropX += 120;
		}
		
		PHL_DrawSurfacePart(p->x, p->y, cropX, 280, 40, 40, images[imgEnemies]);
	}
}

//Poison Goop
void createGoop(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Goop* g = malloc(sizeof *g);
			g->id = i;
			
			g->x = x;
			g->y = y;
			
			g->dir = dir;
			
			g->imageIndex = 0;
			
			e->data = g;
			e->enemyStep = goopStep;
			e->enemyDraw = goopDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void goopStep(Goop* g)
{
	char dead = 0;
	
	//Animate
	{
		g->imageIndex += 0.33;
		if (g->imageIndex >= 3) {
			g->imageIndex -= 3;
		}
	}
	
	//Movement
	{
		int hsp = 4;
		g->x += hsp * g->dir; 
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 22;
		mask.h = 22;
		mask.x = g->x + ((40 - mask.w) / 2);
		mask.y = g->y + ((40 - mask.h) / 2);
	}
	
	//Collide with hero
	{
		//Collide with shield
		if (checkCollision(mask, shieldMask) == 1) {
			dead = 1;
			PHL_PlaySound(sounds[sndHit07], CHN_EFFECTS);
			createEffect(1, g->x, g->y);			
		}
		//Collide with hero
		else if (checkCollision(mask, getHeroMask()) == 1) {
			if (heroHit(10, mask.x + (mask.w / 2)) == 1) {
				heroPoison();
			}
		}
	}
	
	//Collide with wall
	{
		if (checkTileCollision(1, mask) == 1) {
			dead = 1;
			createEffect(1, g->x, g->y);
		}
	}
	
	//Destroy if out of room
	{
		if (g->x + 40 < 0 || g->x > 640) {
			dead = 1;
		}
	}
	
	//Destroy object
	{
		if (dead == 1) {
			enemyDestroy(g->id);
		}
	}
}

void goopDraw(Goop* g)
{
	int cropX = 400 + ((int)g->imageIndex * 40);
	
	if (g->dir == -1) {
		cropX += 120;
	}
	
	PHL_DrawSurfacePart(g->x, g->y, cropX, 520, 40, 40, images[imgMisc20]);
}