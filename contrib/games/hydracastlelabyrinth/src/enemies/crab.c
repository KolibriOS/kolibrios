#include "crab.h"
#include "../PHL.h"
#include "../enemy.h"
#include "../game.h"
#include "../hero.h"
#include <math.h>
#include <stdlib.h>

int boss3flag = 13;

void crabStep(Crab* c);
void crabDraw(Crab* c);
void updateCrabMask(Crab* c);
void crabDestroy(Crab* c);

void electricityStep(Electricity* e);
void electricityDraw(Electricity* e);

void createCrab(int x, int y)
{
	if (flags[boss3flag] == 0) { //have not beaten boss 3
		PHL_FreeSurface(images[imgBoss]);
		images[imgBoss] = PHL_LoadQDA("boss06.bmp");
	
		int i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i] == NULL) {
				//Boss start
				setBossRoom();
				
				Enemy* e = /*(Enemy*)*/malloc(sizeof(Enemy));
				Crab* c = /*(Crab*)*/malloc(sizeof(Crab));
				c->id = i;
				
				//c->hp = 1;
				c->hp = 35;
				c->invincible = 0;
				
				c->x = x;
				c->y = y;
				
				c->vsp = 0;
				c->hsp = 0;
				
				c->imageIndex = 0;
				
				c->state = 0;
				c->timer = 0;
				c->counter = 0;
				
				c->mask.unused = 0;
				c->mask.circle = 1;
				c->mask.w = 33;
				c->mask.h = 33;
				updateCrabMask(c);
				
				//Setup phase
				c->timer = 60;
				
				e->data = c;
				e->enemyStep = crabStep;
				e->enemyDraw = crabDraw;
				e->type = 42;
				
				enemies[i] = e;
				i = MAX_ENEMIES;
			}
		}
	}
}

