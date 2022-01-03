#include "devil.h"
#include "../game.h"
#include "../PHL.h"
#include "../hero.h"
#include <stdlib.h>
#include <math.h>

int boss6flag = 31;

void devilStep(Devil* d);
void devilDraw(Devil* d);

void orbStep(Orb* o);
void orbDraw(Orb* o);

void createDevil(int x, int y)
{
	if (flags[boss6flag] == 0) { //have not beaten boss 6
		PHL_FreeSurface(images[imgBoss]);
		images[imgBoss] = PHL_LoadQDA("boss04.bmp");
	
		int i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i] == NULL) {
				setBossRoom();
				
				Enemy* e = /*(Enemy*)*/malloc(sizeof *e);
				Devil* d = /*(Devil*)*/malloc(sizeof *d);
				d->id = i;
				
				d->x = x;
				d->y = y;
				
				d->ystart = d->y;
				d->newystart = d->ystart;
				d->hsp = -2.5;
				
				d->hp = 100;
				//d->hp = 1;
				
				d->state = 0;
				d->timer = 0;
				
				d->blink = 0;
				d->boblen = 32;
				d->bobspd = 3;
				
				d->tailangle = 90;
				
				d->rotcounter = 0;
				d->bobcounter = 0;
				
				d->bobspd = 3;
				d->rotspd = 1;
				
				d->imageIndex = 0;
				
				e->data = d;
				e->enemyStep = devilStep;
				e->enemyDraw = devilDraw;
				e->type = 45;
				
				enemies[i] = e;
				i = MAX_ENEMIES;
			}
		}
	}
}

