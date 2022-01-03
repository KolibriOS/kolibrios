#include "thwomp.h"
#include "../enemy.h"
#include "../hero.h"
#include "../game.h"
#include "../PHL.h"
#include "../effect.h"
#include <stdlib.h>

void thwompStep(Thwomp* t);
void thwompDraw(Thwomp* t);

void createThwomp(int x, int y, int type, int offset, int delay, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Thwomp* t = malloc(sizeof *t);
			
			t->id = i;
			
			t->x = x;
			t->y = y;
			
			t->vsp = 0;
			t->grav = 0.3;
			
			t->imageIndex = 0;
			
			t->type = type;
			t->state = 0;
			t->timer = offset * 30;
			t->delay = delay * 30;
			//default delay is 60
			if (delay == 0) {
				t->delay = 60;
			}
			
			t->dir = dir;
			
			t->hp = 3;
			t->blink = 0;
			
			e->data = t;
			e->enemyStep = thwompStep;
			e->enemyDraw = thwompDraw;
			e->type = 16;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void thwompStep(Thwomp* t)
{
	//Animate
	{
		t->imageIndex += 0.1;
		if (t->imageIndex >= 3) {
			t->imageIndex -= 3;
		}
	}
	
	//Counters
	{
		if (t->blink > 0) {
			t->blink -= 1;
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.unused = mask.circle = 0;		
		mask.w = mask.h = 36;
		mask.x = t->x + ((40 - mask.w) / 2);
		mask.y = t->y + ((40 - mask.h) / 2);
	}
	
	//Wait
	if (t->state == 0) {
		t->vsp = 0;
		
		//Waiter
		if (t->type == 0) {
			Mask area;
			area.unused = area.circle = 0;
			area.x = t->x - 40;
			area.y = t->y;
			area.w = 120;
			area.h = 160;
			
			if (checkCollisionXY(area, herox, heroy)) {
				t->state = 1;
			}
		}
		
		//Automatic
		else{
			t->timer -= 1;
			if (t->timer <= 0) {
				t->state = 1;
			}
		}
	}
	
	//Fall
	else if (t->state == 1) {
		//Down
		if (t->dir == 0) {
			t->y += t->vsp;
		}
		//Left
		if (t->dir == 1) {
			t->x -= t->vsp;
		}
		//Right
		if (t->dir == 2) {
			t->x += t->vsp;
		}
		
		t->vsp += t->grav;

		if (t->vsp >= 7) {
			t->vsp = 7;
		}
		
		//Update Mask
		mask.x = t->x + ((40 - mask.w) / 2);
		mask.y = t->y + ((40 - mask.h) / 2);
		
		PHL_Rect collide = getTileCollision(1, mask);
		if (collide.x != -1) {
			int effX = t->x,
				effY = t->y;
			//Down
			if (t->dir == 0) {
				t->y = collide.y - 40;
				effY = t->y + 20;
			}
			//Left
			if (t->dir == 1) {
				t->x = collide.x + 40;
				effX = t->x - 20;
			}
			//Right
			if (t->dir == 2) {
				t->x = collide.x - 40;
				effX = t->x + 20;
			}
			
			t->state = 2;
			t->timer = 60;
			createEffect(1, effX, effY);
			PHL_PlaySound(sounds[sndHit07], CHN_ENEMIES);
		}
	}
	else if (t->state == 2) {
		if (t->type == 1) { //Automatic
			t->timer -= 1;
			if (t->timer <= 0) {
				t->state = 3;
			}
		}
	}
	else if (t->state == 3) { //rise up
		//Down
		if (t->dir == 0) {
			t->y -= 2;
		}
		//Left
		if (t->dir == 1) {
			t->x += 2;
		}
		//Right
		if (t->dir == 2) {
			t->x -= 2;
		}
		
		//Update Mask
		mask.x = t->x + ((40 - mask.w) / 2);
		mask.y = t->y + ((40 - mask.h) / 2);
		
		if (t->dir == 0) {
			mask.y -= 4;
		}
		if (t->dir == 1) {
			mask.x += 4;
		}
		if (t->dir == 2) {
			mask.x -= 4;
		}
		
		PHL_Rect collide = getTileCollision(1, mask);
		if (collide.x != -1) {
			//Down
			if (t->dir == 0) {
				t->y = collide.y + 40 + 2;
			}
			//Left
			if (t->dir == 1) {
				t->x = collide.x - 40 + 2; 
			}
			//Right
			if (t->dir == 2) {
				t->x = collide.x + 40 + 2;
			}
			
			t->state = 0;
			t->timer = t->delay;
		}
	}
	
	//Update Mask
	{
		mask.x = t->x + ((40 - mask.w) / 2);
		mask.y = t->y + ((40 - mask.h) / 2);
	}
	
	//Hit Player
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
						
						if (hasItem[16] == 1) { //Has blue paper
							t->hp -= 1;
							t->blink = 15;
							
							if (t->hp <= 0) {
								createRockSmash(t->x + 20, t->y + 20);
								spawnCollectable(t->x + 20, t->y);
								enemyDestroy(t->id);
							}
						}else{
							//Tink
							PHL_PlaySound(sounds[sndHit03], CHN_WEAPONS);
						}
						
						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}
	
}

void thwompDraw(Thwomp* t)
{
	if (t->blink % 2 == 0) {
		PHL_DrawSurfacePart(t->x, t->y, 240 + ((int)t->imageIndex * 40), 400, 40, 40, images[imgMisc20]);
	}
}