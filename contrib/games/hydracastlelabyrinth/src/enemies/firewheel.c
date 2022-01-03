#include "firewheel.h"
#include "../game.h"
#include "../PHL.h"
#include "../hero.h"
#include <stdlib.h>

void firewheelRotate(Firewheel* f, int clockwise);

void firewheelStep(Firewheel* f);
void firewheelDraw(Firewheel* f);

void createFirewheel(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Firewheel* f = malloc(sizeof *f);
			
			f->id = i;
			
			f->x = x;
			f->y = y;
			
			f->imageIndex = 0;
			
			f->hp = 2;
			f->blink = 0;

			f->hsp = 1;
			f->vsp = 0;
			
			f->wallx = 0;
			f->wally = 1;
			
			f->timer = 0;
			if (x % 40 != 0) {
				f->timer = 20;
			}
			
			//Start on ceiling
			{				
				Mask mask;
				mask.circle = mask.unused = 0;
				mask.w = 40;
				mask.h = 40;
				mask.x = f->x;
				mask.y = f->y + 10;
				
				PHL_Rect collide = getTileCollision(1, mask);
				if (collide.x == -1) {
					collide = getTileCollision(3, mask);
				}
				
				if (collide.x == -1) {
					f->wally = -1;
					f->hsp *= -1;
				}
			}
			
			f->dir = 1;
			if (dir == 1) {
				f->hsp *= -1;
				f->dir = -1;
			}
			
			e->data = f;
			e->enemyStep = firewheelStep;
			e->enemyDraw = firewheelDraw;
			e->type = 27;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void firewheelStep(Firewheel* f)
{			
	//Animate
	{
		f->imageIndex += 0.33;
		if (f->imageIndex >= 4) {
			f->imageIndex -= 4;
		}
		
		if (f->blink > 0) {
			f->blink -= 1;
		}
	}
	
	//Movement
	int spd = 2;
	f->x += spd * f->hsp;
	f->y += spd * f->vsp;
	
	//Setup mask
	Mask mask;
	mask.circle = mask.unused = 0;
	mask.w = 40;
	mask.h = 40;
	mask.x = f->x;
	mask.y = f->y;
	
	//Check if ready to change angle
	if ( (f->hsp != 0 && (int)f->x % 20 == 0) || (f->vsp != 0 && (int)f->y % 20 == 0) )
	{
		int doCheck = 1;
		while (doCheck == 1) {
			doCheck = 0;
			
			//Check on edge
			mask.x += (f->wallx * 10);
			mask.y += (f->wally * 10);
			
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			
			//Outside of room
			if (f->y <= -40) {
				collide.x = 1;
			}
			
			//On edge
			if (collide.x == -1) {
				int tempHsp = f->hsp;
				int tempVsp = f->vsp;
				f->hsp = f->wallx;
				f->vsp = f->wally;
				
				f->wallx = -tempHsp;
				f->wally = -tempVsp;
				doCheck = 1;
			}
			//Hit wall
			else {
				mask.x = f->x;
				mask.y = f->y;
				mask.x += f->hsp * 10;
				mask.y += f->vsp * 10;
				
				collide = getTileCollision(1, mask);
				if (collide.x == -1) {
					collide = getTileCollision(3, mask);
				}
				
				//Outside of room
				if (collide.x == -1) {
					if (f->y <= -40 && f->vsp != 1) {
						collide.x = 1;
					}
				}
				
				//Did collide with wall
				if (collide.x != -1) {
					int tempWallx = f->wallx;
					int tempWally = f->wally;
					f->wallx = f->hsp;
					f->wally = f->vsp;
					
					f->hsp = -tempWallx;
					f->vsp = -tempWally;
					
					doCheck = 1;
				}
				
			}
		}
		
		/*
		mask.x += f->hsp * 10;
		mask.y += f->vsp * 10;
		
		//Collide with wall
		PHL_Rect collide = getTileCollision(1, mask);
		if (collide.x == -1) {
			collide = getTileCollision(3, mask);
		}
		
		//Outside of room
		if (collide.x == -1) {
			if (mask.y <= 0 && f->vsp < 0) {
				collide.x = f->x;
				collide.y = -40;
				collide.w = 40;
				collide.h = 40;
			}
		}
		
		//Did collide with wall
		if (collide.x != -1) {
			int tempWallx = f->wallx;
			int tempWally = f->wally;
			f->wallx = f->hsp;
			f->wally = f->vsp;

			f->hsp = -tempWallx;
			f->vsp = -tempWally;
		}
		//Edge rotate
		else{
			mask.x = f->x;
			mask.y = f->y;
			mask.x += (f->wallx * 10);
			mask.y += (f->wally * 10);
			
			collide = getTileCollision(1, mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			
			if (collide.x == -1) {
				int tempHsp = f->hsp;
				int tempVsp = f->vsp;
				f->hsp = f->wallx;
				f->vsp = f->wally;
				
				f->wallx = -tempHsp;
				f->wally = -tempVsp;
			}			
		}	
*/		
	}
	
	//Update Mask
	mask.w = 30;
	mask.h = 30;
	mask.x = f->x + 5;
	mask.y = f->y + 5;
	
	//Collide with hero
	if (checkCollision(mask, getHeroMask())) {
		heroHit(20, mask.x + (mask.w / 2));
	}
	
	//Weapon collision
	for (int i = 0; i < MAX_WEAPONS; i++) {
		if (weapons[i] != NULL) {
			if (weapons[i]->cooldown == 0) {
				if (checkCollision(mask, weapons[i]->weaponMask)) {
					weaponHit(weapons[i]);
					f->hp -= 1;
					f->blink = 15;
					
					//Death
					if (f->hp <= 0) {
						createEffect(2, f->x - 12, f->y - 12);
						spawnCollectable(f->x + 20, f->y);
						enemyDestroy(f->id);
					}
					
					i = MAX_WEAPONS;
				}
			}
		}	
	}
	
}

void firewheelDraw(Firewheel* f)
{
	if (f->blink % 2 == 0) {
		int cy = 80;
		if (f->dir == -1) {
			cy += 40;
		}
		
		PHL_DrawSurfacePart(f->x, f->y, 480 + ((int)f->imageIndex * 40), cy, 40, 40, images[imgEnemies]);
	}
}