void devilStep(Devil* d)
{	
	char dead = 0;

	//Animate
	{
		d->imageIndex += 0.1;
		if (d->imageIndex >= 2) {
			d->imageIndex -= 2;
		}
		
		if (d->blink > 0) {
			d->blink -= 1;
		}
	}
	
	//Bob
	{
		if (d->state != 4) {
			d->bobcounter += d->bobspd;
			if (d->bobcounter >= 360) {
				d->bobcounter -= 360;
			}
			
			d->y = d->ystart + (d->boblen * cos(d->bobcounter * 3.14159 / 180));
		}
	}
	
	//Swing tail
	{
		d->rotcounter += d->rotspd;
		if (d->rotcounter >= 360) {
			d->rotcounter -= 360;
		}
		
		d->tailangle = 90 + (55 * cos(d->rotcounter * 3.14159 / 180));
	}
	
	//Patterns
	{
		//movement
		if (d->state == 0 || d->state == 2)
		{		
			d->rotspd = 1;
			d->boblen = 32;
			d->bobspd = 3;
			
			//Re-align ystart
			if (d->ystart > d->newystart) {
				d->ystart -= 1;
			}
			if (d->ystart < d->newystart) {
				d->ystart += 1;
			}
			
			d->x += d->hsp;
			
			//Slow Down
			double rate = 0.016;
			if (d->hsp < 0) {
				d->hsp += rate;
				if (d->hsp >= 0) {
					d->hsp = 0;
				}
			}
			
			if (d->hsp > 0) {
				d->hsp -= rate;
				if (d->hsp <= 0) {
					d->hsp = 0;
				}
			}		
			
			if (d->hsp == 0) {
				d->timer = 0;
				if (d->state == 0) {
					d->state = 1;
				}
				
				if (d->state == 2) {
					if ((d->rotcounter >= 90 && d->rotcounter <= 90 + d->rotspd) || (d->rotcounter >= 270 && d->rotcounter <= 270 + d->rotspd)) {
						d->state = 3;
					}
				}
			}
		}
		//mid room pause
		else if (d->state == 1)
		{
			d->timer += 1;
			if (d->timer >= 60) {
				if (d->state == 1) {
					d->hsp = 2.5;
					if (herox < d->x) {
						d->hsp *= -1;
					}
				}
				d->state = 2;
			}
		}
		//Shoot
		else if (d->state == 3)
		{
			d->rotspd = 3;
			d->boblen = 10;
			d->bobspd = 10;
			
			d->timer += 1;
			
			//Shoot orbs
			if (d->timer == 120 || d->timer == 240 || d->timer == 360) {
				int aim = (atan2((heroy + 20) - d->y, d->x - herox) * 180 / 3.14159) + 270;
				
				int spawnY = d->y + 20;		
				createOrb(d->x, spawnY, aim + 22);
				createOrb(d->x, spawnY, aim + 11);
				createOrb(d->x, spawnY, aim);
				createOrb(d->x, spawnY, aim - 11);				
				createOrb(d->x, spawnY, aim - 22);		

				PHL_PlaySound(sounds[sndShot03], CHN_ENEMIES);
			}
			
			if (d->timer == 360) {
				d->state = 0;
				d->hsp = 2.5;
				
				if (d->x > 320) {
					d->hsp *= -1;
				}				
				
				int chaseY = heroy - d->ystart;
				if (chaseY > 52) { chaseY = 52; }
				if (chaseY < -52) { chaseY = -52; }
				
				d->newystart = d->ystart + chaseY;
			}
		}
		
		//Death
		if (d->state == 4) {
			d->rotspd = 3;
			d->y += 0.2;
			d->timer -= 1;
		
			if (d->timer % 12 == 0) {
				createEffect(2, d->x - 64 + (rand() % 100), d->y - 64 + (rand() % 80));
			}
			
			if (d->timer <= 0) {
				dead = 1;
			}
		}
	}
	
	//Collisions
	if (d->state != 4) {
		//Setup masks
		Mask masks[6];
		
		//Head mask
		masks[0].unused = masks[0].circle = 0;
		masks[0].w = 100;
		masks[0].h = 104;	
		masks[0].x = d->x - (masks[0].w / 2);
		masks[0].y = d->y - (masks[0].h / 2);
		
		//Link masks
		for (int i = 1; i < 5; i++) {
			int taildis[4] = {54, 80, 108, 134};
			int taillag[4] = {10, 15, 10, 5};
			
			double newtailangle = 90 + (55 * cos((d->rotcounter - taillag[i-1]) * 3.14159 / 180));
			
			masks[i].unused = 0;
			masks[i].circle = 1;
			masks[i].w = 16;
			masks[i].h = 16;
			masks[i].x = d->x + (taildis[i-1] * cos(newtailangle * 3.14159 / 180));
			masks[i].y = d->y + (taildis[i-1] * sin(newtailangle * 3.14159 / 180));
		}
		
		//Barb mask
		masks[5].unused = masks[5].circle = 0;
		masks[5].w = 40;
		masks[5].h = 40;
		masks[5].x = (d->x + (160 * cos(d->tailangle * 3.14159 / 180))) - (masks[5].w / 2);
		masks[5].y = (d->y + (160 * sin(d->tailangle * 3.14159 / 180))) - (masks[5].h / 2);
		
		//Collisions
		int hitHead = 0;
		for (int a = 0; a < 6; a++)
		{
			if (a == 0 || a == 5) {
				//Hit Player
				if (checkCollision(masks[a], getHeroMask())) {
					
					int damage = 25;
					if (a == 0) {
						damage = 50;
					}
					
					if (heroHit(damage, masks[a].x + (masks[a].w / 2)) == 1) {
						if (a == 5) { //Barb
							heroStun();
						}
					}
				}
			}
			
			//Weapon collision
			if (hitHead == 0) {
				for (int i = 0; i < MAX_WEAPONS; i++) {
					if (weapons[i] != NULL) {
						if (weapons[i]->cooldown == 0) {
							if (checkCollision(masks[a], weapons[i]->weaponMask)) {
								weaponHit(weapons[i]);
								
								//Head
								if (a == 0) {
									d->hp -= 1;
									d->blink = 15;
									hitHead = 1;
									
									if (d->hp <= 0) {
										d->state = 4;
										d->timer = 180;
										d->blink = 200;
									}
								}else{
									PHL_PlaySound(sounds[sndHit03], CHN_WEAPONS);
								}
								
								i = MAX_WEAPONS;
							}
						}
					}	
				}
			}
		}
	}
	
	//Destroy
	if (dead == 1) {
		enemyDestroy(d->id);
		bossDefeatedFlag = 1;
		roomSecret = 1;

		flags[boss6flag] = 1;
		PHL_StopMusic();
	}
}

