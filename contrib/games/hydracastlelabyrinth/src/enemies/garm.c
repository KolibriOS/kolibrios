#include "garm.h"
#include "../game.h"
#include "../PHL.h"
#include "../hero.h"
#include <stdlib.h>

int boss7flag = 47;

void garmStep(Garm* g);
void garmDraw(Garm* g);

void garmrockStep(Garmrock* g);
void garmrockDraw(Garmrock* g);

void createGarm(int x, int y)
{	
	if (flags[boss7flag] == 0) { //have not beaten boss 7
		PHL_FreeSurface(images[imgBoss]);
		images[imgBoss] = PHL_LoadQDA("boss07.bmp");
	
		int i;
		for (i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i] == NULL) {
				setBossRoom();
				
				Enemy* e = malloc(sizeof *e);
				Garm* g = malloc(sizeof *g);
				
				g->id = i;
				
				g->hp = 105;
				//g->hp = 1;
				
				g->x = x;
				g->y = y;
				
				g->hsp = 0;
				g->vsp = 0;
				
				g->dir = -1;
				
				g->imageIndex = 0;
				
				g->state = 0;
				g->timer = 0;
				
				g->blink = 0;
				
				g->substate = 0;
				g->wallcounter = 0;
				g->targx = 0;
				
				e->data = g;
				e->enemyStep = garmStep;
				e->enemyDraw = garmDraw;
				e->type = 46;
				
				enemies[i] = e;
				i = MAX_ENEMIES;
			}
		}
	}
}

