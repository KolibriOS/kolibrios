#include "hydra.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>
#include <math.h>

const double PI = 3.14159;

double headRot = 0;

void hydraStep(Hydra* h);
void hydraDraw(Hydra* h);

void hydraDestroy(Hydra* h);

void hydraheadStep(Hydrahead* h);
void hydraheadDraw(Hydrahead* h);

void hydragoopStep(Hydragoop* h);
void hydragoopDraw(Hydragoop* h);

void hydrarockStep(Hydrarock* h);
void hydrarockDraw(Hydrarock* h);

void hydrashockStep(Hydrashock* h);
void hydrashockDraw(Hydrashock* h);

double getHydraX(Hydrahead* h);
double getHydraY(Hydrahead* h);

Mask getHydraMask(Hydra* h);
int checkWeaponCollision(Mask m);

double lengthdir_x(double ang, double len);
double lengthdir_y(double ang, double len);

void setHeadState(int headid, int state);


//#hydra
void createHydra(int x)
{
	PHL_FreeSurface(images[imgBoss]);
	images[imgBoss] = PHL_LoadQDA("lboss01.bmp");
	
	int i;
	for (i = 4; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			setBossRoom();
			
			Enemy* e = malloc(sizeof *e);
			Hydra* h = malloc(sizeof *h);
			h->id = i;
			
			h->hp = 10;
			//h->hp = 1;
			h->blink = 0;
			
			h->x = x;
			h->y = -64;
			
			h->hsp = 0;
			h->vsp = 0;
			
			h->imageIndex = 0;
			
			h->state = 0;
			h->timer = 0;
			
			h->patternCounter = 0;
			
			h->onground = 0;
			h->noheads = 0;
			
			e->data = h;
			e->enemyStep = hydraStep;
			e->enemyDraw = hydraDraw;
			e->type = 47;
			
			enemies[i] = e;
						
			h->headid[0] = createHydrahead(-1, 0, i);
			h->headid[1] = createHydrahead(1, 0, i);
			h->headid[2] = createHydrahead(1, 1, i);
			h->headid[3] = createHydrahead(-1, 1, i);
			
			i = MAX_ENEMIES;
		}
	}
}

