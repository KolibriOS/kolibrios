#include "lolidra.h"
#include "../enemy.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>
#include <math.h>

int boss5flag = 38;

void lolidraDestroy(Lolidra* l);
int getNumOfMinions();

void createLolidra(int x, int y)
{
	if (flags[boss5flag] == 0) { //have not beaten boss 5
		PHL_FreeSurface(images[imgBoss]);
		images[imgBoss] = PHL_LoadQDA("boss05.bmp");
	
		int i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i] == NULL) {
				//Boss start
				setBossRoom();
				
				Enemy* e = malloc(sizeof *e);
				Lolidra* l = malloc(sizeof *l);
				l->id = i;
				
				l->x = x;
				l->y = y;
				
				l->positionY = l->y;
				
				l->imageIndex = 0;
				l->hoverRot = 0;
				
				l->hp = 100;
				//l->hp = 1;
				l->state = 0;				
				l->invincible = 0;
				l->visible = 1;
				
				l->timer = 0;
				l->counter = 0;
				
				l->mask.unused = 0;
				l->mask.circle = 1;
				l->mask.w = 46;
				l->mask.h = 0;
				l->mask.x = l->x;
				l->mask.y = l->y;
				
				e->data = l;
				e->enemyStep = lolidraStep;
				e->enemyDraw = lolidraDraw;
				e->type = 44;
				
				enemies[i] = e;
				i = MAX_ENEMIES;
			}
		}
	}
}

void lolidraStep(Lolidra* l)
{
	char dead = 0;
	
	l->imageIndex += 0.1;
	if (l->imageIndex >= 3) {
		l->imageIndex -= 3;
	}
	
	if (l->invincible > 0) {
		l->invincible -= 1;
	}
	
	//Spawn minions
	if (l->state == 0)
	{		
		if (l->counter < 5) {
			l->counter += 1;
		}else{
			if (getNumOfMinions() < 10) {
				l->counter = 0;
				PHL_PlaySound(sounds[sndPi02], CHN_ENEMIES);
				createMinion(l->x, l->y - 10);
			}
		}
		
		l->timer += 1;
		if (l->timer >= 600){
			l->counter = 0;
			l->timer = 0;
			l->state = 1;
			l->invincible = 20;
		}
	}
	//Disappear
	else if (l->state == 1 || l->state == 3)
	{
		if (l->invincible <= 0) {
			l->visible = 0;
		}

		if (l->timer == 0) {
			PHL_PlaySound(sounds[sndPi10], CHN_ENEMIES);
		}
		
		l->timer += 1;
		if (l->timer >= 330) {
			PHL_PlaySound(sounds[sndPi03], CHN_ENEMIES);
			l->timer = 0;
			l->visible = 1;
			l->invincible = 20;			
			l->x = herox;
			l->positionY = heroy - 40;
			
			if (l->state == 1) {
				l->state = 2;
			}
			if (l->state == 3) {
				l->state = 0;
			}
		}
	}
	//Pop-up
	else if (l->state == 2)
	{
		l->timer += 1;
		if (l->timer >= 180) {
			l->timer = 0;
			l->state = 3;
			l->invincible = 20;
		}
	}
	//Death
	else if (l->state == 4)
	{		
		l->y += 0.2;
		
		l->timer -= 1;
		l->invincible -= 1;
		
		if (l->timer % 12 == 0) {
			createEffect(2, l->x - 64 + (rand() % 128), l->y - 64 + (rand() % 128));
		}
		
		if (l->timer <= 0) {
			lolidraDestroy(l);
			dead = 1;
		}
	}
	
	if (dead == 0) 
	{
		if (l->state != 4) {
			//Hover
			l->hoverRot += 5;
			if (l->hoverRot >= 360) {
				l->hoverRot -= 360;
			}
			l->y = l->positionY + (5 * sin(l->hoverRot * 3.14159 / 180));		
		
			//Update Mask
			l->mask.x = l->x;
			l->mask.y = l->y;
			
			//Collisions
			if (l->visible == 1) {
				//Collide with Hero
				if (checkCollision(getHeroMask(), l->mask) == 1) {
					heroHit(30, l->x);
				}
				
				//Weapon collision
				int i;
				for (i = 0; i < MAX_WEAPONS; i++) {
					if (weapons[i] != NULL) {
						if (weapons[i]->cooldown == 0) {
							if (checkCollision(l->mask, weapons[i]->weaponMask)) {
								weaponHit(weapons[i]);
								l->invincible = 15;
								l->hp -= 1;
								
								//Die
								if (l->hp <= 0) {
									l->state = 4;
									l->timer = 180;
									l->invincible = 200;
								}
								
								i = MAX_WEAPONS;
							}
						}
					}	
				}
			}
		}
	}
	
}