void garmStep(Garm* g)
{
	char dead = 0;
	
	//Blink animation
	{
		if (g->blink > 0) {
			g->blink -= 1;
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 88;
		mask.h = 104;
		mask.x = g->x - (mask.w / 2);
		mask.y = g->y + (120 - mask.h);
	}
	
	//Stand still
	if (g->state == 0)
	{
		//Animate
		{
			g->imageIndex += 0.0625;
			if (g->imageIndex >= 2) {
				g->imageIndex -= 2;
			}
		}
		
		//End state
		{
			g->timer += 1;
			if (g->timer >= 60) {
				g->state = 1;
				//g->vsp = -4.5;
				g->counter = 0;
				g->timer = 0;
				//PHL_PlaySound(sounds[sndPi09], CHN_ENEMIES);
			}
		}
	}
	
	//Bounce
	else if (g->state == 1)
	{
		//Animate
		{
			g->imageIndex += 0.33;
			if (g->imageIndex >= 3) {
				g->imageIndex -= 3;
			}
		}
		
		if (g->timer > 0) {
			g->vsp = 0;
			g->imageIndex = 0;
			g->timer -= 1;
			if (g->timer <= 0) {
				//End state
				if (g->counter >= 3) {
					g->state = 2;
					g->counter = 0;
					g->imageIndex = 0;
					g->vsp = -6;
					g->hsp = 8;
					if (g->x > herox) {
						g->hsp *= -1;
					}
					
					if (g->substate == 0) {
						g->wallcounter = 1;
						g->substate = 1;
					}else{
						g->wallcounter = 2;
						g->substate = 0;
					}
				}else{
					g->vsp = -5;					
				}
			}
		}
		
		else if (g->timer == 0) {
			double grav = 0.25;
			
			//Movement
			if (g->timer == 0) {
				g->y += g->vsp;
				g->vsp += grav;
				mask.y = g->y + (120 - mask.h);
			}
			
			//Land on ground
			if (g->vsp >= 0 && g->timer == 0) {
				PHL_Rect collide = getTileCollision(1, mask);
				if (collide.x != -1) {
					g->y = collide.y - 120;
					mask.y = g->y + (120 - mask.h);
					g->vsp = 0;
					g->timer = 3;
					g->counter += 1;
					PHL_PlaySound(sounds[sndPi09], CHN_ENEMIES);
				}
			}
		}
	}
	
	//Leap towards wall
	else if (g->state == 2)
	{		
		double grav = 0.25;
		
		//Set image
		{
			if (g->hsp > 0) {
				g->imageIndex = 0;
			}
			
			if (g->hsp < 0) {
				g->imageIndex = 1;
			}
		}
		
		//Movement
		{
			g->y += g->vsp;
			g->vsp += grav;
			mask.y = g->y + (120 - mask.h);

			g->x += g->hsp;
			mask.x = g->x - (mask.w / 2);
		}
		
		if (g->wallcounter > 0)
		{				
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x != -1) {
				g->wallcounter -= 1;				
				if (g->hsp < 0) {
					g->x = collide.x + 40 + (mask.w / 2);
				}
				if (g->hsp > 0) {
					g->x = collide.x - (mask.w / 2);
				}
				g->state = 3;
				g->timer = 0;
			}
		}
		
		//Ground pound
		else {
			char action = 0;
			
			if ( (g->hsp > 0 && g->x > g->targx) || (g->hsp < 0 && g->x < g->targx) ) {
				action = 1;
			}
			//Wall collision backup
			else{
				PHL_Rect collide = getTileCollision(1, mask);
				if (collide.x != -1) {
					if (g->hsp < 0) {
						g->x = collide.x + 40 + (mask.w / 2);
					}
					if (g->hsp > 0) {
						g->x = collide.x - (mask.w / 2);
					}
					action = 1;
				}
			}
			
			if (action == 1) {
				g->state = 4;
				g->vsp = -4;
				PHL_PlaySound(sounds[sndWolf01], CHN_ENEMIES);
			}
		}
	}
	
	//Grab wall
	else if (g->state == 3)
	{
		g->timer += 1;
		if (g->timer > 5) {
			g->state = 2;
			g->vsp = -6;
			g->hsp *= -1;			
			PHL_PlaySound(sounds[sndPi09], CHN_ENEMIES);
			
			g->targx = herox;
			
			if (g->wallcounter <= 0) {
				//Get distance from player
				int dis = g->x - g->targx;
				{				
					if (dis < 0) {
						dis *= -1;
					}
				}
				
				if (dis < 200 || g->substate == 1) {
					g->hsp /= 2;
				}
			}
		}
	}
	
	//Ground pound
	else if (g->state == 4)
	{
		double grav = 0.2;
		
		//Animate
		{
			g->imageIndex += 0.33;
			if (g->imageIndex >= 3) {
				g->imageIndex -= 3;
			}
		}
		
		g->y += g->vsp;
		g->vsp += grav;
		mask.y = g->y + (120 - mask.h);
		
		//Collide with floor
		{
			PHL_Rect collide = getTileCollision(1, mask);
			
			if (collide.x != -1) {
				g->y = collide.y - 120;
				PHL_PlaySound(sounds[sndHit04], CHN_ENEMIES);
				quakeTimer = 30;
				g->state = 0;
				g->timer = -20;
				//Create rocks
				createGarmrock(g->x + 64, g->y + 100, 2, -4);
				createGarmrock(g->x + 34, g->y + 100, 1, -5);
				createGarmrock(g->x - 34, g->y + 100, -1, -5);
				createGarmrock(g->x - 64, g->y + 100, -2, -4);
				
				createEffectExtra(3, g->x - 50, g->y + 90, -1, 0, 0);
				createEffectExtra(3, g->x + 10, g->y + 90, 1, 0, 0);
			}
		}
	}
	
	//Dead
	if (g->state == 5) {
		//Animate
		{
			g->imageIndex += 0.33;
			if (g->imageIndex >= 3) {
				g->imageIndex -= 3;
			}
		}
		
		g->y += 0.2;
		
		if (g->blink % 12 == 0) {
			createEffect(2, g->x - 64 + (rand() % 100), g->y + 60 - 64 + (rand() % 80));
		}

		if (g->blink <= 0) {
			dead = 1;
		}
	}
	
	else{	
		if (dead == 0) {
			//Update Mask
			{
				mask.x = g->x - (mask.w / 2);
				mask.y = g->y + (120 - mask.h);
			}
			
			//Hero collision
			{
				if (checkCollision(getHeroMask(), mask) == 1) {
					heroHit(40, mask.x + (mask.w / 2));
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
								g->hp -= 1;
								g->blink = 15;
								//Dead
								if (g->hp <= 0) {
									g->state = 5;
									g->blink = 200;
								}
								
								i = MAX_WEAPONS;
							}
						}
					}	
				}
			}
			
		}
	}
	
	if (dead == 1) {
		//Destroy
		{
			enemyDestroy(g->id);
			bossDefeatedFlag = 1;
			roomSecret = 1;

			flags[boss7flag] = 1;
			PHL_StopMusic();
		}
	}
	
}