void hydraStep(Hydra* h)
{	
	double grav = 0.2;
	double fric = 0.1;
	
	//Death
	if (h->state == 6) {
		h->y += 0.2;
		
		h->timer -= 1;
		h->blink -= 1;
		
		if (h->timer % 12 == 0) {
			createEffect(2, h->x - 64 + (rand() % 128) - 32, h->y - 64 + (rand() % 128));
		}
		
		if (h->timer <= 0) {
			hydraDestroy(h);
		}
	}
	else{	
		//Setup Mask
		Mask mask = getHydraMask(h);
		
		//States with hydra heads
		if (h->noheads == 0) {		
			//Fall in intro
			if (h->state == 0) {
				h->hsp = 0;
				h->timer += 1;
				if (h->timer >= 50) {
					h->timer = 50;			
					h->imageIndex = 2;
					
					if (h->onground == 1) {
						h->state = 1;
						h->timer = 0;
					}
				}else{
					grav = 0;
				}
			}
			
			//Wait/Pattern activate
			else if (h->state == 1)
			{			
				h->timer += 1;
				
				//Stop speed animation
				if (h->timer >= 120) {					
					int patternSize = 9;
					int pattern[9] = {4, 0, 4, 1, 4, 2, 4, 3, 2};
					
					//Head seizure
					if (pattern[h->patternCounter] == 4) {
						h->state = 4;
						h->timer = 0;
					}
					//Small hop
					if (pattern[h->patternCounter] == 0) {
						h->state = 2;
						h->timer = 0;					
					}
					
					//Goop
					if (pattern[h->patternCounter] == 1) {
						h->timer = -120;
						setHeadState(h->headid[0], 2);
						setHeadState(h->headid[1], 2);
					}
					
					//Big Hop
					if (pattern[h->patternCounter] == 2) {
						h->state = 3;
						h->timer = 0;
					}
					
					//Electricity
					if (pattern[h->patternCounter] == 3) {
						h->timer = -40;
						setHeadState(h->headid[2], 3);
						setHeadState(h->headid[3], 3);
					}
					
					h->patternCounter += 1;
					if (h->patternCounter >= patternSize) {
						h->patternCounter = 0;
					}
				}
			}
			
			//Head seizure state
			else if (h->state == 4) {			
				//Speed up head animation
				if (h->timer == 0) {
					setHeadState(h->headid[0], 1);
					setHeadState(h->headid[1], 1);
					setHeadState(h->headid[2], 1);
					setHeadState(h->headid[3], 1);
				}
				
				h->timer += 1;
				
				//Stop speed animation
				if (h->timer == 120) {				
					setHeadState(h->headid[0], 0);
					setHeadState(h->headid[1], 0);
					setHeadState(h->headid[2], 0);
					setHeadState(h->headid[3], 0);
					
					//Pattern
					h->state = 1;
					h->timer = 120;
				}
			}
			
			//Switch to noheads mode
			if (h->onground == 1 &&
				enemies[h->headid[0]] == NULL &&
				enemies[h->headid[1]] == NULL &&
				enemies[h->headid[2]] == NULL &&
				enemies[h->headid[3]] == NULL)
			{
				h->noheads = 1;
				h->state = 1;
				h->timer = -15;
				h->patternCounter = 0;
			}
		}
		
		//States without hydra heads
		else{				
			//Wait/pattern activate
			if (h->state == 1) {
				h->timer += 1;
				
				if (h->timer >= 0) {
					int patternSize = 3;
					int pattern[3] = {0, 0, 2};
					
					//Small hop
					if (pattern[h->patternCounter] == 0) {
						h->state = 2;
						h->timer = 0;					
					}
					
					//Big Hop
					if (pattern[h->patternCounter] == 2) {
						h->state = 3;
						h->timer = 0;
					}
					
					h->patternCounter += 1;
					if (h->patternCounter >= patternSize) {
						h->patternCounter = 0;
					}
				}
			}
		}
		
		//States used by both modes
		{
			//Small hop
			if (h->state == 2) {			
				//Setup
				if (h->timer == 0) {
					h->vsp = -2;
					h->onground = 0;
					h->hsp = 2.5;
					if (herox < h->x) {
						h->hsp *= -1;
					}
				}
				
				h->timer += 1;
				
				if (h->onground == 1) {
					if (h->noheads == 0 || h->hsp == 0) {
						h->state = 1;
						h->timer = 0;
					}
				}
			}
			
			//Large Hop
			else if (h->state == 3) {
				h->hsp = 0;
				
				//Setup
				if (h->timer == 0) {
					h->timer = 1;
					if (h->noheads == 0) {
						h->vsp = -8;
					}else{
						h->vsp = -5;
					}
					h->onground = 0;
				}
				
				if (h->onground == 1) {
					h->timer += 1;
					
					if (h->timer % 20 == 0) {
						createHydrarock();
					}
					
					if (h->timer >= 220) {
						h->state = 1;
						h->timer = -15;
					}
				}
			}
		}
			
		//Animate
		{
			if (h->onground == 1) {
				h->imageIndex += 0.1;
				if (h->imageIndex >= 2) {
					h->imageIndex -= 2;
				}
			}else{
				if (h->vsp < 0) {
					h->imageIndex = 3;
				}
				else {
					h->imageIndex = 2;
				}
			}
			
			//Blink
			if (h->blink > 0) {
				h->blink -= 1;
			}
		}
			
		//Movement
		{
			//Horizontal
			if (h->hsp != 0) {
				h->x += h->hsp;
				mask = getHydraMask(h);
				
				PHL_Rect collide = getTileCollision(1, mask);
				if (collide.x != -1) {
					int dir = 1;
					if (h->hsp < 0) {
						dir = -1;
					}
					h->x = collide.x + 20 - ((20 + (mask.w / 2)) * dir);
					
					h->hsp *= -1;
				}
			}
			
			//Friction
			{
				if (h->onground == 1) {
					if (h->hsp > 0) {
						h->hsp -= fric;
						if (h->hsp < 0) {
							h->hsp = 0;
						}
					}
					if (h->hsp < 0) {
						h->hsp += fric;
						if (h->hsp > 0) {
							h->hsp = 0;
						}
					}
				}
			}
			
			//Vertical
			{
				int maxVsp = 9;
				
				if (h->onground == 0) {
					h->y += h->vsp;
					h->vsp += grav;
					mask = getHydraMask(h);	
					
					//Limit vsp
					{
						if (h->vsp > maxVsp) {
							h->vsp = maxVsp;
						}
					}
					
					//Collide with floor
					{
						PHL_Rect collide = getTileCollision(1, mask);
						if (collide.x != -1) {
							h->y = collide.y - 64;
							h->vsp = 0;
							h->onground = 1;
							PHL_PlaySound(sounds[sndHit04], CHN_ENEMIES);
							quakeTimer = 30;
							createEffectExtra(3, h->x - 30, h->y + 32, -1, 0, 0);
							createEffectExtra(3, h->x - 10, h->y + 32, 1, 0, 0);
						}
					}
				}
			}
				
		}
		
		//Update mask
		mask = getHydraMask(h);
		
		//Hero Collision
		{
			if (checkCollision(mask, getHeroMask()) == 1) {
				heroHit(25, h->x);
			}
		}
			
		//Weapon Collision
		{
			int wid = checkWeaponCollision(mask);
			if (wid != -1) {
				//Pushed back
				if (h->noheads == 0) {
					h->hsp = weapons[wid]->dir;
					PHL_PlaySound(sounds[sndPi05], CHN_ENEMIES);
				}else{
					h->hp -= 1;
					h->blink = 15;
				}
				weaponHit(weapons[wid]);
				//Die
				if (h->hp <= 0) {
					h->state = 6;
					h->timer = 180;
					h->blink = 200;
				}
				
			}
		}
	}
	
}

