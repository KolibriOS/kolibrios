#include "slug.h"
#include "../enemy.h"
#include "../game.h"
#include "../PHL.h"
#include "../hero.h"
#include <stdlib.h>

void slugStep(Slug* s);
void slugDraw(Slug* s);

void createSlug(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Slug* s = malloc(sizeof *s);
			s->id = i;
			
			s->x = x;
			s->y = y;
			
			s->imageIndex = 0;			
			s->vsp = 0;
			
			s->dir = 1;
			if (dir == 1) {
				s->dir = -1;
			}			

			e->data = s;
			e->enemyStep = slugStep;
			e->enemyDraw = slugDraw;
			e->type = 2;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void slugStep(Slug* s)
{
	//Create Mask
	Mask mask;
	{
		mask.circle = 0;
		mask.unused = 0;
		mask.w = 32;
		mask.h = 24;
		mask.x = s->x + ((40 - mask.w) / 2);
		mask.y = s->y + (40 - mask.h);
	}
	
	//Animate
	{
		s->imageIndex += 0.1;
		if (s->imageIndex >= 4) {
			s->imageIndex -= 4;
		}
	}
	
	//Check if on ground
	int onground = 1;
	{
		mask.y += 1;
		if (checkTileCollision(1, mask) == 0 && checkTileCollision(3, mask) == 0) {
			onground = 0;
		}
		mask.y -= 1;
	}
	
	if (onground == 0) {
		double grav = 0.2;
		
		//Fall
		{
			s->y += s->vsp;
			s->vsp += grav;
		}
		
		//Land on ground
		{
			mask.y = mask.y + (40 - mask.h);
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			if (collide.x != -1) {
				s->y = collide.y - 40;
				s->vsp = 0;
			}
		}
	}else{
		//Check if on ledge
		{
			mask.x += mask.w * s->dir;
			mask.y += 1;
			
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			if (collide.x == -1) {
				s->dir *= -1;
			}
		}		
	}
	
	//Horizontal movement
	double hsp = 0.5;
	{
		s->x += s->dir * hsp;
	}
	
	//Check if hit a wall
	{
		mask.x = s->x + ((40 - mask.w) / 2);
		mask.y = s->y + (40 - mask.h);
		
		PHL_Rect collide = getTileCollision(1, mask);
		if (collide.x != -1) {
			s->dir *= -1;
		}
	}
	
	//Hit Player
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
				if (checkCollision(mask, weapons[i]->weaponMask) == 1) {				
					weaponHit(weapons[i]);
					
					createEffect(2, s->x - 12, s->y - 6);
					spawnCollectable(s->x + 20, s->y);
					enemyDestroy(s->id);
					
					i = MAX_WEAPONS;
				}
			}	
		}
	}
	
}

void slugDraw(Slug* s)
{	
	int anim[4] = { 1, 0, 2, 0 };
	
	int cropx = anim[(int)s->imageIndex] * 40;
	if (s->dir == -1) {
		cropx += 120;
	}
	
	PHL_DrawSurfacePart(s->x, s->y + 10, cropx, 40, 40, 40, images[imgEnemies]);
}