void garmDraw(Garm* g)
{
	if (g->blink % 2 == 0) {
		int cropX = 0,
			cropY = 0;
		
		//Jump Spinning
		if ((g->state == 1 && g->timer == 0) || g->state == 4 || g->state == 5) {
			cropY = 128;
			cropX = 256;
		}
		
		//Jump
		if (g->state == 2) {
			cropY = 128;
		}
		
		//Wall grab
		if (g->state == 3) {
			cropX = 384;
		}
			
		cropX += (int)g->imageIndex * 128;
			
		PHL_DrawSurfacePart(g->x - 64, g->y - 8, cropX, cropY, 128, 128, images[imgBoss]);
	}
}

//Rocks
void createGarmrock(int x, int y, double hsp, double vsp)
{		
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {			
			Enemy* e = malloc(sizeof *e);
			Garmrock* g = malloc(sizeof *g);
			
			g->id = i;
			g->hp = 3;
			
			g->x = x;
			g->y = y;
			
			g->hsp = hsp;
			g->vsp = vsp;
			
			g->imageIndex = 0;
			
			g->counter = 0;
			g->inwall = 0;
			g->blink = 0;
			
			e->data = g;
			e->enemyStep = garmrockStep;
			e->enemyDraw = garmrockDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void garmrockStep(Garmrock* g)
{
	char dead = 0;
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 44;
		mask.h = 44;
		mask.x = g->x - (mask.w / 2);
		mask.y = g->y - (mask.h / 2);
	}
	
	//Animate
	{
		g->imageIndex += 0.2;
		if (g->imageIndex >= 4) {
			g->imageIndex -= 4;
		}
		
		if (g->blink > 0) {
			g->blink -= 1;
		}
	}
	
	//Horizontal movement
	{
		g->x += g->hsp;
		mask.x = g->x - (mask.w / 2);
		
		g->inwall = 0;
		if (checkTileCollision(1, mask) == 1) {
			g->inwall = 1;
		}
	}
	
	//Vertical movement
	{
		double grav = 0.1;
		
		g->y += g->vsp;
		g->vsp += grav;
		mask.y = g->y - (mask.h / 2);
		
		if (g->inwall == 0 && g->counter == 0) {
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x != -1) {
				g->counter = 1;
				g->y = collide.y - (mask.h / 2);
				g->vsp = -2;
				PHL_PlaySound(sounds[sndHit06], CHN_ENEMIES);
			}
		}
	}
	
	//Update mask
	{
		mask.x = g->x - (mask.w / 2);
		mask.y = g->y - (mask.h / 2);
	}
	
	//Hero collision
	{
		if (checkCollision(getHeroMask(), mask) == 1) {
			heroHit(30, mask.x + (mask.w / 2));
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
						g->hp -= 1;
						g->blink = 15;
						
						if (g->hp <= 0) {
							dead = 1;
							createRockSmash(g->x, g->y + 20);
						}
						
						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}

	//Destroy when out of room
	{
		if (mask.y > 480) {
			dead = 1;
		}
	}
	
	//Destroy object
	{
		if (dead == 1) {
			enemyDestroy(g->id);
		}
	}
}

void garmrockDraw(Garmrock* g)
{
	if (g->blink % 2 == 0) {
		int cropX = 256,
			cropY = 192;
			
		if (g->hsp < 0) {
			cropX = 512;
		}
		
		cropX += (int)g->imageIndex * 64;
		
		while (cropX >= 640) {
			cropX -= 640;
			cropY += 64;
		}
		
		PHL_DrawSurfacePart(g->x - 32, g->y - 32, cropX, cropY, 64, 64, images[imgMisc32]);
	}
}