#include "effect.h"
#include "game.h"
#include "PHL.h"
#include "hero.h"
#include "collision.h"
#include <stdlib.h>
#include <math.h>

void createEffect(int type, int x, int y)
{
	createEffectExtra(type, x, y, 0, 0, 0);
}

void createEffectExtra(int t, int x, int y, double hsp, double vsp, int val)
{
	int i;
	for (i = 0; i < MAX_EFFECTS; i++) {
		if (effects[i] == NULL) {
			Effect* e = malloc(sizeof *e);
	
			e->id = i;
			e->type = t;
			
			e->x = x;
			e->y = y;
			
			e->vsp = vsp;
			e->hsp = hsp;
			e->grav = 0;
			
			e->imageIndex = 0;
			e->imageSpeed = 0;
			
			e->cropx = 0;
			e->cropy = 0;
			
			e->width = 40;
			e->height = 40;
			
			e->image = imgMisc20;
			e->timer = 60;
			
			
			e->visible = 1;
			e->val1 = 0;
			
			e->loop = 0;
			e->frames = 0;
			
			e->depth = 1;
			
			//Sword collision	
			if (e->type == 1) {			
				e->cropx = 440;
				e->cropy = 40;
				e->imageSpeed = 0.25;
				e->timer = 19;
			}
			
			//Enemy poof
			else if (e->type == 2) {
				PHL_PlaySound(sounds[sndBom01], CHN_EFFECTS);
				e->width = 64;
				e->height = 64;
				e->imageSpeed = 0.33;
				e->timer = 30;
				e->image = imgMisc32;
			}
			
			//Dust after landing from a fall - left/right
			else if (e->type == 3) {
				e->cropx = 320;
				e->cropy = 80;
				e->hsp = -1;
				if (hsp > 0) {
					e->hsp = 1;
					e->cropx = 0;
				}
				e->imageSpeed = 0.33;
				e->timer = 8 * (1 / e->imageSpeed);
			}
			
			//Block destroy/debris
			else if (e->type == 4) {
				e->grav = 0.2;
				e->loop = 1;
				e->frames = 4;
				e->timer = 60;
				
				//Set flash offset
				if ((e->hsp > 0 && val == 0) || (e->hsp > 0 && val == 1)) {
					e->timer -= 1;
				}
				
				e->imageSpeed = 0.34;
				
				int size = (rand() % 2) + 1;

				e->cropx = 0;
				if (e->hsp < 0) {
					e->cropx = 160;
				}
				if (size == 1) { //Big
					e->cropy = 40;
				}else{ //Small
					e->cropy = 440;
				}
				
			}
			
			//Chest sparkle
			else if (e->type == 5) {
				/*e->x -= 20;
				e->y -= 20;
				e->x += -20 + (rand() % 40) + 1;
				e->y += -20 + (rand() % 40) + 1;
				*/
				e->x -= 20;
				e->y -= 20;
				e->cropx = 440;
				e->cropy = 120;
				e->imageSpeed = 0.3;
				e->timer = 16;
				e->depth = 0;
			}
			
			//Charge orbs
			else if (e->type == 6) {
				e->x -= 20;
				e->y -= 20;
				e->cropx = 0;
				e->cropy = 200;
				
				e->val1 = (rand() % 360) + 1;
				e->imageSpeed = 0.3;
				e->timer = 20;
				e->depth = 0;
			}
			
			//Poison bubble
			else if (e->type == 7) {
				PHL_PlaySound(sounds[sndPi02], CHN_EFFECTS);
				e->x -= 30;
				e->x += (rand() % 20) + 1;
				e->cropx = 280;
				e->cropy = 120;
				e->timer = 35;
				
				e->vsp = -2;
				e->imageIndex = 0;
				e->imageSpeed = 0.16;
				e->depth = 0;
			}
			
			//Stone break free
			else if (e->type == 8) {
				e->image = imgMisc32;
				e->cropy = 64;
				e->width = 64;
				e->height = 64;
				
				e->imageSpeed = 0.32;
				e->timer = 18;
			}
			
			//Tiny stone debris
			else if (e->type == 9) {
				e->x -= 20;
				e->y -= 20;
				e->image = imgMisc20;
				e->cropy = 40;
				e->cropx = 320 + ((rand() % 3) * 40);
				e->imageSpeed = 0;
				
				e->vsp = -2 - (0.25 * (rand() % 8));
				e->hsp = -1 + (0.25 * (rand() % 8));
				e->grav = 0.1;
				
				e->timer = 60;
				e->depth = 0;
			}
			
			//Lava top animation
			else if (e->type == 10) {
				e->cropy = 40;
				e->cropx = 80;
				
				e->imageSpeed = 0.125;
				e->depth = -1;
				e->loop = 1;
				e->frames = 3;
				e->timer = 100;
				
				e->image = imgTiles;
			}
			
			//Water top animation
			else if (e->type == 11) {
				e->cropy = 40;
				e->cropx = 240;
				
				e->imageSpeed = 0.125;
				e->depth = -1;
				e->loop = 1;
				e->frames = 4;
				e->timer = 100;
				
				e->image = imgTiles;
			}
			
			//Hero Air Bubble
			else if (e->type == 12) {
				e->x -= 20;
				e->val1 = e->x; //Start x
				e->y -= 20;
				e->cropx = 440;
				e->loop = 1;
				e->frames = 2;
				e->imageSpeed = 0.2;
				e->timer = 120;
				e->vsp = -0.5;
				e->depth = 0;
			}
			
			//Water splash
			else if (e->type == 13) {
				e->cropx = 200;
				e->imageSpeed = 0.1;
				e->timer = 55;
				e->grav = 0.1;
			}
			
			//Lava splash
			else if (e->type == 14) {
				e->cropx = 400;
				e->cropy = 200;
				e->imageSpeed = 0.1;
				e->timer = 55;
				e->grav = 0.1;
			}
			
			effects[i] = e;
			i = MAX_EFFECTS;
		}
	}
}

