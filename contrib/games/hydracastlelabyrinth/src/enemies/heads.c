#include "heads.h"
#include "../enemy.h"
#include "../game.h"
#include "../PHL.h"
#include "../hero.h"
#include <stdlib.h>
#include <math.h>

void headStep(Head* h);
void headDraw(Head* h);

void bulletStep(Bullet* b);
void bulletDraw(Bullet* b);

void fireballStep(Fireball* f);
void fireballDraw(Fireball* f);

void laserStep(Laser* l);
void laserDraw(Laser* l);

void flameStep(Flame* f);
void flameDraw(Flame* f);

void rockStep(Rock* r);
void rockDraw(Rock* r);

void airStep(Air* a);
void airDraw(Air* a);

void createHead(int type, int x, int y, int dir, int offset, int cooloff)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Head* h = malloc(sizeof *h);
			
			h->id = i;
			h->type = type;		
			
			h->x = x;
			h->y = y;
			
			h->state = 0;
						
			h->hp = 5;
			h->invincible = 0;			
			h->counter = 0;
			
			h->dir = 1;
			if (dir == 1) {
				h->dir = -1;
			}

			h->timer = 30 * offset;			
			h->cooloff = 60;
			if (cooloff != 0) {
				h->cooloff = 30 * cooloff;
			}
			
			e->type = -1;
			if (h->type == 0) {
				e->type = 4;
				h->cooloff = 120;
			}
			else if (h->type == 1) {
				e->type = 6;
			}
			else if (h->type == 2) {
				e->type = 5;
			}
			else if (h->type == 3) {
				e->type = 7;
				h->cooloff = 120;
			}
			else if (h->type == 4) {
				e->type = 10;
				h->dir = 0;
			}
			else if (h->type == 5) {
				e->type = 25;
				h->dir = 0;
			}
			e->data = h;
			e->enemyStep = headStep;
			e->enemyDraw = headDraw;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void headStep(Head* h)
{
	int RHYNO = 0,
		MEDUSA = 1,
		DRAGON = 2,
		DEMON = 3,
		FIRE = 4,
		JAR = 5;
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.x = h->x;
		mask.y = h->y + 1;
		mask.w = 40;
		mask.h = 39;
	}

	//Timers
	{
		if (h->invincible > 0) {
			h->invincible -= 1;
		}
		
		if (h->timer > 0) {
			h->timer -= 1;
		}
	}
	
	//Wait
	if (h->state == 0)
	{
		char endstate = 0;
		
		if (h->timer <= 0) {
			//Proximity
			if (h->type == RHYNO || h->type == DEMON) {
				Mask area;
				area.circle = area.unused = 0;
				area.h = 80;
				area.w = 400;
				area.y = h->y - 20;
				area.x = h->x;
				if (h->dir == -1) {
					area.x -= area.w - 40;
				}
				
				if (checkCollision(area, getHeroMask()) == 1) {
					endstate = 1;
				}
			}else{
				endstate = 1;
			}
		}
		
		//Move onto next state
		if (endstate == 1) {
			h->state = 1;
			h->timer = 30;
		}
	}
	
	//Blink
	else if (h->state == 1)
	{
		//Shoot projectile
		if (h->timer <= 0) {
			//Play Sound
			{
				int soundtoplay[6] = {sndShot03, sndShot04, sndFire01, sndHit06, sndShot03, sndShot06};
				PHL_PlaySound(sounds[soundtoplay[h->type]], CHN_ENEMIES);
			}
			
			//Set vars
			{
				h->state = 0;
				h->timer = h->cooloff;
			}
			
			//Create projectile
			{
				//Rhyno head
				if (h->type == RHYNO) {
					createBullet(mask.x + (mask.w / 2), h->y + 24, h->dir, h->id);
				}
				//Medusa head
				if (h->type == MEDUSA) {
					createLaser(h->x, h->y, h->dir);
				}
				//Dragon head
				if (h->type == DRAGON) {
					createFlame(h->x + 20 + (20 * h->dir), h->y - 10, h->dir);
				}
				//Demon head
				if (h->type == DEMON) {
					createRock(h->x + (20 * h->dir), h->y, h->dir);
				}
				//Fireball Statue
				if (h->type == FIRE) {
					createFireball(h->x + 20, h->y + 20, (atan2(heroy - h->y, h->x - (herox - 20)) * 180 / 3.14159) + 270, h->id);
				}
				//Air Jar
				if (h->type == JAR) {
					h->state = 3;
					h->timer = 12;
					h->counter = 0;
				}
			}
			
		}
	}
	
	//Air Jar
	else if (h->state == 3)
	{		
		if (h->timer <= 0) {
			h->counter += 1;
			h->timer = 12;
			createAir(h->x, h->y - 20);
		}
		
		if (h->counter >= 6) {
			h->counter = 0;
			h->state = 0;
			h->timer = h->cooloff;
		}
	}
	
	//Hit player
	if (h->type != JAR) {
		if (checkCollision(getHeroMask(), mask)) {
			heroHit(10, mask.x + (mask.w / 2));
		}
	}
	
	//Weapon collision
	int i;
	for (i = 0; i < MAX_WEAPONS; i++) {
		if (weapons[i] != NULL) {
			if (weapons[i]->cooldown == 0) {
				if (checkCollision(mask, weapons[i]->weaponMask)) {
					h->hp -= 1;
					h->invincible = 15;
					weaponHit(weapons[i]);
					//Death
					if (h->hp <= 0) {
						createRockSmash(h->x + 20, h->y + 20);
						spawnCollectable(h->x + 20, h->y);
						enemyDestroy(h->id);
					}

					i = MAX_WEAPONS;
				}
			}
		}	
	}
}

