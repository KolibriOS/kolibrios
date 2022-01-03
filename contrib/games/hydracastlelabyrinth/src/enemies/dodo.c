#include "dodo.h"
#include "../game.h"
#include "../hero.h"
#include "../PHL.h"
#include "../enemy.h"
#include <stdlib.h>

void dodoStep(Dodo* d);
void dodoDraw(Dodo* d);

Mask updateDodoMask(Dodo* d, Mask mask);
int dodoWallCollision(Dodo* d, Mask mask);

int boss1flag = 1;

void createDodo(int x, int y, int flag)
{
	char miniboss = 0;
	
	if (flag != 0) {
		miniboss = 1;
	}else{
		flag = boss1flag;
	}
	
	if (flags[flag] == 0) {
		PHL_FreeSurface(images[imgBoss]);
		images[imgBoss] = PHL_LoadQDA("boss01.bmp");
		
		int i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i] == NULL) {
				
				if (miniboss == 0) {
					setBossRoom();
				}
				
				Enemy* e = malloc(sizeof *e);
				Dodo* d = malloc(sizeof *d);
				d->id = i;
				
				d->x = x;
				d->y = y;
				
				d->vsp = -6;
				d->hsp = 0;
				d->grav = 0.2;
				
				d->onground = 0;
				d->dir = -1;
				if (herox > d->x) {
					d->dir = 1;
				}
				
				d->imageIndex = 0;
				
				d->timer = 0;
				d->state = 2;
				
				d->hp = 45;
				d->blink = 0;
				
				d->tojump = 1;
				d->jumptoggle = 0;
				
				d->flag = flag;
				
				e->data = d;
				e->enemyStep = dodoStep;
				e->enemyDraw = dodoDraw;
				e->type = 40;
				
				enemies[i] = e;
				i = MAX_ENEMIES;
			}
		}
	}
}

