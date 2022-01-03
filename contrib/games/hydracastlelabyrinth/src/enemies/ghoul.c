#include "ghoul.h"
#include "../game.h"
#include "../enemy.h"
#include "../PHL.h"
#include "../hero.h"
#include <stdlib.h>

void ghoulStep(Ghoul* g);
void ghoulDraw(Ghoul* g);

void createGhoul(int x, int y, int type)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Ghoul* g = malloc(sizeof *g);
			g->id = i;
			g->hp = 2;
			
			g->x = x;
			g->y = y;
			
			g->vsp = 0;
			g->grav = 0.1;
			
			g->dir = 0;
			g->type = type;
			g->onground = 0;
			
			g->timer = 0;
			g->state = 0;
			g->invincible = 0;
			
			g->imageIndex = 0;
			
			g->mask.circle = 0;
			g->mask.unused = 1;
			g->mask.w = 24;
			g->mask.h = 32;
			g->mask.x = g->x + ((40 - g->mask.w) / 2);
			g->mask.y = g->y + (40 - g->mask.h);
			
			e->data = g;
			e->enemyStep = ghoulStep;
			e->enemyDraw = ghoulDraw;
			e->type = 18;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
	
}

void ghoulStep(Ghoul* g)
{
	if (g->invincible > 0) {
		g->invincible -= 1;
	}
	
	if (g->state == 0) { //Wait
		Mask area;
		area.unused = area.circle = 0;
		area.w = 280;
		area.h = 80;
		area.x = g->x - 120;
		area.y = g->y - 20;
		
		if (checkCollisionXY(area, herox, heroy + 20) == 1) {
			g->state = 1;
			g->mask.unused = 0;
			g->imageIndex = 0;
			
			g->dir = 1;
			if (herox < g->x + 20) {
				g->dir = -1;
			}
		}
	}
	else if (g->state == 1) { //Pop-up
		g->imageIndex += 0.16;
		
		if (g->imageIndex >= 4) {
			g->state = 2;
			g->vsp = -1;
			g->imageIndex = 0;
			PHL_PlaySound(sounds[sndPi05],CHN_ENEMIES);
		}
	}
	else if (g->state == 2) { //Walking
		g->mask.unused = 0;
		if (g->onground == 0) {
			//Vertical movement
			g->y += g->vsp;
			g->vsp += g->grav;
			
			g->mask.y = g->y + (40 - g->mask.h);
			
			PHL_Rect collide = getTileCollision(1, g->mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, g->mask);
			}
			if (collide.x != -1) {
				g->onground = 1;
				g->vsp = 0;
				g->y = collide.y - 40;
				g->mask.y = g->y + (40 - g->mask.h);
			}
		}
		
		g->imageIndex += 0.1;
		if (g->imageIndex >= 2) {
			g->imageIndex -= 2;
		}

		double hsp = 1;
		
		if ((int)g->imageIndex == 0) {
			hsp = 0.5;
		}
		
		//Purple
		if (g->type == 1) {
			hsp *= 2;
		}
		
		g->x += hsp * g->dir;
		g->mask.x = g->x + ((40 - g->mask.w) / 2);
		
		if (g->onground == 1) {
			if ((g->x < -20 || g->x > 660) || checkTileCollision(1, g->mask) == 1) {
				g->dir *= -1;
				
				PHL_Rect collide = getTileCollision(1, g->mask);
				if (collide.x != -1) {
					g->x = collide.x + (40 * g->dir);
				}
			}
			else {
				//check on ledge
				g->mask.w = 5;
				if (g->dir == 1) {
					g->mask.x = g->x + 30;
				}
				if (g->dir == -1) {
					g->mask.x = g->x + 5;
				}
				g->mask.y += 20;
				
				if (checkTileCollision(1, g->mask) == 0 && checkTileCollision(3, g->mask) == 0) {
					g->dir *= -1;
				}
				g->mask.w = 24;
				g->mask.x = g->x + ((40 - g->mask.w) / 2);
				g->mask.y = g->y + (40 - g->mask.h);
			}
		}
	}
	
	g->mask.x = g->x + ((40 - g->mask.w) / 2);
	g->mask.y = g->y + (40 - g->mask.h);
	
	//Hit Player
	{
		if (checkCollision(g->mask, getHeroMask())) {
			if (heroHit(10, g->x + 20) == 1 && g->type == 1) {
				heroPoison();
			}
		}
	}
	
	//Weapon Collision
	{
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (weapons[i]->cooldown == 0) {
					if (checkCollision(g->mask, weapons[i]->weaponMask)) {
						weaponHit(weapons[i]);
						
						g->hp -= 1;
						g->invincible = 15;
						//Death
						if (g->hp <= 0) {						
							createEffect(2, g->x - 12, g->y - 6);
							spawnCollectable(g->x + 20, g->y);
							enemyDestroy(g->id);
						}
						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}
	
}

void ghoulDraw(Ghoul* g)
{
	if (g->state != 0 && g->invincible % 2 == 0) {
		int cx = (int)g->imageIndex * 40,
			cy = 160;
			
		if (g->state == 1) {
			cx += 160;
		}else{
			if (g->dir == -1) {
				cx += 80;
			}
		}
		
		//Purple palette
		cy += 160 * g->type;
			
		PHL_DrawSurfacePart(g->x, g->y, cx, cy, 40, 40, images[imgEnemies]);
	}
}