void hydraDraw(Hydra* h)
{
	if (h->blink % 2 == 0) {	
		int cropX = (int)h->imageIndex * 128;
		int cropY = 128;
		
		if (h->noheads == 1) {
			cropY += 128;
		}
		
		PHL_DrawSurfacePart(h->x - 64, h->y - 64, cropX, cropY, 128, 128, images[imgBoss]);		
	}
}

void hydraDestroy(Hydra* h)
{
	enemyDestroy(h->id);
	bossDefeatedFlag = 1;
	roomSecret = 1;

	PHL_StopMusic();
}

Mask getHydraMask(Hydra* h)
{
	Mask mask;
	
	mask.unused = mask.circle = 0;
	mask.w = 84;
	mask.h = 84;
	mask.x = h->x - (mask.w / 2);
	mask.y = h->y - 64 + (128 - mask.h);
	
	return mask;
}

//#heads
int createHydrahead(int dir, int position, int bodyid)
{
	int result = -1;
	
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Hydrahead* h = malloc(sizeof *h);
			
			h->id = i;
			result = i;
			
			h->hp = 25;
			//h->hp = 1;
			h->blink = 0;
			
			h->dir = dir;
			h->position = position;
			
			h->imageIndex = 0;
			
			h->neckRot = 0;
			if (position != 0) {
				h->neckRot -= 45;
			}
			
			h->state = 0;
			h->timer = 0;
			h->counter = 0;
			
			h->bodyid = bodyid;
			
			int a;
			for (a = 0; a < 7; a++) {
				h->bodyposX[a] = 0;
				h->bodyposY[a] = 0;
			}
			
			e->data = h;
			e->enemyStep = hydraheadStep;
			e->enemyDraw = hydraheadDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
	
	return result;
}

