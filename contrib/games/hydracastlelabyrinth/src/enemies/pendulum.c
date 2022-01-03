#include "pendulum.h"
#include "../enemy.h"
#include "../game.h"
#include "../hero.h"
#include <stdlib.h>
#include <math.h>

void createPendulum(int x, int y, int side)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Pendulum* p = malloc(sizeof *p);
			p->id = i;
			
			p->x = x;
			p->y = y;
			
			p->angle = 0;
			p->rotCounter = 180;
			if (side == 1) {
				p->rotCounter += 180;
			}
			
			p->mask.circle = 1;
			p->mask.unused = 0;
			p->mask.w = 24;
			p->mask.x = 0;
			p->mask.y = 0;
			
			e->data = p;
			e->enemyStep = pendulumStep;
			e->enemyDraw = pendulumDraw;
			e->type = 22;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void pendulumStep(Pendulum* p)
{
	p->rotCounter += 2;
	if (p->rotCounter >= 360) {
		p->rotCounter -= 360;
	}
	
	p->angle += (3.15 * cos(p->rotCounter * 3.14159 / 180));
	
	//Update Mask	
	p->mask.x = p->x + (96 * cos((p->angle + 90) * 3.14159 / 180));
	p->mask.y = p->y + (96 * sin((p->angle + 90) * 3.14159 / 180));
	
	//Hit Player
	if (checkCollision(p->mask, getHeroMask())) {
		heroHit(15, p->mask.x);
	}
}

void pendulumDraw(Pendulum* p)
{
	int drawX = p->x,
		drawY = p->y;
		
	int len[] = {0, 16, 32, 48, 66, 96};
	int cropX[] = {64, 64, 64, 64, 0, 576};
	int cropY[] = {128, 128, 128, 128, 128, 64};
	
	int i;
	for (i = 0; i < 6; i++) {
		drawX = p->x + (len[i] * cos((p->angle + 90) * 3.14159 / 180));
		drawY = p->y + (len[i] * sin((p->angle + 90) * 3.14159 / 180));
		
		PHL_DrawSurfacePart(drawX- 32, drawY - 32, cropX[i], cropY[i], 64, 64, images[imgMisc32]);
	}
}