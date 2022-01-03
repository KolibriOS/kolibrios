#include "waterjumper.h"
#include <math.h>
#include <stdlib.h>
#include "../game.h"
#include "../enemy.h"
#include "../hero.h"
#include "../collision.h"

void waterJumperStep(WaterJumper* w);
void waterJumperDraw(WaterJumper* w);

void createWaterJumper(int x, int y, int type, int offset, int height)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			WaterJumper* w = malloc(sizeof *w);
			w->id = i;
			w->type = type;
			
			w->x = x;
			w->y = w->ystart = y;
			
			w->hp = 1;
			
			w->hsp = w->vsp = 0;
			w->grav = 0.115;
			w->yoffset = 0;
			
			w->imageIndex = 0;
			w->blink = 0;
			
			w->timer = 60;
			w->timer += 60 * offset;
			
			w->rot = 0;
			w->state = 0;
			
			if (type == 1) {
				w->hp = 2;
				w->timer = 60 * offset;
			}
			
			//Specific offset timer
			if (offset > 10) {
				w->timer = offset;
			}
			
			w->height = height;
			/*
			w->mask.unused = w->mask.circle = 0;
			w->mask.x = x + 8;
			w->mask.y = y + 8;
			w->mask.w = 24;
			w->mask.h = 24;
			*/
			e->data = w;
			e->enemyStep = waterJumperStep;
			e->enemyDraw = waterJumperDraw;
			e->type = 14;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void waterJumperStep(WaterJumper* w)
{
	//Counters
	{
		if (w->blink > 0) {
			w->blink -= 1;
		}
		
		if (w->timer > 0) {
			w->timer -= 1;
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.unused = mask.circle = 0;
		mask.w = 24;
		mask.h = 24;
		mask.x = w->x + ((40 - mask.w) / 2);
		mask.y = w->y + ((40 - mask.h) / 2);		
	}
	
	//Float
	if (w->state == 0)
	{
		//Animate
		{
			w->imageIndex += 0.1;
			if (w->imageIndex >= 2) {
				w->imageIndex -= 2;
			}
		}
	
		//Movement
		{
			w->rot += 5;
			if (w->rot >= 360) {
				w->rot -= 360;
			}
			
			w->y = w->ystart + (5 * sin(w->rot * 3.14159 / 180));
		}
		
		//Hop out of water
		{
			if (w->timer <= 0) {
				w->state = 1;
				w->timer = -1;
				
				createSplash(w->x + 20, w->y);
				w->y = w->ystart;			
			}
		}
	}
	//In air
	else if (w->state == 1)
	{
		//Animate
		{
			w->imageIndex += 0.25;
			if (w->imageIndex >= 3) {
				w->imageIndex -= 3;
			}
		}
	
		//Green type
		if (w->type == 0)
		{			
			//State Start
			{
				if (w->timer == -1) {
					w->timer = 0;
					
					w->vsp = -5.5;
					w->hsp = -2;
					if (w->x + 20 < herox) {
						w->hsp *= -1;
					}
				}
			}
			
			//Horizontal Movement
			w->x += w->hsp;
			
			//Land in water
			{
				if (w->vsp > 0 && w->y >= w->ystart) {
					createSplash(w->x + 20, w->y);
					w->y = w->ystart;
					w->state = 0;
					w->hsp = w->vsp = 0;
					w->timer = 120;
				}
			}
		}
		
		//Blue type
		else
		{			
			//State Start
			{
				if (w->timer == -1) {
					w->timer = 0;
					
					if (w->height == 2) {
						w->vsp = -5.5;
					}
					else if (w->height == 3) {
						w->vsp = -6;
					}
					else if (w->height == 4) {
						w->vsp = -7;
					}
					else if (w->height == 5) {
						w->vsp = -7.5;
					}
				}
			}
			
			//Land on expected ground
			{
				if (w->vsp > 0) {				
					if (w->y >= w->ystart - 22 - (w->height * 40)) {
						w->y = w->ystart - 22 - (w->height * 40);
						w->imageIndex = 5;
						w->state = 2;
						w->timer = 240;
						w->hsp = 2;
						if (herox < w->x + 20) {
							w->hsp *= -1;
						}
						
					}
				}
			}
			
		}	

		//Vertical Movement
		w->y += w->vsp;
		w->vsp += w->grav;
		
	}
	
	//Walk
	else if (w->state == 2) {
		//Animate
		{
			w->imageIndex += 0.16;
			if (w->imageIndex >= 7) {
				w->imageIndex -= 2;
			}
		}
		
		//Movement
		{
			w->x += w->hsp;
			mask.x = w->x + ((40 - mask.w) / 2);
		}
		
		//Hit wall
		if (checkTileCollision(1, mask) == 1) {
			w->hsp *= -1;
		}
		
		//Turn on edge
		else{			
			mask.x += (mask.w / 2) * w->hsp;
			mask.y += mask.h;
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			if (collide.x == -1) {
				w->hsp *= -1;
			}
		}
		
		//End walk
		{
			if (w->timer <= 0) {
				w->state = 3;
				w->timer = -1;			
			}
		}
	}
	
	//Jump Down
	else if (w->state == 3)
	{
		//Setup
		{
			if (w->timer == -1) {
				w->timer = 0;
				PHL_PlaySound(sounds[sndPi02], CHN_ENEMIES);
				w->vsp = -4;
				w->imageIndex = 2;
			}
		}
		
		//Animate
		{
			w->imageIndex += 0.25;
			if (w->imageIndex >= 5) {
				w->imageIndex -= 3;
			}
		}
		
		//Movement
		{
			w->y += w->vsp;
			w->vsp += w->grav;
			
			if (w->vsp > 6) {
				w->vsp = 6;
			}
		}
		
		//Land in Water
		{
			if (w->y >= w->ystart) {
				w->state = 0;
				createSplash(w->x + 20, w->y);
				w->timer = 60;
			}
		}
	}
	
	//Update Mask
	{
		mask.x = w->x + ((40 - mask.w) / 2);
		mask.y = w->y + ((40 - mask.h) / 2);
	}
	
	//Collide with hero
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
						
						w->blink = 15;
						w->hp -= 1;
						if (w->hp <= 0) {
							createEffect(2, w->x - 12, w->y - 12);
							spawnCollectable(w->x + 20, w->y);
							enemyDestroy(w->id);
						}
						
						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}
	
}

void waterJumperDraw(WaterJumper* w)
{
	if (w->blink % 2 == 0) {
		int cx = (int)w->imageIndex * 40;

		if (w->state == 1) {
			cx += 80;
		}
		
		if (w->type == 1) {
			cx += 200;
			
			if (w->state == 2 && w->hsp < 0) {
				cx += 80;
			}
		}
		
		PHL_DrawSurfacePart(w->x, w->y, cx, 440, 40, 40, images[imgEnemies]);
	}
}