void crabStep(Crab* c)
{
	char dead = 0;
	double grav = 0.15;
	
	if (c->invincible > 0) {
		c->invincible -= 1;
	}
	
	//Wait
	if (c->state == 0)
	{
		c->imageIndex = 0;
		
		if (c->timer <= 0) {
			c->timer = 0;
			if (c->counter == 2 || c->counter == 5) { //Goto roll
				c->state = 3;
				if (c->counter == 5) {
					c->counter = 0;
				}else{
					c->counter = 3;
				}
			}else if (c->counter == 3) {
				c->state = 2;
			}else{
				c->state = 1; //Goto shoot
			}		
		}else{
			c->timer -= 1;
		}
	}
	//Shoot Electric orbs
	else if (c->state == 1)
	{
		//Create orbs
		if (c->timer == 0) {
			PHL_PlaySound(sounds[sndPi05], CHN_ENEMIES);
			
			double angle = (atan2(heroy + 20 - (c->y + 60), c->x - (herox - 20)) * 180 / 3.14159) + 270;
			createElectricity(c->x, c->y + 60, angle - 45, c->id);
			createElectricity(c->x, c->y + 60, angle - 22.5, c->id);
			createElectricity(c->x, c->y + 60, angle, c->id);
			createElectricity(c->x, c->y + 60, angle + 22.5, c->id);
			createElectricity(c->x, c->y + 60, angle + 45, c->id);
		}
		
		if (c->timer >= 20) {
			c->state = 2;
			c->timer = 0;
		}else{
			c->timer += 1;
		}
	}
	//Leap
	else if (c->state == 2)
	{
		c->imageIndex = 1;
		
		//Hopping down or hopping up
		int hopup = 1;
		if (c->counter > 2) {
			hopup = 0;
		}
		
		//Jump
		if (c->timer == 0) {
			PHL_PlaySound(sounds[sndJump02], CHN_ENEMIES);
			
			c->vsp = -6.5;
			if (hopup == 0) {
				c->vsp = -1.5;
			}
			c->timer = 1;
		}
		
		//Vertical velocity
		c->y += c->vsp;
		c->vsp += grav;
		
		if (c->vsp >= 6) {
			c->vsp = 6;
		}
		
		//Check if onground
		if ((hopup == 1 && c->vsp > 0) || (hopup == 0 && c->vsp >= 6)) {
			Mask area;
			area.unused = area.circle = 0;
			area.w = 40;
			area.h = 10;
			area.x = c->x - (area.w / 2);
			area.y = c->y + (80 - area.h);
			
			PHL_Rect collide = getTileCollision(1, area);
			if (collide.x != -1) {
				c->y = collide.y - 80;
				c->state = 0;
				c->counter += 1;
				c->timer = 25;
				if (c->counter == 2 || c->counter == 5) {
					c->timer = 3;
				}				
			}
		}
	}
	//Roll hop
	else if (c->state == 3)
	{
		//Animate
		if (c->hsp > 0) {
			c->imageIndex += 0.25;
		}
		if (c->hsp < 0) {
			c->imageIndex -= 0.25;
		}
		if (c->imageIndex < 2) { c->imageIndex += 4; }
		if (c->imageIndex >= 6) { c->imageIndex -= 4; }
		
		
		if (c->timer == 0) {
			PHL_PlaySound(sounds[sndHit04], CHN_ENEMIES);
			
			c->timer = 1;
			c->vsp = -1.5;
			c->imageIndex = 2;
			if (c->x > 320) {
				c->hsp = -8;
			}else{
				c->hsp = 8;
			}
		}
		
		//Movement
		c->y += c->vsp;
		c->vsp += grav;
		
		//Check if onground
		if (c->vsp > 0) {
			Mask area;
			area.unused = area.circle = 0;
			area.w = 40;
			area.h = 10;
			area.x = c->x - (area.w / 2);
			area.y = c->y + (80 - area.h);
			
			PHL_Rect collide = getTileCollision(1, area);
			if (collide.x != -1) {
				c->y = collide.y - 80;
				c->state = 4;			
			}
		}
	}
	//Roll
	if (c->state == 4)
	{
		//Animate
		if (c->hsp > 0) {
			c->imageIndex += 0.25;
		}
		if (c->hsp < 0) {
			c->imageIndex -= 0.25;
		}
		if (c->imageIndex < 2) { c->imageIndex += 4; }
		if (c->imageIndex >= 6) { c->imageIndex -= 4; }
		
		//Movement
		c->x += c->hsp;
		
		//Collide with wall
		Mask area;
		area.unused = area.circle = 0;
		area.w = area.h = c->mask.w * 2;
		area.x = c->x - c->mask.w;
		area.y = c->y + (40 - c->mask.h);
		
		if (checkTileCollision(1, area) == 1) {
			c->state = 5;
			c->timer = 0;
		}
	}
	//Bounce off wall
	if (c->state == 5)
	{
		if (c->timer == 0) {
			PHL_PlaySound(sounds[sndHit04], CHN_ENEMIES);
			
			c->timer = 1;
			c->vsp = -2;
			c->hsp = 2;
			if (c->x > 320) {
				c->hsp *= -1;
			}
		}
		
		c->imageIndex = 1;
		
		c->x += c->hsp;
		
		c->y += c->vsp;
		c->vsp += grav;
		
		//Check if onground
		if (c->vsp > 0) {
			Mask area;
			area.unused = area.circle = 0;
			area.w = 40;
			area.h = 10;
			area.x = c->x - (area.w / 2);
			area.y = c->y + (80 - area.h);
			
			PHL_Rect collide = getTileCollision(1, area);
			if (collide.x != -1) {
				c->y = collide.y - 80;
				c->state = 0;
				c->timer = 65;
			}
		}		
	}
	//Death
	else if (c->state == 6)
	{
		c->imageIndex = 1;
		
		c->y += 0.2;
		
		c->timer -= 1;
		
		if (c->timer % 12 == 0) {
			createEffect(2, c->x - 64 + (rand() % 100), c->y + (rand() % 80));
		}
		
		if (c->timer <= 0) {
			crabDestroy(c);
			dead = 1;
		}
	}
	
	if (dead == 0) {
		if (c->state != 6) {
			//Update Mask
			c->mask.x = c->x;
			c->mask.y = c->y + 40;
			
			//Weapon collision
			int i;
			for (i = 0; i < MAX_WEAPONS; i++) {
				if (weapons[i] != NULL) {
					if (weapons[i]->cooldown == 0) {
						if (checkCollision(c->mask, weapons[i]->weaponMask)) {
							weaponHit(weapons[i]);
							c->invincible = 15;
							c->hp -= 1;	
							i = MAX_WEAPONS;
						}
					}
				}	
			}				
			
			//Hit Player
			if (checkCollision(c->mask, getHeroMask())) {
				heroHit(30, c->x);
			}
			
			//Die
			if (c->hp <= 0) {
				c->state = 6;
				c->timer = 180;
				c->invincible = 200;
			}
		}
	}
}

