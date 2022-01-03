#include "slime.h"
#include <stdlib.h>
#include "../PHL.h"
#include "../game.h"
#include "../collision.h"
#include "../hero.h"
#include "../enemy.h"
#include "../weapon.h"

void slimeStep(Slime* s);
void slimeDraw(Slime* s);

void createSlime(int x, int y, int type, int offset)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* result = malloc(sizeof *result);
			Slime* s = malloc(sizeof *s);
			
			s->id = i;
			
			s->x = x + 20;
			s->y = y;
			s->type = type;
			s->offset = offset;
			
			s->hp = 1;
			s->state = 0;
			s->counter = 0;
			s->timer = 0;
			s->grav = 0.125;
			s->vsp = 0;
			s->hsp = 0;
			s->imageIndex = 0;
			
			result->data = s;
			result->enemyStep = slimeStep;
			result->enemyDraw = slimeDraw;
			result->type = 0;
			
			enemies[i] = result;
			i = MAX_ENEMIES;
		}
	}
}

void slimeStep(Slime* s)
{
	char dead = 0;
	
	//Stay within room
	{
		if (s->x > 640) {
			s->x = 640;
		}
		
		if (s->x < 0) {
			s->x = 0;
		}
	}
	
	//Setup Rectangle Mask
	Mask mask;
	{
		mask.unused = 0;
		mask.circle = 0;
		mask.w = 24;
		mask.h = 24;
		mask.x = s->x - (mask.w / 2);
		mask.y = s->y + 28 - (mask.h / 2);
	}
	
	//Idle
	if (s->state == 0)
	{
		s->imageIndex += 0.25;
		
		if (s->imageIndex >= 6) {
			s->imageIndex = 0;
			
			if (s->offset <= 0) {
				s->state = 1;
				
				//Red/Yellow Slime
				if (s->type == 1 || s->type == 2)
				{
					s->hsp = 1;
					if (s->type == 2) {
						s->hsp = 1.5;
					}
					if ((int)(rand() % 2) == 0) {
						s->hsp *= -1;
					}
				}
				
				if (s->counter < 2) {
					s->vsp = -2;
					s->counter += 1;
				}else{
					s->vsp = -4;
					s->counter = 0;
				}
			}else{
				s->offset -= 1;
			}
		}
	}
	
	//Jump
	else if (s->state == 1)
	{
		//Red/Yellow Slime
		if (s->type == 1 || s->type == 2)
		{
			s->x += s->hsp;
			mask.x = s->x - (mask.w / 2);
			
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x != -1) {
				if (s->hsp > 0) {
					s->x = collide.x - (mask.w / 2);
				}else if (s->hsp < 0) {
					s->x = collide.x + 40 + (mask.w / 2);
				}
			}
			
			mask.x = s->x - (mask.w / 2);
		}
		
		s->y += s->vsp;
		s->vsp += s->grav;
		
		mask.y = s->y + 28 - (mask.h / 2);
		
		PHL_Rect collide = getTileCollision(1, mask);
		if (collide.x == -1) {
			collide = getTileCollision(3, mask);
		}
		if (collide.x != -1) {
			if (s->vsp >= 0) {
				s->state = 0;
				s->hsp = 0;
				s->y = collide.y - 40;
			}else{
				s->y = collide.y + 40 - (40 - mask.h) + 1;
			}
		}
	}
	
	//Setup Collision Mask
	{
		mask.unused = 0;
		mask.circle = 1;
		mask.w = 12;
		mask.x = s->x;
		mask.y = s->y + 28;
	}
	
	//Fell in a pit
	{
		if (s->y > 480) {
			dead = 1;
		}
	}
	
	//Collide with hero
	{
		if (checkCollision(mask, heroMask)) {
			int dmg[3] = {10, 20, 20};
			
			if (heroHit(dmg[s->type], s->x) == 1 && s->type == 2) {
				heroStun();
			}
		}
	}
	
	//Sword collision
	{
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (checkCollision(mask, weapons[i]->weaponMask)) {
					weaponHit(weapons[i]);
					
					spawnCollectable(s->x, s->y + 6);
					createEffect(2, s->x - 32, s->y - 12);
					dead = 1;	
					
					i = MAX_WEAPONS;
				}
			}	
		}
	}
	
	//Destroy object
	{
		if (dead == 1) {
			enemyDestroy(s->id);
		}
	}
}

void slimeDraw(Slime* s)
{	
	int cropX = 0,
		cropY = 0;

	//Idle
	if (s->state == 0) {
		int image[6] = { 0, 1, 2, 3, 4, 6 };
		cropX = image[(int)s->imageIndex] * 40;		
	}
	
	//Jump
	else if (s->state == 1) {
		cropX = 200;
		if (s->vsp >= 0) {
			cropX += 40;
		}
	}
	
	//Color offsets
	int addX[3] = {0, 280, 0};
	int addY[3] = {0, 0, 480};
	
	PHL_DrawSurfacePart(s->x - 20, s->y + 12, cropX + addX[s->type], cropY + addY[s->type], 40, 40, images[imgEnemies]);
}