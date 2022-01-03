#include "batboss.h"
#include "../game.h"
#include "../PHL.h"
#include "../hero.h"
#include "heads.h"
#include <stdlib.h>
#include <math.h>

int boss2flag = 5;

//void updateBatMask(Batboss* b);
void batbossStep(Batboss* b);
void batbossDraw(Batboss* b);

void createBatboss(int x, int y)
{
	if (flags[boss2flag] == 0) { //have not beaten boss 2
		PHL_FreeSurface(images[imgBoss]);
		images[imgBoss] = PHL_LoadQDA("boss03.bmp");
	
		int i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i] == NULL) {
				setBossRoom();
				
				Enemy* e = /*(Enemy*)*/malloc(sizeof *e);
				Batboss* b = /*(Batboss*)*/malloc(sizeof *b);
				b->id = i;
				
				b->x = x;
				b->y = y;
				
				b->vsp = 0;
				b->hsp = 0;
				b->grav = 0.1;
				
				b->imageIndex = 0;
				
				b->ypos = y;
				b->rot = 0;
				
				b->hp = 35;
				
				b->invincible = 0;
				b->state = 0;
				b->timer = 0;
				b->mode = 0; //0 for flame, 1 for tornado stomp
				
				/*
				b->mask.unused = b->mask.circle = 0;
				b->mask.w = 100;
				b->mask.h = 68;
				updateBatMask(b);
				*/
				//Setup phase
				b->state = 0;
				b->hsp = 2;
				b->ypos = b->y - 24;
				b->timer = 60;
				
				e->data = b;
				e->enemyStep = batbossStep;
				e->enemyDraw = batbossDraw;
				e->type = 41;
				
				enemies[i] = e;
				i = MAX_ENEMIES;
			}
		}
	}
}

