#include "podoboo.h"
#include "../PHL.h"
#include "../hero.h"
#include "../game.h"
#include "../effect.h"
#include <stdlib.h>
#include <math.h>

void podobooStep(Podoboo* p);
void podobooDraw(Podoboo* p);

void createPodoboo(int x, int y, int offset, int height)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Podoboo* p = malloc(sizeof *p);
			
			p->id = i;
			
			p->x = x;
			p->y = p->ystart = y;
			
			p->hp = 2;
			p->blink = 0;			
			
			p->yoffset = p->rot = 0;
			
			p->vsp = 0;
			p->grav = 0.13;
			
			p->jumpheight = -5;
			/*
			if (height == 1) {
				p->jumpheight = -5.4;
			}
			*/
			if (height == 1) {
				p->jumpheight = -7;
			}
			
			p->imageIndex = 0;			
			
			p->timer = 30 * offset;
			p->state = 0;
			
			e->data = p;
			e->enemyStep = podobooStep;
			e->enemyDraw = podobooDraw;
			e->type = 15;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void podobooStep(Podoboo* p)
{
	//Blinking
	{
		if (p->blink > 0) {
			p->blink -= 1;
		}
	}
	
	p->timer -= 1;
	
	//Patterns
	{
		//Float in lava
		if (p->state == 0)
		{
			//Animate
			p->imageIndex += 0.1;
			if (p->imageIndex >= 2) {
				p->imageIndex -= 2;
			}
		
			//Bob movement
			p->rot += 5;
			if (p->rot >= 360) {
				p->rot -= 360;
			}
			p->y = p->ystart + (5 * sin(p->rot * 3.14159 / 180));
			
			//Jump
			if (p->timer <= 0) {
				p->state = 1;
				createLavaSplash(p->x + 20, p->y);
				
				p->y = p->ystart;
				p->vsp = p->jumpheight;
			}
		}
		//In air
		else if (p->state == 1)
		{
			//Animate
			p->imageIndex += 0.25;
			if (p->imageIndex >= 3) {
				p->imageIndex -= 3;
			}
		
			//Movement
			p->y += p->vsp;
			p->vsp += p->grav;
			
			//Land in lava again
			if (p->vsp > 0 && p->y >= p->ystart) {
				createLavaSplash(p->x + 20, p->y);
				p->y = p->ystart;
				p->state = 0;
				p->vsp = 0;
				p->timer = 60;
			}
		}
		
	}
	
	//Create Mask
	Mask mask;
	{
		mask.unused = mask.circle = 0;
		mask.w = mask.h = 30;
		mask.x = p->x + 5;
		mask.y = p->y + 5;
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
						p->hp -= 1;
						p->blink = 15;
						
						//Death
						if (p->hp <= 0) {
							createEffect(2, p->x - 12, p->y - 12);
							spawnCollectable(p->x + 20, p->y);
							enemyDestroy(p->id);
						}
						
						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}

}

void podobooDraw(Podoboo* p)
{
	if (p->blink % 2 == 0) {
		int thisImage = p->imageIndex;
		
		if (p->state == 1) {
			thisImage += 2;
		}
		
		PHL_DrawSurfacePart(p->x, p->y, 280 + (40 * thisImage), 520, 40, 40, images[imgEnemies]);
	}
}