void dodoStep(Dodo* d)
{	
	char dead = 0;
	
	//Constants
	double fric = 0.06;
	
	//Animation vars
	double imgspd = 0;
	int frames = 0;
	
	//timers
	{
		if (d->timer > 0) {
			d->timer -= 1;
		}
		
		if (d->blink > 0) {
			d->blink -= 1;
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 56;
		mask.h = 56;
		mask = updateDodoMask(d, mask);
	}
	
	//Idle
	if (d->state == 0)
	{
		d->hsp = 0;
		d->vsp = 0;
		
		//Animate
		imgspd = 0.1;
		frames = 4;
	
		//End state
		if (d->timer <= 0) {
			//Go to chase
			if (d->tojump == 0) {
				d->state = 1;
				d->timer = 260;
				d->tojump = 1;
			}
			//Go to windup
			else {
				d->state = 3;
				d->timer = 30;
			}
		}
		
		//Fall
		{
			if (d->onground == 0) {
				d->state = 6;
				d->imageIndex = 1;
			}
		}
	}
	
	//Chase
	else if (d->state == 1)
	{
		//Animate
		imgspd = 0.2;
		frames = 4;
		
		//Chase
		if ( (d->dir == -1 && herox < d->x) || (d->dir == 1 && herox > d->x) ) {
			d->hsp += (fric / 2) * d->dir;
			
			//limit speed
			if (d->hsp > 3) {
				d->hsp = 3;
			}
			if (d->hsp < -3) {
				d->hsp = -3;
			}
		}
		
		//Turn around
		else{ 
			d->hsp -= fric * d->dir;
			
			//Done slowing down
			if ( (d->dir == 1 && d->hsp <= 0) || (d->dir == -1 && d->hsp >= 0) ) {
				d->hsp = 0;
				d->state = 4;
				d->imageIndex = 0;
			}
		}

		//Stop running
		{
			if (d->timer <= 0) {
				if (d->hsp >= 1 || d->hsp <= -1) {
					d->state = 5;
				}
			}
		}
		
		//Fall
		{
			if (d->onground == 0) {
				d->state = 6;
				d->imageIndex = 1;
			}
		}
		
	}
	
	//Turn around
	else if (d->state == 4)
	{
		//Animate
		imgspd = 0;
		d->imageIndex += 0.2;
		
		//Done turning around
		if (d->imageIndex >= 3) {
			d->dir *= -1;
			d->state = 1;
			d->imageIndex = 0;				
		}
		
		//Fall
		{
			if (d->onground == 0) {
				d->state = 6;
				d->imageIndex = 1;
			}
		}
	}
	
	//Jump
	if (d->state == 2)
	{
		//Set image
		imgspd = 0;
		{
			//Fall
			d->imageIndex = 0;
			
			//Jump
			if (d->vsp < 0) {
				d->imageIndex = 1;
			}
		}
		
		//Face hsp
		{
			if (d->hsp > 0) {
				d->dir = 1;
			}
			if (d->hsp < 0) {
				d->dir = -1;
			}
		}
		
		//Land
		{
			if (d->onground == 1) {
				d->state = 5;
				d->tojump = 0;
				
				PHL_PlaySound(sounds[sndHit04], CHN_ENEMIES);
				quakeTimer = 30;
				createEffectExtra(3, d->x - 30, d->y + 50, -1, 0, 0);
				createEffectExtra(3, d->x - 10, d->y + 50, 1, 0, 0);
			}
		}
		
	}
	
	//Windup
	if (d->state == 3)
	{ 
		//Set image
		imgspd = 0;
		d->imageIndex = 0;
		
		//Jump
		if (d->timer <= 0) {
			d->state = 2;
			PHL_PlaySound(sounds[sndJump01], CHN_ENEMIES);
			if (d->jumptoggle == 0) {
				d->jumptoggle = 1;
				d->vsp = -3;
				d->hsp = 2 * d->dir;
			}else{
				d->jumptoggle = 0;
				d->hsp = 1.5 * d->dir;
				d->vsp = -6;
			}
		}
	}
	
	//Slide to a stop
	else if (d->state == 5)
	{
		//Friction
		{
			if (d->hsp > 0) {
				d->hsp -= fric;
				if (d->hsp <= 0) {
					d->hsp = 0;
				}
			}
			else if (d->hsp < 0) {
				d->hsp += fric;
				if (d->hsp >= 0) {
					d->hsp = 0;
				}
			}
		}
		
		//Go to idle
		{
			if (d->hsp == 0) {
				d->state = 0;
				d->timer = 140;
			}
		}
		
	}
	
	//Fall
	if (d->state == 6)
	{		
		//Set image
		imgspd = 0;
		d->imageIndex = 0;
		
		//Face hsp
		{
			if (d->hsp > 0) {
				d->dir = 1;
			}
			if (d->hsp < 0) {
				d->dir = -1;
			}
		}
		
		//Land
		{
			if (d->onground == 1) {
				d->state = 5;
				d->tojump = 1;
			}
		}
		
	}
	
	//Death
	if (d->state == 7)
	{
		imgspd = 0.2;
		frames = 4;
		
		//Movement
		d->y += 0.2;
		
		if (d->blink % 12 == 0) {
			createEffect(2, d->x - 72 + (rand() % 80), d->y - 12 + (rand() % 76));
		}
		
		if (d->blink <= 0) {
			dead = 1;
		}
	}
	
	else{		
		//Horizontal movement
		{
			if (d->hsp != 0) {
				d->x += d->hsp;
				
				//Wall collision
				if (d->state != 6) {
					if (dodoWallCollision(d, mask) == 1) {					
						d->hsp *= -1;
						if (d->state == 1) {
							d->state = 5;
						}
					}
				}
				
			}
		}
		
		//Vertical movement
		{
			if (d->vsp != 0) {
				d->y += d->vsp;
				
				mask = updateDodoMask(d, mask);
				PHL_Rect collide = getTileCollision(1, mask);
				if (collide.x != -1) {
					if (d->vsp < 0) {
						d->y = collide.y + 40 - (96 - 14 - mask.h);
					}
					else if (d->vsp > 0) {
						d->y = collide.y - 96 + 14;
					}
				}
			}
		}
		
		//Check if onground
		mask = updateDodoMask(d, mask);
		mask.y += 1;
		if (!checkTileCollision(1, mask)) {
			d->onground = 0;
		}else{
			d->onground = 1;
		}
		mask.y -= 1;
		
		//Gravity
		{
			if (d->onground == 0) {
				d->vsp += d->grav;
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
								d->state = 7;
								d->blink = 180;
							}

							i = MAX_WEAPONS;
						}
					}
				}	
			}
		}
		
		//Hit Player
		{
			if (checkCollision(mask, getHeroMask())) {
				heroHit(30, d->x);
			}
		}
	}
	
	//Animate
	{
		if (imgspd != 0) {
			d->imageIndex += imgspd;
			
			if (d->imageIndex >= frames) {
				d->imageIndex -= frames;
			}
		}
	}
	
	//Destroy object
	{
		if (dead == 1) {
			//Is the level 1 boss
			if (d->flag == boss1flag) {
				bossDefeatedFlag = 1;
				PHL_StopMusic();
			}
			
			roomSecret = 1;
			flags[d->flag] = 1;
			enemyDestroy(d->id);
		}
	}
	
}