void effectStep(Effect* e)
{
	e->x += e->hsp;
	e->y += e->vsp;
	e->vsp += e->grav;
	e->imageIndex += e->imageSpeed;
	
	if (e->loop == 1) {
		if (e->imageIndex >= e->frames) {
			e->imageIndex -= e->frames;
		}
	}
	
	if (e->type == 12) { //Hero Air Bubble
		e->x = e->val1 + 5 * sin((e->timer * 5) * 3.14159 / 180);
		if (checkTileCollisionXY(4, e->x + 20, e->y + 20) == 0) {
			e->timer = 0;
		}
	}
	
	if (e->type == 10 || e->type == 11) { //Lava top
		e->timer = 100;
	}
	
	if (e->type == 4 || e->type == 9 || e->type == 12) { //Stone Rubble
		if (e->timer <= 30 && e->timer % 2 != 0) {
			e->visible = 0;
		}else{
			e->visible = 1;
		}
	}
	else if (e->type == 6) { //Charge orb
		if (e->timer % 2 == 0) {
			e->visible = 1;
			e->x = herox + ((e->timer * 3) * sin(e->val1 * 3.14159 / 180)) - 20;
			e->y = heroy + ((e->timer * 3) * cos(e->val1 * 3.14159 / 180));
		}else{
			e->visible = 0;
		}
	}
	
	e->timer -= 1;
	if (e->timer <= 0) {
		effectDestroy(e->id);
	}
}

void effectDraw(Effect* e)
{	
	//if (e->type != 4 || (e->timer > 30 || e->timer % 2 == 0)) {
	if (e->visible == 1) {
		if (e->type == 7) { //Poison Bubble
			int animation[6] = {0, 1, 2, 1, 0, 3};
			PHL_DrawSurfacePart(e->x, e->y, e->cropx + (e->width * (animation[(int)e->imageIndex])), e->cropy, e->width, e->height, images[e->image]);
		}else{
			PHL_DrawSurfacePart(e->x, e->y, e->cropx + (e->width * ((int)e->imageIndex)), e->cropy, e->width, e->height, images[e->image]);
		}
	}	
}

void effectDestroy(int id)
{	
	if (effects[id] != NULL) {
		free(effects[id]);
	}
	effects[id] = NULL;
}

void createRockSmash(int x, int y)
{
	x -= 20;

	int randvsp = (rand() % 3) + 1;
	createEffectExtra(4, x, y, -1.5, -2 - randvsp, 0);
	
	randvsp = (rand() % 3) + 1;
	createEffectExtra(4, x, y, -1, -5 - randvsp, 1);
	
	randvsp = (rand() % 3) + 1;
	createEffectExtra(4, x, y, 1.5, -2 - randvsp, 0);
	
	randvsp = (rand() % 3) + 1;
	createEffectExtra(4, x, y, 1, -5 - randvsp, 1);
	PHL_PlaySound(sounds[sndBom02], 2);
}

void createSplash(int x, int y)
{
	double chsp = 0,
		   cvsp = 0;
	
	x -= 20;
	
	chsp = -0.25 - ((rand() % 9) * 0.25);
	cvsp = -1.5 - ((rand() % 9) * 0.25);
	createEffectExtra(13, x, y, chsp, cvsp, 0);
	
	chsp = 0.25 + ((rand() % 9) * 0.25);
	cvsp = -1.5 - ((rand() % 9) * 0.25);
	createEffectExtra(13, x, y, chsp, cvsp, 0);
	
	chsp = -0.25 - ((rand() % 9) * 0.25);
	cvsp = -0.5 - ((rand() % 4) * 0.25);
	createEffectExtra(13, x, y, chsp, cvsp, 0);
	
	chsp = 0.25 + ((rand() % 9) * 0.25);
	cvsp = -0.5 - ((rand() % 4) * 0.25);
	createEffectExtra(13, x, y, chsp, cvsp, 0);
	
	PHL_PlaySound(sounds[sndWater01], CHN_EFFECTS);
}

void createLavaSplash(int x, int y)
{
	double chsp = 0,
		   cvsp = 0;
	
	x -= 20;
	
	chsp = -0.25 - ((rand() % 9) * 0.25);
	cvsp = -1.5 - ((rand() % 9) * 0.25);
	createEffectExtra(14, x, y, chsp, cvsp, 0);
	
	chsp = 0.25 + ((rand() % 9) * 0.25);
	cvsp = -1.5 - ((rand() % 9) * 0.25);
	createEffectExtra(14, x, y, chsp, cvsp, 0);
	
	chsp = -0.25 - ((rand() % 9) * 0.25);
	cvsp = -0.5 - ((rand() % 4) * 0.25);
	createEffectExtra(14, x, y, chsp, cvsp, 0);
	
	chsp = 0.25 + ((rand() % 9) * 0.25);
	cvsp = -0.5 - ((rand() % 4) * 0.25);
	createEffectExtra(14, x, y, chsp, cvsp, 0);
	
	PHL_PlaySound(sounds[sndShot07], CHN_EFFECTS);
}