#include "knight.h"
#include "../enemy.h"
#include "../hero.h"
#include "../PHL.h"
#include "../game.h"
#include <stdlib.h>

void knightDestroy(Knight* k);

void createKnight(int x, int y, int type)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Knight* k = malloc(sizeof *k);
			
			k->id = i;
			k->type = type;
			
			k->x = x;
			k->y = y;
			
			//They face the player when they are spawned
			k->dir = -1;
			if (herox > x + 20) {
				k->dir = 1;
			}
			
			k->vsp = 0;
			k->grav = 0.2;
			
			k->state = 0;
			k->timer = 60 + (((rand() % 5) + 1) * 60);
			k->imageIndex = 0;
			
			k->hp = 2;
			//Shield Knight
			if (k->type == 1) {
				k->hp = 3;
			}
			
			k->invincible = 0;
			k->shieldhit = 0;
			
			k->mask.circle = 0;
			k->mask.unused = 0;
			k->mask.x = x + 4;
			k->mask.y = y + 8;
			k->mask.w = 32;
			k->mask.h = 32;
			
			e->data = k;
			e->enemyStep = knightStep;
			e->enemyDraw = knightDraw;
			e->type = 3;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void knightStep(Knight* k)
{
	if (k->shieldhit > 0) {
		k->shieldhit -= 1;
	}
	
	if (k->invincible > 0) {
		k->invincible -= 1;
	}
	
	if (k->state == 0) { //Walk
		k->imageIndex += 0.1;
		if (k->imageIndex >= 2) {
			k->imageIndex -= 2;
		}
		
		double spd = 1;
		if (k->type == 1) {
			spd = 0.5;
		}
		spd *= k->dir;
		
		k->x += spd;
		
		k->mask.x = k->x + 4;
		k->mask.y = k->y + 8;
		
		Mask emask;
		emask.circle = emask.unused = 0;
		emask.w = 16;
		emask.h = 32;
		emask.x = k->x + 12;
		emask.y = k->y + 8;
		
		//Turn when colliding with a wall
		if (checkTileCollision(1, emask)) {
			k->dir *= -1;
		}else{
			//Turn when on an edge
			k->mask.x += k->mask.w * k->dir;
			k->mask.y += 1;
			PHL_Rect collide = getTileCollision(1, k->mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, k->mask);
			}
			if (collide.x == -1) {
				k->dir *= -1;
			}
		}
		
		if (k->x + 20 >= 640 || k->x + 20 <= 0) {
			k->dir *= -1;
		}
		
		k->mask.x = k->x + 4;
		k->mask.y = k->y + 8;
		
		k->timer -= 1;
		if (k->timer <= 0) {
			k->state = 1;
			k->timer = 120;
			k->imageIndex = 0;
		}
	}
	else if (k->state == 1) { //Wait
		k->timer -= 1;
		if (k->timer <= 0) {
			k->state = 0;
			k->dir = 1;
			if (herox < k->x + 20) {
				k->dir = -1;
			}
			k->timer = 60 + (((rand() % 5) + 1) * 60);
		}		
	}
	
	//Green Sword Knight
	if (k->type == 0) {
		//Hit player
		Mask swordMask;
		swordMask.unused = 0;
		swordMask.circle = 0;
		swordMask.x = k->x + (24 * k->dir);
		swordMask.y = k->y + 20;
		swordMask.w = 40;
		swordMask.h = 10;
		
		if (checkCollision(getHeroMask(), swordMask)) {
			heroHit(30, k->x + 20);
		}
	}
	
	if (checkCollision(getHeroMask(), k->mask)) {
		heroHit(15, k->x + 20);
	}
	
	//Weapon collision
	int i;
	for (i = 0; i < MAX_WEAPONS; i++) {
		if (weapons[i] != NULL) {
			if (weapons[i]->cooldown == 0) {
				if (checkCollision(k->mask, weapons[i]->weaponMask)) {
					char gotHit = 1;
					
					int weapondir = weapons[i]->dir;
					weaponHit(weapons[i]);
					
					//Shield Collision
					if (k->type == 1) {							
						if (weapondir == k->dir * -1) {
							gotHit = 0;
							k->shieldhit = 15;
							PHL_PlaySound(sounds[sndHit03], CHN_WEAPONS);
						}
					}					
					
					if (gotHit == 1) {
						k->hp -= 1;
						k->invincible = 15;
						
						i = MAX_WEAPONS;
					}					
					
					if (k->hp <= 0) {
						knightDestroy(k);
					}					
				}
			}
		}	
	}
}

void knightDraw(Knight* k)
{
	if (k->invincible % 2 == 0) {
		int cx = 0, cy = 200;
		
		//Green Knight's Sword
		if (k->type == 0) {
			int swordimg = 0;
			if (k->dir == -1) {
				swordimg = 1;
			}
			int posx = 24, posy = 8;
			if ((int)k->imageIndex == 1) {
				posx -= 2;
				posy -= 2;
			}
			PHL_DrawSurfacePart(k->x + (posx * k->dir), k->y + posy, 160 + (swordimg * 40), 200, 40, 40, images[imgEnemies]);
		}
		
		//Shield Knight
		if (k->type == 1) {
			cx = 240;
		}

		if (k->dir == -1) {
			cx += 80;
		}
		PHL_DrawSurfacePart(k->x, k->y, cx + ((int)k->imageIndex * 40), cy, 40, 40, images[imgEnemies]);
	}
}

void knightDestroy(Knight* k)
{
	createEffect(2, k->x - 12, k->y - 6);
	spawnCollectable(k->x + 20, k->y);
	enemyDestroy(k->id);
}