void dodoDraw(Dodo* d)
{
	//PHL_DrawMask(d->mask);
	if (d->blink % 2 == 0) {
		int cropX = 0,
			cropY = 0;
			
		int dirW = 0;

		//Idle
		if (d->state == 0) {
			dirW = 0;
			int frame = 0;			
			
			if (d->dir == 1) {
				int animation[4] = {0, 6, 7, 6};
				frame = animation[(int)d->imageIndex];
			}else{
				int animation[4] = {3, 8, 9, 8};
				frame = animation[(int)d->imageIndex];
			}
			
			cropX = frame * 96;
			
			while (cropX >= 576) {
				cropX -= 576;
				cropY += 96;
			}
		}
		
		//Chase
		else if (d->state == 1 || d->state == 7) {
			dirW = 288;
			int animation[4] = {0, 1, 0, 2};
			
			cropX = animation[(int)d->imageIndex] * 96;
		}

		//Jump
		else if (d->state == 2) {
			dirW = 192;
			
			cropY = 192;
			cropX = (int)d->imageIndex * 96;
		}
		
		//Turn around
		else if (d->state == 4) {
			dirW = 0;
			cropY = 288;
			
			if (d->dir == 1) {
				int animation[3] = {0, 1, 2};
				cropX = animation[(int)d->imageIndex] * 96;
			}else{
				int animation[3] = {2, 1, 0};
				cropX = animation[(int)d->imageIndex] * 96;
			}					
		}
		
		//Duck
		else if (d->state == 3 || d->state == 5 || d->state == 6) {
			dirW = 192;
			
			cropX = 0;
			cropY = 192;
		}
		
		//Direction offset
		{
			if (dirW != 0 && d->dir == -1) {
				cropX += dirW;
			}
		}
		
		PHL_DrawSurfacePart(d->x - 48, d->y, cropX, cropY, 96, 96, images[imgBoss]);
	}
}

Mask updateDodoMask(Dodo* d, Mask mask)
{
	mask.x = d->x - (mask.w / 2);
	mask.y = d->y + (96 - 14 - mask.h);
	
	return mask;
}

int dodoWallCollision(Dodo* d, Mask mask)
{
	int result = 0;
	
	mask = updateDodoMask(d, mask);
	
	//Stay inside of room
	if (d->x < 24) {
		result = 1;
		d->x = 24;
	}
	else if (d->x > 616) {
		result = 1;
		d->x = 616;
	}
	else{
		//Tile collision
		PHL_Rect collide = getTileCollision(1, mask);
		if (collide.x != -1) {
			result = 1;
			if (d->hsp > 0) {
				d->x = collide.x - (mask.w / 2);
			}else{
				d->x = collide.x + 40 + (mask.w / 2);
			}
		}
	}
	
	return result;
}