void headDraw(Head* h)
{
	if (h->invincible % 2 == 0)
	{		
		int sheetX[6] = {0, 320, 160, 240, 560, 400};
		int sheetY[6] = {80, 80, 80, 120, 0, 120};
		
		int cropX = sheetX[h->type];
		
		int addx[6] = {6, 2, 0, 0, 0, 0};
		int frames = 2;
		
		//Change dir
		if (h->dir == 0) {
			frames = 1;
		}else{
			frames = 2;
			if (h->dir == -1) {
				cropX += 40;
			}
		}
		
		//White flash
		if (h->state == 1 && h->timer % 6 < 3) {
			cropX += 40 * frames;
		}
		
		PHL_DrawSurfacePart(h->x - (addx[h->type] * h->dir), h->y, cropX, sheetY[h->type], 40, 40, images[imgEnemies]);
	}
}

//Bullets
void createBullet(int x, int y, int dir, int minid)
{
	int i;
	for (i = minid; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Bullet* b = malloc(sizeof *b);
			b->id = i;
			
			b->x = x;
			b->y = y;
			
			b->hsp = dir * 4;
			
			b->imageIndex = 0;
			
			e->data = b;
			e->enemyStep = bulletStep;
			e->enemyDraw = bulletDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void bulletStep(Bullet* b)
{
	char dead = 0;
	
	//Movement
	{
		b->x += b->hsp;
	}
	
	//Create Mask
	Mask mask;
	{
		mask.unused = 0;
		mask.circle = 1;
		mask.w = mask.h = 10;
		mask.x = b->x;
		mask.y = b->y;
	}
	
	//Animation
	{
		if (b->hsp > 0) {
			b->imageIndex += 0.33;
		}else{
			b->imageIndex -= 0.33;
		}
		
		if (b->imageIndex < 0) {
			b->imageIndex += 4;
		}
		if (b->imageIndex >= 4) {
			b->imageIndex -= 4;
		}
	}	
	
	//Collide with wall
	{
		if (checkTileCollision(1, mask) == 1) {
			createEffect(1, b->x - 20, b->y - 20);
			dead = 1;
		}
	}
	
	//Collide with hero
	{
		//Shield collision
		if (checkCollision(mask, shieldMask) == 1) {
			dead = 1;
			createEffect(1, b->x - 20, b->y - 20);
			PHL_PlaySound(sounds[sndHit07], CHN_EFFECTS);
		}
		//Collide with hero
		else{
			if (checkCollision(getHeroMask(), mask)) {
				heroHit(10, mask.x);
			}
		}
	}
	
	//Destroy if outside of view
	{
		if (b->x > 660 || b->x < -20 || b->y < -20 || b->y > 520) {
			dead = 1;
		}
	}
	
	//Destroy
	{
		if (dead == 1) {
			enemyDestroy(b->id);
		}
	}
}

void bulletDraw(Bullet* b)
{
	PHL_DrawSurfacePart(b->x - 20, b->y - 20, 160 + (40 * (int)b->imageIndex), 480, 40, 40, images[imgMisc20]);
}

//Fireball
void createFireball(int x, int y, int angle, int minid)
{
	//General idea: try to place fireball over spawner
	int i;
	for (i = minid; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Fireball* f = malloc(sizeof *f);
			f->id = i;
			
			f->x = x;
			f->y = y;
			
			f->spd = 3;
			
			f->imageIndex = 0;
			f->angle = angle;
			
			f->mask.circle = 1;
			f->mask.unused = 0;
			f->mask.x = x;
			f->mask.y = y;
			f->mask.w = f->mask.h = 14;
			
			e->data = f;
			e->enemyStep = fireballStep;
			e->enemyDraw = fireballDraw;
			e->type = -1;
			
			enemies[i] = e;
			
			i = MAX_ENEMIES;
		}
	}
}

void fireballStep(Fireball* f)
{	
	f->x += (f->spd) * sin(f->angle * 3.14159 / 180);
	f->y += (f->spd) * cos(f->angle * 3.14159 / 180);
	
	f->mask.x = f->x;
	f->mask.y = f->y;
	
	f->imageIndex += 0.5;
	if (f->imageIndex >= 8) {
		f->imageIndex -= 8;
	}	
	
	//Collide with shield
	if (checkCollision(f->mask, shieldMask)) {
		createEffect(1, f->x - 20, f->y - 20);
		PHL_PlaySound(sounds[sndHit07], CHN_EFFECTS);
		enemyDestroy(f->id);
	}else{
		//Hit player
		if (checkCollision(getHeroMask(), f->mask)) {
			heroHit(10, f->mask.x);
		}
		//Destroy if outside of view
		if (f->x > 660 || f->x < -20 || f->y < -20 || f->y > 520) {
			enemyDestroy(f->id);
		}
	}
}

void fireballDraw(Fireball* f)
{
	PHL_DrawSurfacePart(f->x - 20, f->y - 20, 320 + (40 * (int)f->imageIndex), 440, 40, 40, images[imgMisc20]);
}

//Laser
void createLaser(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Laser* l = malloc(sizeof *l);
			l->id = i;
			
			l->x = x;
			l->y = y;
			
			l->dir = dir;			
			l->imageIndex = 0;
			
			l->mask.circle = l->mask.unused = 0;
			l->mask.x = x;
			l->mask.y = y + 17;
			l->mask.w = 40;
			l->mask.h = 6;
			
			e->data = l;
			e->enemyStep = laserStep;
			e->enemyDraw = laserDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void laserStep(Laser* l)
{
	char dead = 0;
	
	l->x += l->dir * 10;
	l->mask.x = l->x;
	
	l->imageIndex += 0.34;
	if (l->imageIndex >= 2) {
		l->imageIndex -= 2;
	}
	
	if (checkCollision(shieldMask, l->mask)) { //Hit shield		
		PHL_PlaySound(sounds[sndHit07], CHN_EFFECTS);
		createEffect(1, l->x + (20 * l->dir), l->y);
		enemyDestroy(l->id);
		dead = 1;
	}else if (checkCollision(getHeroMask(), l->mask)) {
		heroStone();
		heroHit(15, l->x + 20);
	}
	
	if (dead == 0) {
		if (checkTileCollision(1, l->mask)) {			
			createEffect(1, l->x + (20 * l->dir), l->y);
			enemyDestroy(l->id);
			dead = 1;
		}
		
		if (dead == 0) {
			if (l->mask.x > 640 || l->mask.x + l->mask.w <= 0) {
				enemyDestroy(l->id);
			}
		}
	}
}

void laserDraw(Laser* l)
{
	int dx = 0,
		dy = 480;
	if (l->dir == -1) {
		dx += 80;
	}
	
	PHL_DrawSurfacePart(l->x, l->y, dx + (((int)l->imageIndex) * 40), dy, 40, 40, images[imgMisc20]);
}

//Dragon Flame
void createFlame(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Flame* f = malloc(sizeof *f);
			f->id = i;
			
			f->x = x;
			f->y = y;
			
			f->dir = dir;
			f->timer = 60;
			
			f->imageIndex = 0;			
			
			e->data = f;
			e->enemyStep = flameStep;
			e->enemyDraw = flameDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void flameStep(Flame* f)
{
	f->imageIndex += 0.25;
	
	if (f->timer > 0) {		
		if (f->imageIndex >= 3) {
			f->imageIndex -= 3;
		}
	}
	
	f->timer -= 1;
	
	if (f->timer == 0) {
		f->imageIndex = 3;
	}
	
	//Hero Collision
	{
		Mask mask;
		mask.circle = mask.unused = 0;
		mask.x = f->x;
		mask.y = f->y + 16;
		mask.w = 120;
		mask.h = 18;
		if (f->dir == -1) {
			mask.x -= 120;
		}
		
		if (checkCollision(mask, getHeroMask()) == 1) {
			int centerX = mask.x + 60 - (60 * f->dir);
			
			//Hero is on ladder
			if (getHeroState() == 3) {
				centerX = herox;
			}
			
			heroHit(30, centerX);
		}
	}
	
	if (f->timer < 0 && f->imageIndex >= 6) {
		enemyDestroy(f->id);
	}
}

void flameDraw(Flame* f)
{
	int drawX = f->x,
		drawY = f->y;
		
	int cropX = 0,
		cropY = 0;
		
	if (f->dir == -1) {
		cropX += 720;
		drawX -= 120;
	}
	
	cropX += 120 * (int)f->imageIndex;
	
	while (cropX >= 600) {
		cropX -= 600;
		cropY += 40;
	}
		
	PHL_DrawSurfacePart(drawX, drawY, cropX, cropY, 120, 40, images[imgMisc6020]);
}

//Demon Rock
void createRock(int x, int y, int dir)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Rock* r = malloc(sizeof *r);
			r->id = i;
			
			r->x = x;
			r->y = y;
			
			r->vsp = -3;
			r->dir = dir;
			
			r->imageIndex = 0;			
			
			e->data = r;
			e->enemyStep = rockStep;
			e->enemyDraw = rockDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

void rockStep(Rock* r)
{
	char dead = 0;
	
	//Animate
	{
		r->imageIndex += 0.25 * r->dir;
		if (r->imageIndex >= 8) {
			r->imageIndex -= 8;
		}
		if (r->imageIndex < 0) {
			r->imageIndex += 8;
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.unused = mask.circle = 0;
		mask.x = r->x + 2;
		mask.y = r->y + 2;
		mask.w = 36;
		mask.h = 36;
	}
	
	int hsp = 3;
	double grav = 0.12;
	
	//Movement
	{		
		r->y += r->vsp;		
		r->vsp += grav;
		
		//Collide with floor
		{
			mask.y = r->y + 2;
			
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			
			if (collide.x != -1) {
				PHL_PlaySound(sounds[sndHit06], CHN_ENEMIES);
				r->y = collide.y - mask.h - 2;				
				r->vsp = -3;
				mask.y = r->y + 2;
			}
		}		
		
		r->x += hsp * r->dir;
		
		//Collide with wall
		{
			mask.x = r->x + 2;
			
			PHL_Rect collide = getTileCollision(1, mask);
			
			if (collide.x != -1) {
				dead = 1;
			}
		}
	}
	
	//Collision
	{
		//Hero collision
		if (checkCollision(mask, getHeroMask()) == 1) {
			heroHit(20, mask.x + (mask.w / 2));
		}
		
		//Weapon collision
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (weapons[i]->cooldown == 0) {
					if (checkCollision(mask, weapons[i]->weaponMask)) {
						weaponHit(weapons[i]);
						PHL_PlaySound(sounds[sndHit03], CHN_WEAPONS);

						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}
	
	//Destroy
	if (dead == 1) {
		createRockSmash(r->x + 20, r->y);
		enemyDestroy(r->id);
	}
}

void rockDraw(Rock* r)
{
	PHL_DrawSurfacePart(r->x, r->y, 320 + ((int)r->imageIndex * 40), 160, 40, 40, images[imgEnemies]);
}

//Air Stream
void createAir(int x, int y)
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] == NULL) {
			Enemy* e = malloc(sizeof *e);
			Air* a = malloc(sizeof *a);
			a->id = i;
			
			a->x = x;
			a->y = y;
			
			a->imageIndex = 0;			
			
			e->data = a;
			e->enemyStep = airStep;
			e->enemyDraw = airDraw;
			e->type = -1;
			
			enemies[i] = e;
			i = MAX_ENEMIES;
		}
	}
}

//Air Puff
void airStep(Air* a)
{
	Mask mask;
	mask.circle = mask.unused = 0;
	mask.w = 36;
	mask.h = 30;
	mask.x = a->x + ((40 - mask.w) / 2);	
	
	//Animate
	a->imageIndex += 0.5;
	if (a->imageIndex >= 2) {
		a->imageIndex -= 2;
	}
	
	//Movement
	a->y -= 6;
	mask.y = a->y + (40 - mask.h);
	
	//Collide with player
	if (getHeroState() != 2) {
		if (checkCollision(mask, getHeroMask())) {
			if (hasItem[27] == 0) {
				heroHit(10, mask.x + (mask.w / 2));
			}else{
				//Floating stuff
				if (getHeroVsp() > -5) {
					setHeroVsp(-5);
					setHeroOnground(0);
				}
			}
		}
	}
	
	//destroy if outside of room
	if (mask.y + mask.h < 0) {
		enemyDestroy(a->id);
	}
}

void airDraw(Air* a)
{
	PHL_DrawSurfacePart(a->x, a->y, (int)a->imageIndex * 40, 560, 40, 40, images[imgMisc20]);
}