void hydraheadStep(Hydrahead* h)
{
	char dead = 0;
	
	//Animate
	{
		h->imageIndex += 0.1;
		if (h->imageIndex >= 2) {
			h->imageIndex -= 2;
		}
		
		if (h->blink > 0) {
			h->blink -= 1;
		}
		
		h->neckRot += 2;
		if (h->neckRot >= 360) {
			h->neckRot -= 360;
		}
	}
	
	//States
	{
		//Death
		if (h->state == 4) {
			h->timer += 1;
			if (h->timer % 6 == 0) {
				createEffect(2, h->bodyposX[6 - h->counter] - 32, h->bodyposY[6 - h->counter] - 32);
				h->counter += 1;
			}
			
			if (h->counter >= 7) {
				dead = 1;
			}
		}
		else{
			if (h->state == 0) {
				//Do nothing special
			}
		
			//Fast movements
			else if (h->state == 1) {
				h->neckRot += 2;
			}
		
			//Shoot goop
			else if (h->state == 2) {
				h->neckRot += 2;
				h->timer += 1;
				
				//Create Goop
				if (h->timer % 15 == 0) {				
					int ghsp = -4 + (rand() % 9),
						gvsp = -6;
					
					createHydragoop(h->bodyposX[6], h->bodyposY[6], ghsp, gvsp);
				}
				
				if (h->timer >= 120) {
					h->state = 0;
				}
			}
		
			//Shoot electricity
			else if (h->state == 3) {
				if (h->timer == 0) {
					h->timer = 1;
				}
			
				if (h->timer % 20 == 0) {
					if (h->counter == 0) {
						createHydrashock(h->bodyposX[6] + (70 * h->dir), h->bodyposY[6] + 20);
					}
					h->neckRot -= 2;
					h->counter += 1;
					if (h->counter >= 20) {
						h->counter = 0;
						h->timer += 1;
					}
				}else{
					h->neckRot += 2;
					h->timer += 1;
				}		
				
				if (h->timer > 80) {
					h->state = 0;
					/*
					Hydra* body = enemies[h->bodyid]->data;
					body->state = 1;
					body->timer = 239;
					*/
				}
				
			}
			
			Mask mask;
			mask.circle = mask.unused = 0;
			
			//Collide with player
			{		
				int i;
				for (i = 0; i < 7; i+=2) {
					//Setup mask
					{
						mask.w = 48;
						mask.h = 48;
						
						//Head
						if (i == 6) {
							mask.w = 60;
							mask.h = 36;
						}
						
						mask.x = h->bodyposX[i] - (mask.w / 2);
						mask.y = h->bodyposY[i] - (mask.h / 2);
					}
					
					//Collide
					if (checkCollision(getHeroMask(), mask) == 1) {
						heroHit(25, getHydraX(h));
					}			
				}
			}
			
			//Weapon collision
			{
				//Mask should still be on the head
				int wid = checkWeaponCollision(mask);
				if (wid != -1) {
					h->blink = 15;
					h->hp -= 1;
					weaponHit(weapons[wid]);
					
					if (h->hp <= 0) {
						h->state = 4;
						h->timer = 0;
						h->counter = 0;
					}
				}
			}
			
			
		}
	
	}
	
	//Destroy object
	if (dead == 1) {
		enemyDestroy(h->id);
	}
}

void hydraheadDraw(Hydrahead* h)
{
	/*
	char c[10];
	sprintf(c, "%02d", h->timer);
	PHL_DrawTextBold(c, h->bodyposX[6], 0, 0);
	*/
	
	h->bodyposX[0] = getHydraX(h) + 20;
	h->bodyposY[0] = getHydraY(h);
		
	double drawX = getHydraX(h) + 20;
	double drawY = getHydraY(h);
	
	int dis = 24;
	int angle = -25;
		
	if (h->position == 1) {
		angle = -60;
			
		drawX -= 5;
		drawY -= 20;
	}
		
	int i;
	for (i = 0; i < 7; i++) {
		double wavlen = sin((h->neckRot + (45 * i)) * PI / 180);
		
		double incang = 45;
			
		if (h->position != 0) {
			incang = 45;
		}
			
		if (i == 6) {
			//incang += 15;
			incang = 50;
				
			if (h->position == 1) {
				incang = 80;
			}
		}
		
		drawX += lengthdir_x(angle + (incang * wavlen), dis);
		drawY += lengthdir_y(angle + (incang * wavlen), dis);
			
		h->bodyposX[i] = drawX;
		h->bodyposY[i] = drawY;
			
		if (h->dir == -1) {
			double difference = h->bodyposX[i] - getHydraX(h);
			h->bodyposX[i] = getHydraX(h) - difference;
		}
			
		if (h->blink % 2 == 0) {
			if (h->state != 4 || (6 - h->counter >= i)) {
				if (i != 6) {
					int cropX = 0;
					
					if (h->dir == -1) {
						cropX += 64;
					}
					
					PHL_DrawSurfacePart(h->bodyposX[i] - 32, h->bodyposY[i] - 32, cropX, 64, 64, 64, images[imgBoss]);
				}else{
					int cropX = 0;
					
					if (h->dir == -1) {
						cropX += 320;
					}
					
					cropX += (int)h->imageIndex * 80;
					
					PHL_DrawSurfacePart(h->bodyposX[i] - 40, h->bodyposY[i] - 32, cropX, 0, 80, 64, images[imgBoss]);
				}
			}
		}
	}
	
}

