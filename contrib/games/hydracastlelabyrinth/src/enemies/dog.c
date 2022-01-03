#include "dog.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>

void dogStep(Dog* d);
void dogDraw(Dog* d);

int hitWall(Dog* d, Mask mask);

void createDog(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Dog* d = malloc(sizeof *d);
			d->id = i;
			d->hp = 3;
			d->blink = 0;
			
			d->x = x;
			d->y = y;
			
			d->hsp = 0;
			d->vsp = 0;
			
			d->imageIndex = 0;
			
			d->dir = 1;
			if (herox < d->x) {
				d->dir = -1;
			}
			
			d->state = 0;
			d->timer = 0;
			d->counter = 0;
			
			e->data = d;
			e->enemyStep = dogStep;
			e->enemyDraw = dogDraw;
			e->type = 30;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void dogStep(Dog* d)
{
	double grav = 0.175;
	
	char onground = 0;
	char wallhit = 0;
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 32;
		mask.h = 32;
		mask.x = d->x + ((40 - mask.w) / 2);
		mask.y = d->y + (40 - mask.h);
	}
	
	//Blink animation
	{
		if (d->blink > 0) {
			d->blink -= 1;
		}
	}
	
	//Horizontal movement
	{
		d->x += d->hsp;
		mask.x = d->x + ((40 - mask.w) / 2);
		
		//Wall collision
		if (hitWall(d, mask) == 1) {
			wallhit = 1;
			mask.x = d->x + ((40 - mask.w) / 2);
		}
	}
	
	//Vertical Movement
	{
		d->vsp += grav;
		d->y += d->vsp;		
		mask.y = d->y + (40 - mask.h);		
				
		PHL_Rect collide = getTileCollision(1, mask);
		if (collide.x == -1) {
			collide = getTileCollision(3, mask);
		}
		if (collide.x != -1) {
			//Floor
			if (d->vsp >= 0) {
				onground = 1;
				d->vsp = 0;
				d->y = collide.y - 40;
			}
			//Ceiling
			if (d->vsp < 0) {
				d->y = collide.y + 40 - (40 - mask.h);
			}
			mask.y = d->y + (40 - mask.h);
		}
	}
	
	//Wait
	if (d->state == 0)
	{
		double fric = 0.1;
		
		//Animate
		{
			d->imageIndex += 0.1;
			if (d->imageIndex >= 2) {
				d->imageIndex -= 2;
			}
		}		
		
		//Collide with wall
		{
			if (wallhit == 1 && onground == 1) {
				d->hsp *= -1;
			}
		}
		
		//Slide to hault
		if (d->hsp > 0) {
			d->dir = 1;
			
			d->hsp -= fric;
			if (d->hsp <= 0) {
				d->hsp = 0;
			}
		}
		if (d->hsp < 0) {
			d->dir = -1;
			
			d->hsp += fric;
			if (d->hsp >= 0) {
				d->hsp = 0;
			}
		}
		
		//Player is close
		{
			if (d->hsp == 0) {	
				Mask area;
				area.unused = area.circle = 0;
				area.w = 220;
				area.h = 60;
				area.x = d->x - 90;
				area.y = d->y - 20;
				
				if (checkCollision(area, getHeroMask()) == 1) {
					d->state = 1;
					d->counter = 0;
					d->vsp = 1;
				}
			}
		}
		
	}
	
	//Hopping
	else if (d->state == 1)
	{
		int spd = 2;

		d->hsp = spd * d->dir;
		
		//Land on floor
		{
			if (onground == 1) {
					
				//Landed					
				d->counter += 1;
				d->vsp = -1.5;				
				if (d->counter == 3) {
					d->vsp = -4;
				}
				if (d->counter == 4) {
					d->state = 0;
					d->counter = 0;
					d->vsp = 0;
					d->hsp = spd * d->dir;
				}else{
					PHL_PlaySound(sounds[sndPi05], CHN_ENEMIES);
					d->dir = 1;
					if (herox < d->x + 20) {
						d->dir = -1;
					}
				}
			}
		}
		
		//Animate
		{
			d->imageIndex = 1;
			if (d->vsp < 0) {
				d->imageIndex = 2;
			}
		}
		
	}
	
	//Update mask to be safe
	mask.x = d->x + ((40 - mask.w) / 2);
	mask.y = d->y + (40 - mask.h);
	
	//Hit Player
	{
		if (checkCollision(mask, getHeroMask())) {
			if (heroHit(10, mask.x + (mask.w / 2)) == 1) {
				heroStun();
			}
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
						//Hit
						d->blink = 15;
						d->hp -= 1;

						//Death
						if (d->hp <= 0) {
							createEffect(2, d->x - 12, d->y - 6);
							spawnCollectable(d->x + 20, d->y);
							enemyDestroy(d->id);
						}

						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}
	
}

void dogDraw(Dog* d)
{
	if (d->blink % 2 == 0) {
		int cropX = 240 + ((int)d->imageIndex * 40);
		
		if (d->dir == -1) {
			cropX += 120;
		}
		
		PHL_DrawSurfacePart(d->x, d->y, cropX, 40, 40, 40, images[imgEnemies]);
	}
}

int hitWall(Dog* d, Mask mask)
{
	PHL_Rect collide = getTileCollision(1, mask);
	
	if (collide.x == -1) {
		collide = getTileCollision(3, mask);
	}
	
	if (collide.x != -1) {
		int dir = 1;
		if (d->hsp < 0) {
			dir = -1;
		}
		d->x = collide.x + 20 - ((20 + (mask.w / 2)) * dir) - 20;
		
		return 1;
	}else{
		if (d->x < -20) {
			d->x = -20;
			return 1;
		}
		
		if (d->x > 620) {
			d->x = 620;
			return 1;
		}
	}
	
	return 0;
}