void lolidraDraw(Lolidra* l)
{
	if (l->visible == 1 && l->invincible % 2 == 0) {
		PHL_DrawSurfacePart(l->x - 64, l->y - 74, ((int)l->imageIndex) * 128, 0, 128, 128, images[imgBoss]);
	}
}

void lolidraDestroy(Lolidra* l)
{
	enemyDestroy(l->id);
	bossDefeatedFlag = 1;
	roomSecret = 1;

	flags[boss5flag] = 1;
	PHL_StopMusic();
}

//Minions
void createMinion(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Minion* m = malloc(sizeof *m);
			m->id = i;
			
			m->state = 0;
			m->timer = 0;
			
			m->x = x;
			m->y = y;
			
			m->positionY = m->y;
			
			m->dir = rand() % 360;
			m->spd = 8;
			
			m->imageIndex = 0;
			
			m->mask.circle = 1;
			m->mask.unused = 0;
			m->mask.w = 10;
			m->mask.x = 0;
			m->mask.y = 0;
			
			e->data = m;
			e->enemyStep = minionStep;
			e->enemyDraw = minionDraw;
			e->type = 23;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void minionStep(Minion* m)
{
	char dead = 0;
	
	m->imageIndex += 0.2;
	if (m->imageIndex >= 2) {
		m->imageIndex -= 2;
	}
	
	//Slow down
	if (m->state == 0)
	{
		if (m->spd > 0) {
			m->spd -= 0.3;
		}
		
		if (m->spd <= 0) {
			m->positionY = m->y;
			m->spd = 0;
			m->dir = 0;
			m->state = 1;
		}
	}
	//Hover
	else if (m->state == 1)
	{
		//Hover
		m->dir += 5;
		if (m->dir >= 360) {
			m->dir -= 360;
		}
		m->y = m->positionY + (10 * sin(m->dir * 3.14159 / 180));
		
		m->timer += 1;
		if (m->timer >= 120) {
			m->timer = 0;
			m->state = 2;
			m->spd = (rand() % 2) + 1;
			m->dir = (atan2(heroy + 20 - m->y, m->x - herox) * 180 / 3.14159) + 270;
		}
	}
	//Suicide
	else if (m->state == 2)
	{
		m->timer += 1;
		if (m->timer >= 120) {
			createEffect(5, m->x, m->y);
			enemyDestroy(m->id);
			dead = 1;
		}
	}
	
	if (dead == 0)
	{
		//Movement
		if (m->spd != 0) {
			m->x += m->spd * sin(m->dir * 3.14159 / 180);
			m->y += m->spd * cos(m->dir * 3.14159 / 180);
		}
		
		//Update Mask
		m->mask.x = m->x;
		m->mask.y = m->y;
		
		//Collide with Hero
		if (checkCollision(getHeroMask(), m->mask) == 1) {
			if (heroHit(10, m->x) == 1) {
				heroPoison();
			}
		}
		
		//Weapon collision
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (checkCollision(m->mask, weapons[i]->weaponMask)) {
					weaponHit(weapons[i]);
					createEffect(2, m->x - 32, m->y - 32);
					enemyDestroy(m->id);
					
					i = MAX_WEAPONS;
				}
			}	
		}
	}
}

void minionDraw(Minion* m)
{
	PHL_DrawSurfacePart(m->x - 32, m->y - 32, ((int)m->imageIndex) * 64, 128, 64, 64, images[imgBoss]);
}

int getNumOfMinions()
{
	int result = 0;
	
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] != NULL) {
			if (enemies[i]->type == 23) {
				result += 1;
			}
		}
	}
	
	return result;
}