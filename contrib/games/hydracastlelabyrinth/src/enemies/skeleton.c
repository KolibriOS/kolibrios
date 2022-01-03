#include "skeleton.h"
#include "../enemy.h"
#include "../game.h"
#include "../PHL.h"
#include "../hero.h"
#include <stdlib.h>

void skeletonStep(Skeleton* s);
void skeletonDraw(Skeleton* s);

void boneStep(Bone* b);
void boneDraw(Bone* b);

void createSkeleton(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Skeleton* s = malloc(sizeof *s);
			s->id = i;
			
			s->x = x;
			s->y = y;
			
			s->imageIndex = 0;
			
			s->dir = 1;
			if (dir == 1) {
				s->dir = -1;
			}
			
			s->hsp = 0.5 * s->dir;
			
			s->hp = 2;
			
			s->state = 0;
			s->timer = 0;
			s->invincible = 0;
			
			s->mask.unused = s->mask.circle = 0;
			s->mask.w = 24;
			s->mask.h = 36;
			s->mask.x = s->x + 8;
			s->mask.y = s->y + 4;
			
			e->data = s;
			e->enemyStep = skeletonStep;
			e->enemyDraw = skeletonDraw;
			e->type = 17;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void skeletonStep(Skeleton* s)
{	
	if (s->invincible > 0) {
		s->invincible -= 1;
	}

	//Collide with wall
	if (checkTileCollision(1, s->mask) == 1) {
		s->hsp *= -1;
	}else{
		//Check if on ledge
		int tempdir = 1;
		if (s->hsp < 0) {
			tempdir = -1;
		}
		s->mask.x += tempdir * s->mask.w;
		s->mask.y += 10;
		if (checkTileCollision(1, s->mask) == 0 && checkTileCollision(3, s->mask) == 0) {
			s->hsp *= -1;
		}
		s->mask.y -= 10;
		s->mask.x = s->x + 8;
	}
	
	s->x += s->hsp;
	
	if (s->timer >= 0) {
		s->timer -= 1;
	}

	//Walk around
	if (s->state == 0)
	{
		s->imageIndex += 0.1;
		
		if (s->hsp < 0) {
			s->dir = -1;
			s->hsp = -0.5;
		}else if (s->hsp > 0) {
			s->dir = 1;
			s->hsp = 0.5;
		}else{
			s->hsp = 0.5 * s->dir;
		}
		
		if (s->timer <= 0) {
			//If hero is too close
			Mask area;
			area.unused = area.circle = 0;
			area.x = s->x - 80;
			area.y = s->y - 20;
			area.w = 200;
			area.h = 80;
			
			if (checkCollision(area, getHeroMask()) == 1) {
				s->state = 1;
				s->timer = 30;
				s->hsp = 0;
				
				s->dir = 1;
				if (herox < s->mask.x + (s->mask.w / 2)) {
					s->dir = -1;
				}
			}
		}
	}
	//Alert
	else if (s->state == 1)
	{
		s->hsp = 0;
		s->imageIndex += 0.1;
		
		if (s->timer <= 0) {
			s->state = 2;
			s->hsp = 2.5 * -s->dir;
			PHL_PlaySound(sounds[sndPi05], CHN_ENEMIES);
		}
	}
	//Slide backwards
	else if (s->state == 2)
	{
		s->imageIndex = 0;
		double fric = 0.075;
		if (s->hsp > 0) {
			s->hsp -= fric;
			if (s->hsp <= 0) { s->hsp = 0; }
		}
		else if (s->hsp < 0) {
			s->hsp += fric;
			if (s->hsp >= 0) { s->hsp = 0; }
		}
		
		if (s->hsp == 0) {
			s->state = 3;
			s->timer = 30;
			createBone(s->x, s->y, s->dir);
			PHL_PlaySound(sounds[sndShot05], CHN_ENEMIES);
		}
	}
	//Throw bone
	else if (s->state == 3)
	{
		s->imageIndex += 0.1;
		if (s->timer <= 0) {
			s->timer = 0;
			s->state = 0;
		}
	}
	
	if (s->imageIndex >= 2) {
		s->imageIndex -= 2;
	}
	
	//Update mask
	s->mask.x = s->x + 8;
	
	//Hit Player
	if (checkCollision(s->mask, getHeroMask())) {
		heroHit(10, s->x + 20);
	}
		
	int i;
	for (i = 0; i < MAX_WEAPONS; i++) {
		if (weapons[i] != NULL) {
			if (weapons[i]->cooldown == 0) {
				if (checkCollision(s->mask, weapons[i]->weaponMask)) {
					s->hp -= 1;
					s->invincible = 15;
					weaponHit(weapons[i]);
					if (s->hp <= 0) {						
						createRockSmash(s->x + 20, s->y + 20);
						spawnCollectable(s->x + 20, s->mask.y + s->mask.h - 40);
						enemyDestroy(s->id);
					}
					i = MAX_WEAPONS;
				}
			}
		}	
	}
}

void skeletonDraw(Skeleton* s)
{
	if (s->invincible % 2 == 0) {
		int dx = 160 + ((int)s->imageIndex * 40);
		
		if (s->dir == -1) {
			dx += 80;
		}
		
		PHL_DrawSurfacePart(s->x, s->y, dx, 240, 40, 40, images[imgEnemies]);
	}
}



void createBone(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Bone* b = malloc(sizeof *b);
			b->id = i;
			
			b->x = x;
			b->y = y;
			
			b->hsp = dir * 0.75;
			b->vsp = -4;
			b->grav = 0.1;
			
			b->imageIndex = 0;
			
			b->mask.unused = 0;
			b->mask.circle = 1;
			b->mask.w = 12;
			b->mask.h = 12;
			b->mask.x = b->x + 20;
			b->mask.y = b->y + 20;
			
			e->data = b;
			e->enemyStep = boneStep;
			e->enemyDraw = boneDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void boneStep(Bone* b)
{
	if (b->hsp < 0) {
		b->imageIndex += 0.25;
		if (b->imageIndex >= 4) {
			b->imageIndex -= 4;
		}
	}
	else{
		b->imageIndex -= 0.25;
		if (b->imageIndex < 0) {
			b->imageIndex += 4;
		}
	}
	
	b->x += b->hsp;
	
	b->y += b->vsp;
	b->vsp += b->grav;
	
	//Update Mask
	b->mask.x = b->x + 20;
	b->mask.y = b->y + 20;
	
	if (b->y > 480) {
		enemyDestroy(b->id);
	}
	
	//Hit Player
	if (checkCollision(b->mask, shieldMask)) {
		enemyDestroy(b->id);
		PHL_PlaySound(sounds[sndHit07], CHN_EFFECTS);
		createEffect(1, b->x, b->y);
	}else{
		if (checkCollision(b->mask, getHeroMask())) {
			heroHit(10, b->x + 20);
		}
	}
}

void boneDraw(Bone* b)
{
	int img = 320 + ((int)b->imageIndex * 40);
	if (b->hsp > 0) {
		img += 160;
	}
	
	PHL_DrawSurfacePart(b->x, b->y, img, 240, 40, 40, images[imgEnemies]);
}