int checkWeaponCollision(Mask m)
{
	int i;
	for (i = 0; i < MAX_WEAPONS; i++) {
		if (weapons[i] != NULL) {
			if (weapons[i]->cooldown == 0) {
				if (checkCollision(weapons[i]->weaponMask, m) == 1) {
					return i;
				}
			}
		}
	}
	
	return -1;
}

double lengthdir_x(double ang, double len)
{
	return cos(ang * PI / 180) * len;
}

double lengthdir_y(double ang, double len)
{
	return sin(ang * PI / 180) * len;
}

double getHydraX(Hydrahead* h)
{
	if (enemies[h->bodyid] != NULL) {
		Hydra* hbody = enemies[h->bodyid]->data;
		return hbody->x;
	}
	
	return -1;
}

double getHydraY(Hydrahead* h)
{
	if (enemies[h->bodyid] != NULL) {
		Hydra* hbody = enemies[h->bodyid]->data;
		return hbody->y;
	}
	
	return -1;
}

void setHeadState(int headid, int state)
{
	if (enemies[headid] != NULL) {
		Hydrahead* h = enemies[headid]->data;
		if (h->state != 4) {
			h->state = state;
			h->timer = 0;
			h->counter = 0;
		}
	}
}

//#goop
void createHydragoop(int x, int y, int hsp, int vsp)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Hydragoop* h = malloc(sizeof *h);
			h->id = i;
			
			h->x = x;
			h->y = y;
			
			h->hsp = hsp;
			h->vsp = vsp;
			
			h->inwall = 0;
			h->bounce = 0;
			
			h->imageIndex = 0;
			
			e->data = h;
			e->enemyStep = hydragoopStep;
			e->enemyDraw = hydragoopDraw;
			e->type = -1;
			
			enemies[i] = e;			
			i = MAX_ENEMIES;
			
			PHL_PlaySound(sounds[sndPi06], CHN_ENEMIES);
		}
	}
}

void hydragoopStep(Hydragoop* h)
{
	char dead = 0;
	
	//Animate
	{
		h->imageIndex += 0.16;
		if (h->imageIndex >= 3) {
			h->imageIndex -= 3;
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 36;
		mask.h = 36;
		mask.x = h->x - mask.w / 2;
		mask.y = h->y - mask.h / 2;
	}
	
	//Movement
	{
		double grav = 0.2;
		
		h->x += h->hsp;
		mask.x = h->x - mask.w / 2;
		
		if (checkTileCollision(1, mask) == 1) {
			h->inwall = 1;
		}
		
		h->y += h->vsp;
		h->vsp += grav;
		mask.y = h->y - mask.h / 2;
		
		if (h->inwall == 0 && h->bounce == 0) {
			if (checkTileCollision(1, mask) == 1) {
				h->bounce = 1;
				h->vsp = -2;
			}
		}
	}
	
	//Outside of room
	{
		if ( (h->y > 500 && h->vsp >= 0) ||
			 (h->x < -20 && h->hsp <= 0) ||
			 (h->x > 660 && h->hsp >= 0) )
		{
			dead = 1; 
		}
	}
	
	//Collide with hero
	{
		//Collide with shield
		if (checkCollision(mask, shieldMask) == 1) {
			createEffect(1, h->x - 20, h->y - 20);
			PHL_PlaySound(sounds[sndHit07], CHN_EFFECTS);
			dead = 1;
		}
		else if (checkCollision(mask, getHeroMask()) == 1) {
			if (heroHit(25, h->x) == 1) {
				heroPoison();
			}
		}
	}
	
	//Destroy object
	{
		if (dead == 1) {
			enemyDestroy(h->id);
		}
	}
	
}

void hydragoopDraw(Hydragoop* h)
{
	int cropX = 320;
	
	cropX += (int)h->imageIndex * 40;
	
	PHL_DrawSurfacePart(h->x - 20, h->y - 20, cropX, 480, 40, 40, images[imgMisc20]);
}

//#rock
void createHydrarock()
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Hydrarock* h = malloc(sizeof *h);
			h->id = i;
			
			h->x = 70 + (rand() % 26) * 20;
			h->y = -24;
			
			h->vsp = 0;

			h->bounce = 0;
			
			h->imageIndex = 0;
			
			e->data = h;
			e->enemyStep = hydrarockStep;
			e->enemyDraw = hydrarockDraw;
			e->type = -1;
			
			enemies[i] = e;			
			i = MAX_ENEMIES;
		}
	}
}

