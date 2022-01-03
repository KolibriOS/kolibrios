#include "seal.h"
#include "../game.h"
#include "../enemy.h"
#include "../collision.h"
#include "../hero.h"
#include <stdlib.h>

void sealStep(Seal* s);
void sealDraw(Seal* s);

void createSeal(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Seal* s = malloc(sizeof *s);
			s->id = i;
			s->hp = 2;
			
			s->x = x;
			s->y = y;
			
			s->imageIndex = 0;
			
			s->dir = 1;
			if (x + 20 > herox) {
				s->dir = -1;
			}
			
			s->state = 0;
			s->timer = 0;
			
			s->invincible = 0;
			
			e->data = s;
			e->enemyStep = sealStep;
			e->enemyDraw = sealDraw;
			e->type = 19;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void sealStep(Seal* s)
{
	if (s->invincible > 0) {
		s->invincible -= 1;
	}
	
	Mask mask;
	mask.unused = mask.circle = 0;
	mask.w = mask.h = 28;
	mask.x = s->x + ((40 - mask.w) / 2);
	mask.y = s->y + (40 - mask.h);
	
	//Walk
	if (s->state == 0)
	{
		//Animate
		s->imageIndex += 0.1;
		if (s->imageIndex >= 2) {
			s->imageIndex -= 2;
		}		
		
		//Check if hit a wall
		if (checkTileCollision(1, mask) == 1) {
			s->dir *= -1;
		}else{
			//Check if on edge
			mask.x += mask.w * s->dir;
			mask.y += mask.h;
			
			PHL_Rect collide = getTileCollision(1, mask);	
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			if (collide.x == -1) {
				s->dir *= -1;
			}
			
			mask.x = s->x + ((40 - mask.w) / 2);
			mask.y = s->y + (40 - mask.h);
		}
		
		//Movement
		s->x += 0.5 * s->dir;
		
		if (s->timer <= 0) {
			//Check if player is close enough
			Mask area;
			area.unused = area.circle = 0;
			area.x = s->x - 40;
			area.y = s->y;
			area.w = 120;
			area.h = 120;
			
			if (checkCollision(area, getHeroMask()) == 1) {
				s->state = 1;
				s->timer = -1;
			}
		}else{
			s->timer -= 1;
		}
	}
	
	//Rear back
	else if (s->state == 1)
	{
		//Setup
		if (s->timer == -1) {
			s->imageIndex = 4;
			s->timer = 20;
		}
		
		s->timer -= 1;
		
		if (s->timer <= 0) {
			s->state = 2;
			s->timer = -1;
			s->imageIndex = 0;
		}
	}
	
	//Tounge attack
	else if (s->state == 2)
	{
		//Setup
		if (s->timer == -1) {
			s->timer = 0;
			PHL_PlaySound(sounds[sndGet01], CHN_ENEMIES);
		}
		
		//Animate
		int animation[41] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 4,
						     4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
						     4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 
						     2, 2, 1, 1, 0, 0, 5, 5, 5, 5, 5 };
							 
		s->imageIndex = animation[(int)s->timer];
		
		//Update mask height to fit tounge
		int len[6] = { 18, 38, 58, 64, 66, 0};
		mask.h += len[(int)s->imageIndex];
		
		s->timer += 1;
		
		if (s->timer >= 41) {
			s->state = 0;
			s->timer = 120;
			s->imageIndex = 0;
		}
	}
	
	//Hit Player
	if (checkCollision(mask, getHeroMask())) {
		heroHit(10, s->x + 20);
	}
	
	int i;
	for (i = 0; i < MAX_WEAPONS; i++) {
		if (weapons[i] != NULL) {
			if (weapons[i]->cooldown == 0) {
				if (checkCollision(mask, weapons[i]->weaponMask)) {
					s->hp -= 1;
					s->invincible = 15;
					weaponHit(weapons[i]);
					if (s->hp <= 0) {
						enemyDestroy(s->id);
						createEffect(2, s->x - 12, s->y - 6);
						spawnCollectable(s->x + 20, s->y);
					}
					i = MAX_WEAPONS;
				}
			}
		}	
	}
}

void sealDraw(Seal* s)
{
	if (s->invincible % 2 == 0) {
		int cx = 400 + ((int)s->imageIndex * 40);
		
		if (s->state == 0) {
			if (s->dir == -1) {
				cx += 80;
			}
		}
		
		if (s->state == 2) {
			cx = 600;
		}
		
		PHL_DrawSurfacePart(s->x, s->y, cx, 200, 40, 40, images[imgEnemies]);
		
		//Draw tounge
		if (s->state == 2) {
			PHL_DrawSurfacePart(s->x, s->y + 28, 200 + ((int)s->imageIndex * 40), 0, 40, 80, images[imgMisc2040]);
		}
	}
}