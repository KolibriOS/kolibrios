#include "boar.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>

void boarStep(Boar* b);
void boarDraw(Boar* b);

void createBoar(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = /*(Enemy*)*/malloc(sizeof *e);
			Boar* b = /*(Boar*)*/malloc(sizeof *b);
			
			b->id = i;
			
			b->hp = 3;
			
			b->x = x;
			b->y = y;
			
			b->hsp = 0;
			
			b->imageIndex = 0;
			b->dir = 1;
			
			b->blink = 0;
			
			b->state = 0;
			b->timer = 0;
			
			e->data = b;
			e->enemyStep = boarStep;
			e->enemyDraw = boarDraw;
			e->type = 26;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void boarStep(Boar* b)
{
	//Setup mask
	Mask mask;
	{
		mask.unused = mask.circle = 0;
		mask.w = 32;
		mask.h = 28;
		mask.x = b->x + ((40 - mask.w) / 2);
		mask.y = b->y + 40 - mask.h;
	}
	
	//Blink animation
	{
		if (b->blink > 0) {
			b->blink -= 1;
		}
	}
	
	//Patterns
	{
		//Dance
		if (b->state == 0)
		{
			//Animate
			b->imageIndex += 0.15;
			if (b->imageIndex >= 8) {
				b->imageIndex -= 8;
			}
			
			//if player gets near
			Mask area;
			area.unused = area.circle = 0;
			area.x = b->x - 80;
			area.y = b->y - 40;
			area.w = 200;
			area.h = 80;
			
			if (checkCollision(area, getHeroMask()) == 1) {
				b->state = 1;
				b->timer = -1;
				b->imageIndex = 0;
				b->dir = 1;
				if (herox < b->x + 20) {
					b->dir = -1;
				}
			}
		}
		//Rev up
		else if (b->state == 1)
		{
			b->timer += 1;
			
			//Play sound
			if (b->timer % 10 == 0) {
				PHL_PlaySound(sounds[sndShot01], CHN_ENEMIES);
			}
			
			//Create effect
			if (b->timer % 16 == 0) {
				if (b->dir == 1) {
					createEffectExtra(3, b->x + 20 - 30, b->y + 8, -1, 0, 0);
				}
				if (b->dir == -1) {
					createEffectExtra(3, b->x + 20 - 10, b->y + 8, 1, 0, 0);
				}
			}			
			
			if (b->timer >= 60) {
				b->state = 2;
				b->hsp = 3;
			}
		}
		//Running
		else if (b->state == 2)
		{
			b->x += b->hsp * b->dir;
			mask.x = b->x + ((40 - mask.w) / 2);
			
			//Collide with wall
			if (checkTileCollision(1, mask) == 1) {
				b->x -= b->hsp * b->dir;
				b->dir *= -1;
			}
			
			//On edge
			{
				mask.x = b->x + ((40 - mask.w) / 2);
				
				mask.x += mask.w * b->dir;
				mask.y += 1;
				
				PHL_Rect collide = getTileCollision(1, mask);
				if (collide.x == -1) {
					collide = getTileCollision(3, mask);
				}
				if (collide.x == -1) {
					b->dir *= -1;
				}
				
				mask.y -= 1;
				mask.x = b->x + ((40 - mask.w) / 2);
			}
			
			b->hsp -= 0.05;
			if (b->hsp <= 0) {
				b->state = 0;
			}
		}
		
		//Running animation
		if (b->state == 1 || b->state == 2) {
			//Animate
			b->imageIndex += 0.2;
			if (b->imageIndex >= 2) {
				b->imageIndex -= 2;
			}
		}
	}
	
	//Collide with hero
	{
		if (checkCollision(mask, getHeroMask())) {
			heroHit(30, mask.x + (mask.w / 2));
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
						b->hp -= 1;
						b->blink = 15;
						
						//Death
						if (b->hp <= 0) {
							createEffect(2, b->x - 12, b->y - 12);
							spawnCollectable(b->x + 20, b->y);
							enemyDestroy(b->id);
						}
						
						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}
}

void boarDraw(Boar* b)
{
	if (b->blink % 2 == 0)
	{
		int cropx = 0, cropy = 360;
		int drawx = b->x, drawy = b->y;
		
		//Dance
		if (b->state == 0) {
			int animation[8] = {0, 1, 2, 1, 0, 3, 4, 3};
			cropx = 160 + (animation[(int)b->imageIndex] * 40);
		}
		//Charge
		else{
			cropx = (int)b->imageIndex * 40;
			
			if (b->dir == -1) {
				cropx += 80;
			}
		}
		
		PHL_DrawSurfacePart(drawx, drawy, cropx, cropy, 40, 40, images[imgEnemies]);
	}
}