void hydrarockStep(Hydrarock* h)
{
	//Animate
	{
		h->imageIndex += 0.25;
		if (h->imageIndex >= 8) {
			h->imageIndex -= 8;
		}
	}
	
	//Movement
	{
		double grav = 0.15;
		
		h->y += h->vsp;
		h->vsp += grav;
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 44;
		mask.h = 44;
		mask.x = h->x - mask.w / 2;
		mask.y = h->y - mask.h / 2;
	}
	
	if (h->bounce == 0) {
		if (checkTileCollision(1, mask) == 1) {
			h->bounce = 1;
			h->vsp = -2;
			PHL_PlaySound(sounds[sndHit06], CHN_ENEMIES);
		}
	}
	
	//Hero collision
	{
		if (checkCollision(mask, getHeroMask()) == 1) {
			heroHit(30, h->x);
		}
	}
	
	//Weapon Collision
	{
		int wid = checkWeaponCollision(mask);
		if (wid != -1) {
			weaponHit(weapons[wid]);
			PHL_PlaySound(sounds[sndHit03], CHN_WEAPONS);
		}
	}
	
	if (h->y >= 520) {
		enemyDestroy(h->id);
	}
}

void hydrarockDraw(Hydrarock* h)
{
	int cropX = 128;
	
	cropX += (int)h->imageIndex * 64;
	
	PHL_DrawSurfacePart(h->x - 32, h->y - 32, cropX, 128, 64, 64, images[imgMisc32]);
}

//#electricity
void createHydrashock(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Hydrashock* h = malloc(sizeof *h);
			h->id = i;
			
			h->timer = 0;
			
			h->x = x;
			h->y = y;
			
			h->angle = 0;
			
			h->imageIndex = 0;
			
			e->data = h;
			e->enemyStep = hydrashockStep;
			e->enemyDraw = hydrashockDraw;
			e->type = -1;
			
			enemies[i] = e;			
			i = MAX_ENEMIES;
			
			PHL_PlaySound(sounds[sndShot03], CHN_ENEMIES);
		}
	}
}

void hydrashockStep(Hydrashock* h)
{
	//Animate
	{
		h->imageIndex += 0.5;
		if (h->imageIndex >= 4) {
			h->imageIndex -= 4;
		}
	}
	
	h->timer += 1;
	
	if (h->timer >= 20) {
		if (h->timer == 20) {			
			//Set angle
			h->angle = (atan2(h->x - (herox), heroy + 20 - h->y) * 180 / PI) + 90;
		}
		
		h->timer = 22;
		
		//Movement
		{
			int spd = 5;
			h->x += lengthdir_x(h->angle, spd);
			h->y += lengthdir_y(h->angle, spd);
		}
	}
	
	//Setup mask
	Mask mask;
	{
		mask.unused = mask.circle = 0;
		mask.w = 28;
		mask.h = 28;
		mask.x = h->x - mask.w / 2;
		mask.y = h->y - mask.h / 2;
	}
	
	//Hero Collision
	{
		if (checkCollision(mask, getHeroMask()) == 1) {
			if (heroHit(25, h->x) == 1) {
				heroStun();
			}
		}
	}
	
	//Destroy if outside of room
	{
		if (mask.x > 660 || mask.x + mask.w < -20 || mask.y > 500 || mask.y + mask.h < -20) {
			enemyDestroy(h->id);
		}
	}
}

void hydrashockDraw(Hydrashock* h)
{
	if (h->timer % 2 == 0) {
		int cropX = (int)h->imageIndex * 64;
		
		PHL_DrawSurfacePart(h->x - 32, h->y - 32, cropX, 192, 64, 64, images[imgMisc32]);
	}
}