void crabDraw(Crab* c)
{
	if (c->invincible % 2 == 0) {
		PHL_DrawSurfacePart(c->x - 40, c->y, (int)c->imageIndex * 80, 0, 80, 80, images[imgBoss]);
	}
}

void updateCrabMask(Crab* c)
{
	c->mask.x = c->x;
	c->mask.y = c->y + 40;
}

void crabDestroy(Crab* c)
{
	enemyDestroy(c->id);
	bossDefeatedFlag = 1;
	roomSecret = 1;

	flags[boss3flag] = 1;
	PHL_StopMusic();
}


void createElectricity(int x, int y, double angle, int minid)
{
	int i;
	for (i = minid; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = /*(Enemy*)*/malloc(sizeof *e);
			Electricity* el = /*(Electricity*)*/malloc(sizeof *el);
			el->id = i;
			
			el->x = x;
			el->y = y;
			
			//Fix angle
			if (angle < 0) {
				angle += 360;
			}
			if (angle >= 360) {
				angle -= 360;
			}
			
			el->angle = angle;			
			el->imageIndex = 0;
			
			el->mask.unused = 0;
			el->mask.circle = 1;
			el->mask.w = 16;
			el->mask.h = 16;
			el->mask.x = x;
			el->mask.y = y;
			
			e->data = el;
			e->enemyStep = electricityStep;
			e->enemyDraw = electricityDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
	/*
	int thisid = -1;
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			if (i <= minid) {
				thisid = i;
				i = MAX_ENEMIES;
			}else{
				i = MAX_ENEMIES;
			}
		}
	}
	
	if (thisid == -1) {
		for (i = minid + 1; i < MAX_ENEMIES; i++) {
			if (enemies[i] == NULL) {
				enemies[i] = enemies[minid];
				Crab* c = enemies[i]->data;
				c->id = i;
				thisid = minid;
				i = MAX_ENEMIES;
			}
		}
	}
		
	if (thisid != -1) {
		Enemy* e = (Enemy*)malloc(sizeof(Enemy));
		Electricity* el = (Electricity*)malloc(sizeof(Electricity));
		el->id = thisid;
		
		el->x = x;
		el->y = y;
		
		//Fix angle
		if (angle < 0) {
			angle += 360;
		}
		if (angle >= 360) {
			angle -= 360;
		}
		
		el->angle = angle;			
		el->imageIndex = 0;
		
		el->mask.unused = 0;
		el->mask.circle = 1;
		el->mask.w = 16;
		el->mask.h = 16;
		el->mask.x = x;
		el->mask.y = y;
		
		e->data = el;
		e->enemyStep = electricityStep;
		e->enemyDraw = electricityDraw;
		e->type = -1;
		
		enemies[thisid] = e;
	}
	*/
}

void electricityStep(Electricity* e)
{
	double spd = 3;
	e->x += spd * sin(e->angle * 3.14159 / 180);
	e->y += spd * cos(e->angle * 3.14159 / 180);
	
	//Update Mask
	e->mask.x = e->x;
	e->mask.y = e->y;
	
	//Collide with Shield
	if (checkCollision(shieldMask, e->mask) == 1) {
		createEffect(1, e->x - 20, e->y - 20);
		PHL_PlaySound(sounds[sndHit07], CHN_EFFECTS);
		enemyDestroy(e->id);
	}else{
		//Collide with Hero
		if (checkCollision(getHeroMask(), e->mask) == 1) {
			if (heroHit(25, e->x) == 1) {
				heroStun();
			}
		}
	}
	
	//Animate
	e->imageIndex += 0.25;
	if (e->imageIndex >= 3) {
		e->imageIndex -= 3;
	}
		
	//Outside of screen
	if (e->x < -20 || e->x > 660 || e->y < -20 || e->y > 500) {
		enemyDestroy(e->id);
	}
}

void electricityDraw(Electricity* e)
{
	PHL_DrawSurfacePart(e->x - 20, e->y - 20, 40 + ((int)e->imageIndex * 40), 0, 40, 40, images[imgMisc20]);
}