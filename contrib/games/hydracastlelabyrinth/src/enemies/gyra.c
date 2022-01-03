#include "gyra.h"
#include "../game.h"
#include "../enemy.h"
#include "../hero.h"
#include <stdlib.h>
#include <math.h>

void gyraStep(Gyra* g);
void gyraDraw(Gyra* g);
void gyraDestroy(Gyra* g);

int boss4flag = 21;

void createGyra(int x, int y)
{
	if (flags[boss4flag] == 0) { //have not yet beaten boss 4
		PHL_FreeSurface(images[imgBoss]);
		images[imgBoss] = PHL_LoadQDA("boss02.bmp");
		
		int i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i] == NULL) {
				//Boss start
				setBossRoom();
				
				Enemy* e = malloc(sizeof *e);
				Gyra* g = malloc(sizeof *g);
				
				g->id = i;				
				g->hp = 50;
				//g->hp = 1;
				
				g->x = x;
				g->y = y;
				
				g->targx = g->x;
				g->targy = g->y;
								
				g->state = 0;
				g->timer = 260;
				g->counter = 0;
				
				g->invincible = 0;
				g->dir = 0;		
				g->imageIndex = 0;
				
				//Setup
				g->targx = g->x - 32;
				g->targy = g->y + 64;
				g->dir = 160;
				
				g->x = g->targx + (80 * sin(g->dir * 3.14159 / 180));
				g->y = g->targy + (80 * cos(g->dir * 3.14159 / 180));
				
				int a;
				for (a = 0; a < 144; a++) {
					g->xrecord[a] = g->x;
					g->yrecord[a] = g->y;
				}
				
				e->data = g;
				e->enemyStep = gyraStep;
				e->enemyDraw = gyraDraw;
				e->type = 43;
				
				enemies[i] = e;
				i = MAX_ENEMIES;
				
			}
		}
	}
}

