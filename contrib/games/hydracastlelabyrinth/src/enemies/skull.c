#include "skull.h"
#include "../enemy.h"
#include "../PHL.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>
#include <math.h>

void skullStep(Skull* s);
void skullDraw(Skull* s);

void createSkull(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Skull* s = malloc(sizeof *s);
			s->id = i;
			
			//X/Y in center of sprite
			s->x = x + 20;
			s->y = y + 20;
			s->yoffset = 0;
			
			s->rot = 0;
			s->state = 0;
			s->timer = 0;
			s->imageIndex = 0;
			s->dir = 0;

			e->data = s;
			e->enemyStep = skullStep;
			e->enemyDraw = skullDraw;
			e->type = 12;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void skullStep(Skull* s)
{
	double imageSpeed = 0;
	
	//Wait
	if (s->state == 0)
	{
		imageSpeed = 0.2;
		
		if (s->timer > 0) {
			s->timer -= 1;
		}else{		
			Mask tempmask;
			
			tempmask.unused = tempmask.circle = 0;
			tempmask.x = s->x - 100;
			tempmask.y = s->y - 100;
			tempmask.w = tempmask.h = 200;
			
			if (checkCollisionXY(tempmask, herox, heroy + 20)) {			
			
			//Calculate distance
			//int dis = sqrt(pow(s->x - herox, 2) + pow(s->y - (heroy + 20), 2));
			//if (dis <= 100) {
				s->state = 1;
				//s->dir = (rand() % 8) * 45;
				s->dir = (rand() % 360) + 1;
				PHL_PlaySound(sounds[sndPi08], CHN_ENEMIES);
				s->timer = 130;
			}
		}
	}
	
	//Chase
	else if (s->state == 1)
	{
		imageSpeed = 0.3;		
		
		int spd = 2;
		s->x += (spd * cos(s->dir * 3.14159 / 180));
		s->y += (spd * sin(s->dir * 3.14159 / 180));
		
		double herodir = ((atan2((heroy + 20) - s->y, herox - s->x) * 180) / 3.14159);
		if (herodir >= 360) {
			herodir -= 360;
		}
		if (herodir < 0) {
			herodir += 360;
		}
		
		double tempdir = s->dir - herodir;
		if (tempdir < 0) {
			tempdir += 360;
		}
		
		if (tempdir < 180) {
			s->dir -= 2;
		}else{
			s->dir += 2;
		}
		if (s->dir >= 360) {
			s->dir -= 360;
		}
		if (s->dir < 0) {
			s->dir += 360;
		}
		
		s->timer -= 1;
		if (s->timer <= 0) {
			s->state = 0;
			s->timer = 10;
		}
	}
	
	//Animate
	{
		s->imageIndex += imageSpeed;
		if (s->imageIndex >= 4) {
			s->imageIndex -= 4;
		}
	}
	
	//Hover offset
	{
		s->rot += 5;
		if (s->rot >= 360) {
			s->rot -= 360;
		}
		s->yoffset = (5 * sin(s->rot * 3.14159 / 180));
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.unused = 0;
		mask.circle = 1;
		mask.x = s->x;
		mask.y = s->y;
		mask.w = mask.h = 10;
	}	
	
	//Hero collision
	{
		if (checkCollision(mask, getHeroMask())) {
			heroHit(15, mask.x);
		}
	}
	
	//Weapon collision
	{
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (checkCollision(mask, weapons[i]->weaponMask)) {
					weaponHit(weapons[i]);
					
					createEffect(2, s->x - 32, s->y - 32);
					spawnCollectable(s->x, s->y - 20);
					enemyDestroy(s->id);
					
					i = MAX_WEAPONS;
				}
			}	
		}
	}
	
}

void skullDraw(Skull* s)
{
	PHL_DrawSurfacePart(s->x - 20, s->y + s->yoffset - 20, 480 + ((int)s->imageIndex * 40), 40, 40, 40, images[imgEnemies]);
}