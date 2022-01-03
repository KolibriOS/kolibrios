#include "bat.h"
#include "../game.h"
#include "../PHL.h"
#include "../hero.h"
#include <stdlib.h>
#include <math.h>

void batStep(Bat* b);
void batDraw(Bat* b);

void createBat(int x, int y, int type)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* result = /*(Enemy*)*/malloc(sizeof *result);
			Bat* b = /*(Bat*)*/malloc(sizeof *b);
			b->id = i;
			
			b->x = b->xstart = x;
			b->y = b->ystart = y;
			b->type = type;
			
			b->imageIndex = 5;
			b->counter = 0;
			b->timer = 0;
			b->state = 0;
			b->dir = 1;

			result->data = b;
			result->enemyStep = batStep;
			result->enemyDraw = batDraw;
			result->type = 1;
			
			enemies[i] = result;
			i = MAX_ENEMIES;
		}
	}
}

void batStep(Bat* b)
{	
	//Wait
	if (b->state == 0)
	{ 
		//Animate
		{
			b->imageIndex = 5;
		}
		
		//wait for hero to get near
		{
			if (b->timer <= 0) {
				Mask area;
				area.circle = 0;
				area.unused = 0;
				area.x = b->xstart - 60;
				area.y = b->ystart;
				area.w = 160; area.h = 100;
				
				if (checkCollisionXY(area, herox, heroy + 20)) {
					PHL_PlaySound(sounds[sndPi07], CHN_ENEMIES);
					b->state = 1;
					b->timer = 270;
					if (b->type == 1) {
						b->counter = 1;
						if (herox < b->x + 20) {
							b->dir = -1;
						}else{
							b->dir = 1;
						}
					}
				}
			}else{
				b->timer -= 1;
			}
		}
	}
	//Fly
	else if (b->state == 1)
	{
		//Animate
		{
			b->imageIndex += 0.25;
			if (b->imageIndex >= 5) {
				b->imageIndex -= 5;
			}
		}
		
		//Rotation angle
		{
			b->timer += 4;
			if (b->timer >= 360) {
				b->timer -= 360;
			}
		}
		
		//Movement
		{
			b->y = b->ystart + 30 + (30 * sin(b->timer * 3.14159 / 180));
			//Red bat
			if (b->type == 1) { 
				b->x += 2 * b->dir;
			}
		}
		
		//Return to perch
		{
			if (b->timer == 270) {
				if (b->type == 1 && b->counter > 0) {
					b->dir *= -1;
					b->timer = 270;
					b->counter -= 1;
				}else{
					b->state = 0;
					b->timer = 70;
				}
			}
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 32;
		mask.h = 28;
		mask.x = b->x + ((40 - mask.w) / 2);
		mask.y = b->y;
	}
	
	//Hit Player
	{
		if (checkCollision(mask, heroMask)) {
			heroHit(10, mask.x + (mask.w / 2));
		}
	}
	
	//Weapon collision
	{
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (checkCollision(mask, weapons[i]->weaponMask)) {
					weaponHit(weapons[i]);
					//Death
					createEffect(2, b->x - 12, b->y - 6);
					spawnCollectable(b->x + 20, b->y);
					enemyDestroy(b->id);

					i = MAX_WEAPONS;
				}
			}	
		}
	}
	
}

void batDraw(Bat* b)
{
	int cropX = 0,
		cropY = 120;
	
	if (b->type == 1) {
		cropX = 400;
		cropY = 280;
	}
	
	cropX += (int)b->imageIndex * 40;
	
	PHL_DrawSurfacePart(b->x, b->y - 4, cropX, cropY, 40, 40, images[imgEnemies]);
}