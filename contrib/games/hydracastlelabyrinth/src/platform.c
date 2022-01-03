#include "platform.h"
#include "game.h"
#include "PHL.h"
#include "hero.h"
#include <stdlib.h>

void createPlatform(int type, int xstart, int ystart, int xend, int yend, int spd, int secret)
{
	int i;
	for (i = 0; i < MAX_PLATFORMS; i++) {
		if (platforms[i] == NULL) {
			Platform* p = malloc(sizeof *p);
			p->id = i;
			p->type = type;
			
			p->x = p->xstart = xstart;
			p->y = p->ystart = ystart;
			
			p->xend = xend;
			p->yend = yend;
			
			p->timer = 0;
			
			p->secret = secret;
			
			if (roomSecret == 1) {
				p->secret = 0;
			}
			
			p->visible = 1;
			if (p->secret == 1) {
				p->visible = 0;
			}
			
			if (type == 1) {
				p->xend = p->x + 1;
				p->yend = p->y;				
			}
			
			p->state = 0;
			p->spd = spd;
			
			p->mask.circle = 0;
			p->mask.unused = 0;
			if (p->secret == 1) {
				p->mask.unused = 1;
			}
			p->mask.x = xstart;
			p->mask.y = ystart;
			p->mask.w = p->mask.h = 40;
			
			platforms[i] = p;
			i = MAX_PLATFORMS;
		}
	}
}

void platformStep(Platform* p)
{
	char isSolid = p->visible;
	
	//Megaman blocks
	if (p->type == 2) {
		int myStep = p->xend,
			maxSteps = p->yend;
			
		p->timer += 1;
		if (p->timer >= maxSteps * 60 + 60) {
			p->timer = 0;
		}
		
		//Play sound
		if (p->timer == myStep * 60) {
			PHL_PlaySound(sounds[sndPi03], CHN_SOUND);
		}
		
		if (p->timer > myStep * 60 && p->timer <= (myStep * 60) + 60) {
			isSolid = 1;
			p->visible = 1;
			
			//Blink effect
			if (p->timer > myStep * 60 + 30) {
				p->visible = p->timer % 2;
			}
		}else{
			isSolid = 0;
			p->mask.unused = 1;
			p->visible = 0;
			
			//Fall off platform
			if (getHeroMask().x > p->mask.x + p->mask.w || getHeroMask().x + getHeroMask().w < p->mask.x) {
			}else if (heroy == p->y - 40 && getHeroVsp() >= 0) {
				setHeroOnground(0);
			}
		}		
	}
	
	if (isSolid == 1) {
		p->mask.unused = 0;
		
		int targx = p->xend,
			targy = p->yend;
			
		if (p->state == 1) {
			targx = p->xstart;
			targy = p->ystart;
		}
		
		//Check if the player is standing on top
		int isontop = 0;
		int isabove = 0;
		if (getHeroMask().x > p->mask.x + p->mask.w || getHeroMask().x + getHeroMask().w < p->mask.x) {
		}else if (heroy == p->y - 40 && getHeroVsp() >= 0) {
			isontop = 1;
		}else if (heroy < p->y - 40) {
			isabove = 1;
		}
		
		//Move platform
		if (p->y != targy) {
			if (p->y < targy) {
				p->y += p->spd;
				if (isontop == 1) {
					heroy += p->spd;
				}
			}else{
				p->y -= p->spd;
				if (isontop == 1) {
					heroy -= p->spd;
				}else{
					p->mask.y = p->y;
					if (checkCollision(p->mask, getHeroMask()) && isabove == 1) {
						heroy = p->y - 40;
					}
				}
			}
		}
			
		if (p->x != targx) {
			if (p->x < targx) {
				p->x += p->spd;
				if (isontop == 1) {
					herox += p->spd;
				}
			}else{
				p->x -= p->spd;
				if (isontop == 1) {
					herox -= p->spd;
				}
			}
		}
			
		if (p->x == targx && p->y == targy) {
			if (p->state == 0) {
				p->state = 1;
			}else{
				p->state = 0;
			}
		}
		
		if (p->type == 0) //Moving platform
		{		
			
		}
		else if (p->type == 1) { //Loose block
			if (p->spd != 0) {
				p->timer -= 1;
				if (p->timer <= 0) {
					createRockSmash(p->x + 20, p->y + 20);
					if (isontop == 1) {
						setHeroOnground(0);
					}
					platformDestroy(p->id);					
					/*createEffectExtra(4, p->x, p->y, -1, 0, 0);
					createEffectExtra(4, p->x, p->y, -1, 0, 1);
					createEffectExtra(4, p->x, p->y, 1, 0, 0);
					createEffectExtra(4, p->x, p->y, 1, 0, 1);
					*/
				}
			}
		
			if (p->spd == 0 && isontop == 1) {
				p->spd = 2;
				p->timer = 30;
			}
		}
		
		//Update Mask
		p->mask.x = p->x;
		p->mask.y = p->y;
	}else{
		//p->mask.unused = 1;
		if (p->secret == 1) {
			if (roomSecret == 1) {
				p->mask.unused = 0;
				p->visible = 1;
				p->secret = 0;
				playSecret();
			}
		}
	}
}

void platformDraw(Platform* p)
{	
	if (p->visible == 1) {
		int cropX = p->type * 40;
		
		if (p->type == 3) {
			cropX = 9 * 40;
		}
		
		PHL_DrawSurfacePart(p->x, p->y, cropX, 400, 40, 40, images[imgMisc20]);
	}
}

void platformDestroy(int id)
{
	if (platforms[id] != NULL) {
		free(platforms[id]);
	}
	platforms[id] = NULL;
}