void batbossStep(Batboss* b)
{
	char dead = 0;
	
	//Animate
	{
		//Wing flap
		if (b->state == 0 || b->state == 1 || b->state == 2 || b->state == 5 || b->state == 6) {
			b->imageIndex += 0.1;
			if (b->imageIndex >= 2) {
				b->imageIndex -= 2;
			}
		}
		//Twister
		if (b->state == 3 || b->state == 4) {
			b->imageIndex += 0.2;
			if (b->imageIndex >= 5) {
				b->imageIndex -= 3;
			}
		}
	}
	
	//Counters
	{
		if (b->timer > 0) {
			b->timer -= 1;
		}
		
		if (b->invincible > 0) {
			b->invincible -= 1;
		}
	}
	
	//Large vertical movement
	{
		if (b->state == 0 || b->state == 1) {
			b->rot += 2;
			if (b->rot >= 360) { b->rot -= 360; }
			
			b->y = b->ypos - (40 * sin(b->rot * 3.14159 / 180));
		}
	}
	
	//Small vertical movement
	{
		if (b->state == 2) {
			b->rot += 2;
			if (b->rot >= 360) { b->rot -= 360; }
			
			b->y = b->ypos - (20 * sin(b->rot * 3.14159 / 180));
		}
	}
	
	//Horizontal movement
	if (b->state == 0) {
		b->x += b->hsp;
		
		if (b->x >= 520 || b->x <= 120) { //Hit walls
			b->hsp *= -1;
		}		
		
		if (b->timer <= 0) {
			b->state = 1;
		}
	}
	//Slow to halt
	else if (b->state == 1) {
		b->x += b->hsp;
	
		if (b->x >= 520 || b->x <= 120) { //Hit walls
			b->hsp *= -1;
		}
	
		double rate = 0.03;
		if (b->hsp > 0) {
			b->hsp -= rate;
			if (b->hsp <= 0) { b->hsp = 0; }
		}
		else if (b->hsp < 0) {
			b->hsp += rate;
			if (b->hsp >= 0) { b->hsp = 0; }
		}
		
		if (b->hsp == 0 && b->rot <= 2) {
			b->state = 2;
			b->timer = 60;
		}
	}
	else if (b->state == 2) {
		if (b->timer == 1) {
			//Shoot flame
			int fx = b->x;
			int fy = b->y + 24;
			int fangle = (atan2(heroy - fy, fx - (herox - 20)) * 180 / 3.14159) + 270;
			createFireball(fx, fy, fangle, b->id);
			createFireball(fx, fy, fangle - 15, b->id);
			createFireball(fx, fy, fangle + 15, b->id);
			PHL_PlaySound(sounds[sndShot03], CHN_ENEMIES);
		}
		
		if (b->timer <= 0 && b->rot <= 2) {
			if (b->mode == 0) {
				b->state = 0;
				b->timer = 60;
				b->hsp = 2;
				b->mode = 1;
			}
			else{
				b->mode = 0;
				b->state = 3;
				b->imageIndex = 2;
				b->vsp = -4;
				PHL_PlaySound(sounds[sndShot06], CHN_ENEMIES);
			}
			
			if (herox < b->x) {
				b->hsp *= -1;
			}
		}
	}
	//Stomp
	else if (b->state == 3) {
		b->y += b->vsp;
		b->vsp += b->grav;
		if (b->vsp >= 6) { b->vsp = 6; }
		
		//Hit floor
		if (b->y >= 480 - 176) {
			b->y = 480 - 176;
			b->state = 4;
			b->timer = 120;
			quakeTimer = 30;
			PHL_PlaySound(sounds[sndHit04], CHN_ENEMIES);
			b->hsp = 1;
			if (b->x > herox) {
				b->hsp *= -1;
			}
		}
	}
	//Chase
	else if (b->state == 4) {
		b->x += b->hsp;
		
		if (b->timer <= 0 || b->x >= 520 || b->x <= 120) {
			b->state = 5;
			b->timer = 80 + (rand() % 61);
		}
	}
	//Rise
	else if (b->state == 5) {
		b->y -= 1;
		
		if (b->timer <= 0) {
			b->state = 0;
			b->ypos = b->y;
			b->rot = 0;
			b->timer = 60;
			
			b->hsp = 2;
			if (b->x > herox) {
				b->hsp *= -1;
			}
		}
	}
	//Death
	else if (b->state == 6) {
		b->y += 0.2;
		
		if (b->timer % 12 == 0) {
			createEffect(2, b->x - 64 + (rand() % 100), b->y + (rand() % 80));
		}
		
		if (b->timer <= 0) {
			dead = 1;
		}
	}
	
	if (b->state != 6) {
		//Setup Mask
		Mask mask;
		{
			mask.unused = mask.circle = 0;
			if (b->state == 3 || b->state == 4) {
				mask.w = 64;
				mask.h = 96;
				mask.y = b->y;
			}else{
				mask.w = 100;
				mask.h = 68;
				mask.y = b->y + 18;
			}
			mask.x = b->x - (mask.w / 2);
		}
		
		//Hit Player
		{
			if (checkCollision(mask, getHeroMask())) {
				heroHit(30, b->x);
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
							//Hit
							b->invincible = 15;
							b->hp -= 1;
							
							i = MAX_WEAPONS;
						}
					}
				}	
			}
		}
		
		if (b->hp <= 0) {
			b->state = 6;
			b->timer = 180;
			b->invincible = 200;
		}
	}
	
	//Destroy object
	{
		if (dead == 1) {
			enemyDestroy(b->id);
			bossDefeatedFlag = 1;
			roomSecret = 1;

			flags[boss2flag] = 1;
			PHL_StopMusic();
		}
	}
	
}

void batbossDraw(Batboss* b)
{
	if (b->invincible % 2 == 0) {
		PHL_DrawSurfacePart(b->x - 64, b->y, (int)b->imageIndex * 128, 0, 128, 96, images[imgBoss]);
	}
}