void devilDraw(Devil* d)
{
	if (d->blink % 2 == 0)
	{
		int dx, dy;
		
		//Draw tail
		int taildis[4] = {54, 80, 108, 134};
		int taillag[4] = {10, 15, 10, 5};
		for (int i = 0; i < 4; i++) {
			double newtailangle = 90 + (55 * cos((d->rotcounter - taillag[i]) * 3.14159 / 180));

			dx = d->x + (taildis[i] * cos(newtailangle * 3.14159 / 180)) - 32;
			dy = d->y + (taildis[i] * sin(newtailangle * 3.14159 / 180)) - 32;
			PHL_DrawSurfacePart(dx, dy, 0, 128, 64, 64, images[imgBoss]);
		}
		
		//Draw Head
		dx = d->x - 64;
		dy = d->y - 64;
		PHL_DrawSurfacePart(dx, dy, (int)d->imageIndex * 128, 0, 128, 128, images[imgBoss]);
		
		//Draw Tail Tip
		dx = d->x + (160 * cos(d->tailangle * 3.14159 / 180)) - 32;
		dy = d->y + (160 * sin(d->tailangle * 3.14159 / 180)) - 32;
		PHL_DrawSurfacePart(dx, dy, 64, 128, 64, 64, images[imgBoss]);
		
	}
}


//Stone Orbs
void createOrb(int x, int y, double dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {				
			Enemy* e = /*(Enemy*)*/malloc(sizeof *e);
			Orb* o = /*(Orb*)*/malloc(sizeof *o);
			o->id = i;
			
			o->x = x;
			o->y = y;
			
			o->dir = dir;
			
			o->imageIndex = 0;
			
			e->data = o;
			e->enemyStep = orbStep;
			e->enemyDraw = orbDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void orbStep(Orb* o)
{
	char dead = 0;
	
	//Animate
	{
		o->imageIndex += 0.33;
		if (o->imageIndex >= 4) {
			o->imageIndex -= 4;
		}
	}
	
	//Movement
	{
		int spd = 4;
		o->x += spd * sin(o->dir * 3.14159 / 180);
		o->y += spd * cos(o->dir * 3.14159 / 180);
	}
	
	//Collision
	{
		Mask mask;
		mask.unused = 0;
		mask.circle = 1;
		mask.w = 6;
		mask.x = o->x;
		mask.y = o->y;
		
		//Collide with shield
		/*if (checkCollision(mask, shieldMask)) {
			createEffect(1, o->x - 20, o->y - 20);
			PHL_PlaySound(sounds[sndHit07], CHN_EFFECTS);
			dead = 1;
		}else{*/
			//Hit player
			if (checkCollision(getHeroMask(), mask)) {
				heroStone();
				heroHit(25, mask.x);
			}
		//}
		
		//Collide with weapon
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (checkCollision(mask, weapons[i]->weaponMask)) {
					weaponHit(weapons[i]);
					
					createEffect(2, o->x - 32, o->y - 32);
					dead = 1;
					
					i = MAX_WEAPONS;
				}
			}	
		}
	}
	
	//Destroy if outside of room
	{
		if (o->x < -20 || o->x > 660 || o->y < -20 || o->y > 500) {
			dead = 1;
		}
	}
	
	//Finally erase object
	{
		if (dead == 1) {
			enemyDestroy(o->id);
		}
	}
}

void orbDraw(Orb* o)
{
	int animation[4] = {0, 1, 0, 2};
	PHL_DrawSurfacePart(o->x - 20, o->y - 20, 440 + (animation[(int)o->imageIndex] * 40), 480, 40, 40, images[imgMisc20]);
}