void gyraStep(Gyra* g)
{	
	//Animate
	g->imageIndex += 0.1;
	if (g->imageIndex >= 2) {
		g->imageIndex -= 2;
	}
	
	int pattern[6] = {0, 1, 2, 1, 0, 2};
	
	//Move in a circle
	if (g->state == 0)
	{
		int len = 80;
		
		//Setup
		if (g->timer == 0) {
			g->targx = g->x + (len * sin((g->dir + 90) * 3.14159 / 180));
			g->targy = g->y + (len * cos((g->dir + 90) * 3.14159 / 180));
			g->dir -= 90;
			g->timer = 250;
		}
		
		g->dir += 1.5;
		if (g->dir >= 360) {
			g->dir -= 360;
		}
		
		g->x = g->targx + (len * sin(g->dir * 3.14159 / 180));
		g->y = g->targy + (len * cos(g->dir * 3.14159 / 180));
		
		g->timer -= 1;
		if (g->timer <= 0) {
			g->counter += 1;
			g->state = pattern[g->counter];
			/*	
			if (g->state != 1 && (g->x < 40 || g->x > 600 || g->y < 40 || g->y > 440)) {
				g->state = 1;
			}*/
			
			g->timer = 0;
		}
	}
	//Attack
	else if (g->state == 1)
	{
		//Setup
		if (g->timer == 0) {
			g->targx = herox;
			g->targy = heroy + 20;
			g->dir += 90;
			g->timer = 320;
		}
		
		double spd = 2;
		double diralt = 1.2;
		
		double targdir = (atan2(g->targy - g->y, g->x - g->targx) * 180 / 3.14159) + 270;
		
		targdir = g->dir - targdir;
		while (targdir >= 360) { targdir -= 360; }
		while (targdir < 0) { targdir += 360; }
		
		if (targdir > 180) {
			g->dir += diralt;
		}
		if (targdir < 180) {
			g->dir -= diralt;
		}
		
		//Movement
		g->x += spd * sin(g->dir * 3.14159 / 180);
		g->y += spd * cos(g->dir * 3.14159 / 180);
		
		//Get (close) to targ coords
		g->timer -= 1;
		if (g->timer <= 0 || sqrt( pow(g->x - g->targx, 2) + pow(g->y - g->targy, 2) ) <= spd * 2) {
			g->counter += 1;
			if (g->counter >= 5) {
				g->counter = 0;
			}
			g->state = pattern[g->counter];
			g->timer = 0;
		}
	}
	//Oval movement
	else if (g->state == 2)
	{
		int wlen = 120,
			hlen = 80;
			
		//Setup
		if (g->timer == 0) {
			g->targx = g->x + (wlen * sin((g->dir - 90) * 3.14159 / 180));
			g->targy = g->y + (hlen * cos((g->dir - 90) * 3.14159 / 180));
			g->dir += 90;
			g->timer = 200;
		}
		
		g->dir -= 1.5;
		if (g->dir < 0) {
			g->dir += 360;
		}
		
		g->x = g->targx + (wlen * sin(g->dir * 3.14159 / 180));
		g->y = g->targy + (hlen * cos(g->dir * 3.14159 / 180));
		
		g->timer -= 1;
		if (g->timer <= 0) {
			g->counter += 1;
			if (g->counter >= 5) {
				g->counter = 0;
			}
			g->state = pattern[g->counter];
			/*
			if (g->state != 1 && (g->x < 40 || g->x > 600 || g->y < 40 || g->y > 440)) {
				g->state = 1;
				g->timer = 0;
			}*/
		}
	}
	
	//Death
	if (g->state == 3)
	{
		g->timer -= 1;
		if (g->timer <= 0) {
			g->timer = 12;
			
			int cx = g->xrecord[128 - (g->counter * 16)],
				cy = g->yrecord[128 - (g->counter * 16)];
			
			createEffect(2, cx - 32, cy - 32);
			
			g->counter += 1;
			if (g->counter == 9) {
				gyraDestroy(g);
			}
		}
	}else{
		//Update tail record
		int i;
		for (i = 142; i >= 0; i--) {
			g->xrecord[i + 1] = g->xrecord[i];
			g->yrecord[i + 1] = g->yrecord[i];
		}
		g->xrecord[0] = g->x;
		g->yrecord[0] = g->y;
		
		//for (i = 8; i >= 0; i--) {
		for (i = 0; i <= 8; i++) {
			int cx = g->x, cy = g->y;
			
			if (i != 0) {
				cx = g->xrecord[i * 16];
				cy = g->yrecord[i * 16];
			}
				
			Mask mask;
			mask.unused = 0;
			mask.circle = 1;
			mask.x = cx;
			mask.y = cy;
			mask.w = mask.h = 28;
			
			int a;
			for (a = 0; a < MAX_WEAPONS; a++) {
				if (weapons[a] != NULL) {
					if (weapons[a]->cooldown == 0) {
						if (checkCollision(mask, weapons[a]->weaponMask)) {
							g->invincible = -15;
							weaponHit(weapons[a]);
							
							if (i == 8) {
								g->hp -= 1;
								g->invincible = 15;
							}else{
								PHL_PlaySound(sounds[sndHit03], CHN_WEAPONS);
							}
							
							a = MAX_WEAPONS;
						}
					}
				}	
			}
			
			//Hit player
			if (checkCollision(getHeroMask(), mask)) {
				if (heroHit(30, mask.x) && i == 0) {
					heroPoison();
				}
			}
		}
				
		//Death
		if (g->hp <= 0) {
			g->state = 3;
			g->timer = 0;
			g->counter = 0;
			g->invincible = 200;
		}
	}
	
	if (g->invincible > 0) {
		g->invincible -= 1;
	}
	if (g->invincible < 0) {
		g->invincible += 1;
	}
	
}

void gyraDraw(Gyra* g)
{
	if (g->invincible <= 0 || g->invincible % 2 == 0) {
		//Draw Tail Tip
		if (g->state != 3 || g->counter <= 0) {
			PHL_DrawSurfacePart(g->xrecord[126] - 40, g->yrecord[126] - 40, 320 + ((int)g->imageIndex * 80), 0, 80, 80, images[imgBoss]);
		}
		
		//Draw Tail
		int i;
		for (i = 7; i > 0; i--) {
			if (g->state != 3 || g->counter <= (7 - i) + 1) {
				PHL_DrawSurfacePart(g->xrecord[i * 16] - 40, g->yrecord[i * 16] - 40, 160 + ((int)g->imageIndex * 80), 0, 80, 80, images[imgBoss]);
			}
		}
		
		//Draw Head
		PHL_DrawSurfacePart(g->x - 40, g->y - 40, (int)g->imageIndex * 80, 0, 80, 80, images[imgBoss]);
	}
	
	//PHL_DrawRect(g->targx, g->targy, 10, 10, PHL_NewRGB(255, 255, 255));	
	//heroAmmo = g->state;
	
	/*
	int i;
	for (i = 8; i >= 0; i--) {
		int cx = g->x, cy = g->y;
		
		if (i != 0) {
			cx = g->xrecord[i * 16];
			cy = g->yrecord[i * 16];
		}
			
		PHL_DrawRect(cx, cy, 10, 10, PHL_NewRGB(255, 255, 255));
	}
	*/
}

void gyraDestroy(Gyra* g)
{
	enemyDestroy(g->id);
	bossDefeatedFlag = 1;
	roomSecret = 1;

	flags[boss4flag] = 1;
	PHL_StopMusic();
}