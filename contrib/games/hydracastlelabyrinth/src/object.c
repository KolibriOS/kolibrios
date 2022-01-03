#include "object.h"
#include "game.h"
#include "hero.h"
#include <stdio.h>
#include <stdlib.h>
#include "enemies/slug.h"
#include "game.h"
#include <math.h>

void nullFunction(void* v);

void ammoStep(Ammo* a);
void ammoDraw(Ammo* a);

void destroyableStep(Destroyable* d);

void secretTriggerStep(SecretTrigger* s);

void chestStep(Chest* c);
void chestDraw(Chest* c);

void savePointStep(SavePoint* s);
void savePointDraw(SavePoint* s);

void doorStep(Door* d);
void doorDraw(Door* d);

void lockBlockStep(LockBlock* l);
void lockBlockDraw(LockBlock* l);

void switchStep(Switch* s);
int switchActivate(Switch* s);
void switchResult(Switch* s);
void switchDraw(Switch* s);

void gateStep(Gate* g);
void gateDraw(Gate* g);

void statueStep(Statue* s);
void statueDraw(Statue* s);

void buttonStep(FloorPad* f);
void buttonDraw(FloorPad* f);

void ladderStep(Ladder* l);
void ladderActivate(int x, int y);

void generatorStep(Generator* g);
void generatorDraw(Generator* g);

void shockgateStep(Shockgate* s);
void shockgateDraw(Shockgate* s);

void crownStep(Crown* c);
void crownDraw(Crown* c);

void nullFunction(void* v)
{
	//Wow, it's literally nothing!
}

void objectDestroy(int id)
{
	if (objects[id] != NULL) {
		if (objects[id]->data != NULL) {
			free(objects[id]->data);
		}
		objects[id]->data = NULL;
		
		free(objects[id]);
	}
	objects[id] = NULL;
}

//Ammo/Health
void spawnCollectable(int x, int y)
{
	int num = (rand() % 100) + 1;
	int result = -1;
	
	int heartchance = 15;
	int ammochance = 10;
	
	if (hasItem[3] == 1) { //Has golden seed
		heartchance *= 2;
		ammochance *= 2;
	}
	
	if (num <= heartchance) {
		result = 1; //Heart
	}
	else if (num > heartchance && num <= heartchance + ammochance) {
		result = 0; //Ammo
	}
	
	//result = rand() % 2;
	
	if (result != -1) {
		createAmmo(x, y, result);
	}
}

void createAmmo(int x, int y, int type)
{
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		if (objects[i] == NULL) {
			Object* o = malloc(sizeof *o);
			Ammo* a = malloc(sizeof *a);
			a->id = i;
			
			a->x = x;
			a->y = y;
			a->type = type;
			
			a->vsp = -2.5;
			a->grav = 0.2;
			a->blink = 30;
			a->canLand = 0;
			a->bounce = 0;

			o->data = a;
			o->objectStep = ammoStep;
			o->objectDraw = ammoDraw;
			o->type = -1;
			
			objects[i] = o;			
			i = MAX_OBJECTS;
		}
	}
}

void ammoStep(Ammo* a)
{
	char dead = 0;
	
	//Flashing animation
	{
		if (a->blink > 0) {
			a->blink -= 1;
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 2;
		mask.h = 1;
		mask.x = a->x - (mask.w / 2);
		mask.y = a->y + (40 - mask.h);
	}
	
	//Movement
	{
		a->y += a->vsp;
		a->vsp += a->grav;
		
		mask.y = a->y + (40 - mask.h);
	}
	
	//Destroy if it falls in a pit
	{
		if (a->y > 480) {
			dead = 1;
		}
	}
	
	//Falling
	if (a->vsp >= 0) {
		//Inside of a block
		if (a->canLand == 0) {
			if (checkTileCollision(1, mask) == 0 && checkTileCollision(3, mask) == 0) {
				a->canLand = 1;
			}
		}
		
		//Land on ground
		if (a->canLand == 1) {
			PHL_Rect collide = getTileCollision(1, mask);
			if (collide.x == -1) {
				collide = getTileCollision(3, mask);
			}
			if (collide.x != -1) {
				
				a->y = collide.y - 40;
				a->vsp = 0;
				
				//Bounce
				if (a->bounce <= 2) {
					double bounceVsp[3] = {-2, -1, 0};
					
					if (a->bounce > 2) {
						a->bounce = 2;
					}
					
					a->vsp = bounceVsp[a->bounce];
					a->bounce += 1;
					PHL_PlaySound(sounds[sndPi02], 2);
				}
			}
		}
	}else{
		a->canLand = 0;
	}
	
	//Setup hero collision mask
	{
		mask.w = 20;
		mask.h = 32;
		//Heart
		if (a->type == 1) {
			mask.w = 28;
			mask.h = 26;
		}		
		mask.x = a->x - (mask.w / 2);
		mask.y = a->y + (40 - mask.h);
	}
	
	//Collect
	{
		if (a->blink <= 0 && checkCollision(mask, heroMask)) {
			//Ammo
			if (a->type == 0) {
				heroAmmo += 5;
				if (heroAmmo > maxAmmo) {
					heroAmmo = maxAmmo;
				}
			}
			//Heart
			else if (a->type == 1) {
				herohp += 10;
				if (herohp > 128) {
					herohp = 128;
				}
			}
			
			dead = 1;
			PHL_PlaySound(sounds[sndGet02], 2);
		}
	}
	
	//Destroy object
	{
		if (dead == 1) {
			objectDestroy(a->id);
		}
	}
	
}

void ammoDraw(Ammo* a)
{
	if (a->blink % 2 == 0) {
		int cropX[2] = {40, 0};
		
		PHL_DrawSurfacePart(a->x - 20, a->y + 2, cropX[a->type], 120, 40, 40, images[imgMisc20]);
	}
}

//Destroyable Block
void createDestroyable(int x, int y, int secret)
{
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		if (objects[i] == NULL) {
			Object* o = malloc(sizeof *o);
			Destroyable* d = malloc(sizeof *d);
			d->id = i;
			
			d->x = x;
			d->y = y;
			d->secret = secret;
			
			d->hp = 3;
			//d->invulnerable = 0;
			
			d->mask.x = x;
			d->mask.y = y;
			d->mask.w = d->mask.h = 40;
			d->mask.unused = d->mask.circle = 0;
			
			o->data = d;
			o->objectStep = destroyableStep;
			o->objectDraw = nullFunction;
			o->type = 3;
			
			objects[i] = o;			
			i = MAX_OBJECTS;
		}
	}
}

void destroyableStep(Destroyable* d)
{
	//if (d->invulnerable <= 0) {
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (weapons[i]->cooldown <= 0) {
					if (checkCollision(d->mask, weapons[i]->weaponMask)) {
						if (hasItem[0] == 1) { //Has copper pick
							d->hp -= 1;
							if (hasItem[1] == 0) {
								PHL_PlaySound(sounds[sndHit03], CHN_WEAPONS);
							}else{ //Has silver pick
								d->hp -= 2;
							}	
						}
						
						//d->invulnerable = 15;
						weaponHit(weapons[i]);
						
						if (d->hp <= 0) {
							createRockSmash(d->x + 20, d->y + 20);
							int sx = d->x / 40;
							int sy = d->y / 40;
							foreground.tileX[sx][sy] = 0;
							foreground.tileY[sx][sy] = 0;
							collisionTiles[sx][sy] = 0;
							PHL_UpdateBackground(background, foreground);
							
							if (d->secret == 0) {
								spawnCollectable(d->x + 20, d->y);
							}else if (d->secret == 1) {
								roomSecret = 1;
							}
							
							objectDestroy(d->id);
							
						}
						
						if (hasItem[0] == 0) {
							PHL_PlaySound(sounds[sndHit03], 1);
						}
					}
				}
			}
		}
	/*}else{
		d->invulnerable -= 1;
	}
	*/
}

//Secret Trigger
void createSecretTrigger(int type, int enemyType, int flag)
{
	if (flag != 0 && flags[flag] == 1) {
		roomSecret = 1;
	}else{
	
		int i;
		for (i = 0; i < MAX_OBJECTS; i++) {
			if (objects[i] == NULL) {
				Object* o = malloc(sizeof *o);
				SecretTrigger* s = malloc(sizeof *s);
				s->id = i;
				s->flag = flag;
				
				s->type = type;
				s->enemyType = enemyType;
				
				o->data = s;
				o->objectStep = secretTriggerStep;
				o->objectDraw = nullFunction;
				o->type = -1;
				
				objects[i] = o;			
				i = MAX_OBJECTS;
			}
		}
	}
}

void secretTriggerStep(SecretTrigger* s)
{
	int i, result = 1;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (s->type == 0) { //Destroy all enemies
			if (enemies[i] != NULL && enemies[i]->type != -1) {
				i = MAX_ENEMIES;
				result = 0;
			}
		}
		else if (s->type == 1) { //Destroy one type of enemy
			if (enemies[i] != NULL && enemies[i]->type == s->enemyType) {
				i = MAX_ENEMIES;
				result = 0;
			}
		}
		else{ //No trigger, only activate on creation
			result = 0;
		}
	}
	
	if (result == 1) {
		if (s->flag != 0) {
			flags[s->flag] = 1;
		}
		roomSecret = 1;
		objectDestroy(s->id);
	}
}

//Chest
void createChest(int x, int y, int item, int secret)
{
	//Don't create if the player already has the item
	int dospawn = 1;
	if (item <= 4) {
		if (hasWeapon[item] == 1) { dospawn = 0; }
	}
	else if (item <= 32) {
		if (hasItem[item - 5] == 1) { dospawn = 0; }
	}
	else if (item <= 40) {
		if (hasKey[item - 33] == 1) { dospawn = 0; }
	}
	
	if (dospawn == 1) {
		int i;
		for (i = 0; i < MAX_OBJECTS; i++) {
			if (objects[i] == NULL) {
				Object* o = malloc(sizeof *o);
				Chest* c = malloc(sizeof *c);
				c->id = i;
				
				c->x = x;
				c->y = y;
				c->item = item;
				c->secret = secret;
				c->timer = 10;
				
				c->visible = 1;
				if (secret == 1 && roomSecret == 0) { //Assume secret trigger is loaded before chest
					c->visible = 0;
					if (hasItem[2] == 1) { //Has Bell
						PHL_PlaySound(sounds[sndBell01], CHN_SOUND);
						//bellFlag = 1;
					}
				}
				
				c->mask.x = x;
				c->mask.y = y;
				c->mask.w = c->mask.h = 40;
				c->mask.unused = c->mask.circle = 0;
				
				o->data = c;
				o->objectStep = chestStep;
				o->objectDraw = chestDraw;
				o->type = 4;
				
				objects[i] = o;			
				i = MAX_OBJECTS;
			}
		}
	}
}

#ifdef EMSCRIPTEN
extern int em_state;
#endif
void chestStep(Chest* c)
{
	if (c->visible == 1) {
		c->timer -= 1;
		if (c->timer <= 0) {
			createEffectExtra(5, c->x + (rand() % 40) + 1, c->y + 4 + (rand() % 40) + 1, 0, 0, 0);
			c->timer = 12;
		}
		
		if (btnUp.pressed == 1) {
			if (getHeroOnground() == 1 && checkCollisionXY(c->mask, herox, heroy + 20) == 1) {
				PHL_PlaySound(sounds[sndGet02], CHN_HERO);
				if (c->item <= 4) {
					hasWeapon[c->item] = 1;
					itemGotX = 40 + (c->item * 40);
					itemGotY = 0;
				}
				else if (c->item <= 32) {
					hasItem[c->item - 5] = 1;
					int itemorder[28] = { 12, 16, 15, 14,  7,  9,  8,
										  17,  3,  5,  4,  6,  2, 10, 
										  13, 11,  0,  1, 21, 25, 26,
										  20, 24, 27, 22, 19, 18, 23 };
					itemGotX = 280 + (itemorder[c->item - 5] * 40);
					itemGotY = 0;
					while (itemGotX >= 640) {
						itemGotX -= 640;
						itemGotY += 40;
					}
				}
				else if (c->item <= 40) {
					hasKey[c->item - 33] = 1;
					itemGotX = 120 + ((c->item - 33) * 40);
					itemGotY = 80;
				}
				
				//Fix no 2nd jump immediatly after getting boots
				if (c->item == 17) {
					setHeroCanjump(1);
				}

				//setHeroState(6); //Set hero state to GETITEM
				int saveItem = c->item;
				
				objectDestroy(c->id);
				#ifdef EMSCRIPTEN
				getItemSetup(saveItem);
				em_state = 50;
				#else
				getItem(saveItem);
				#endif
			}
		}
	}
	else {
		if (roomSecret == 1) {
			c->visible = 1;
			playSecret();
		}
		//Visible if the boss is already defeated
		if (bossDefeatedFlag == 1) {
			c->visible = 1;
		}
	}
}

void chestDraw(Chest* c)
{
	if (c->visible == 1) {
		int dx = 520, dy = 0;
		
		if (c->item > 32) {
			dx = 240;
			dy = 120;
		}
		
		PHL_DrawSurfacePart(c->x, c->y, dx, dy, 40, 40, images[imgMisc20]);
	}
}

//Save point
void createSavePoint(int x, int y, int hidden)
{
	if (hidden == 0 || hasKey[7] == 1) {
		int i;
		for (i = 0; i < MAX_OBJECTS; i++) {
			if (objects[i] == NULL) {
				Object* o = malloc(sizeof *o);
				SavePoint* s = malloc(sizeof *s);
				s->id = i;
				
				s->x = x;
				s->y = y;
				
				s->imageIndex = 0;
				
				s->mask.x = x + 6;
				s->mask.y = y;
				s->mask.w = 28;
				s->mask.h = 40;
				s->mask.unused = s->mask.circle = 0;
				
				o->data = s;
				o->objectStep = savePointStep;
				o->objectDraw = savePointDraw;
				o->type = -1;
				
				objects[i] = o;			
				i = MAX_OBJECTS;
			}
		}
	}
}

void savePointStep(SavePoint* s)
{
	s->imageIndex += 0.15;
	if (s->imageIndex >= 4) {
		s->imageIndex -= 4;
	}
	
	if (btnUp.pressed == 1 && getHeroOnground() == 1) {
		if (checkCollisionXY(s->mask, herox, heroy)) {
			saveScreen();
		}
	}
}

void savePointDraw(SavePoint* s)
{
	PHL_DrawSurfacePart(s->x, s->y, (int)s->imageIndex * 40, 320, 40, 40, images[imgMisc20]);
}

//Door
unsigned char unlockedDoor[8] = {0, 2, 6, 7, 32, 22, 39, 48};

void createDoor(int x, int y, int level, int coords, int warpx, int warpy, int secret)
{
	if (level != 8 || hasKey[7] == 1) {
		int i;
		for (i = 0; i < MAX_OBJECTS; i++) {
			if (objects[i] == NULL) {
				Object* o = malloc(sizeof *o);
				Door* d = malloc(sizeof *d);
				d->id = i;
				
				d->x = x;
				d->y = y;
				
				d->visible = 1;
				d->secret = secret;			
				if (d->secret == 1) {
					d->visible = 0;
				}
				
				d->open = 0;
				if (level == 0) {
					d->open = 1;
				}else{
					d->open = flags[unlockedDoor[level-1]];
				}
							
				d->warplevel = level;
				d->warpcoords = coords;
				d->warpx = warpx;
				d->warpy = warpy;
				
				
				d->mask.x = x + 6;
				d->mask.y = y;
				d->mask.w = 28;
				d->mask.h = 40;
				d->mask.unused = d->mask.circle = 0;
				
				o->data = d;
				o->objectStep = doorStep;
				o->objectDraw = doorDraw;
				o->type = -1;
				
				objects[i] = o;			
				i = MAX_OBJECTS;
			}
		}
	}
}

void doorStep(Door* d)
{
	if (d->visible == 1) {
		if (btnUp.pressed == 1 && getHeroOnground() == 1 && getHeroState() != 6) {
			if (checkCollisionXY(d->mask, herox, heroy)) {
				if (d->open == 0) {
					if (hasKey[d->warplevel - 1] == 1) {
						d->open = 1;
						flags[unlockedDoor[d->warplevel - 1]] = 1;
						//doorUnlocked[d->warplevel-1] = 1;
						PHL_PlaySound(sounds[sndDoor00], CHN_SOUND);
					}
				}else{
					//Setup Door event
					herox = d->x + 20;
					heroy = d->y;
					
					lastDoor = d;
					setHeroState(7); //Set state to DOOR
					PHL_PlaySound(sounds[sndStep01], CHN_HERO);
				}
			}
		}
	}else{
		if (bossDefeatedFlag == 1 && hasKey[getLevel()]) {
			d->visible = 1;
		}
		//Display key and door if the boss didn't show up
		else if (d->secret == 1 && bossFlag == 0) {
			bossFlag = 1;
			bossDefeatedFlag = 1;
			PHL_StopMusic();
		}
	}
}

void doorDraw(Door* d)
{
	if (d->visible == 1) {
		PHL_DrawSurfacePart(d->x, d->y, 600 - (40 * d->open), 0, 40, 40, images[imgMisc20]);
	}
}

//Lock Block
void createLockBlock(int x, int y, int flag)
{
	if (flags[flag] == 0) {
		
		int i;
		for (i = 0; i < MAX_OBJECTS; i++) {
			if (objects[i] == NULL) {
				Object* o = malloc(sizeof *o);
				LockBlock* l = malloc(sizeof *l);
				
				l->id = i;
				l->x = x;
				l->y = y;
				
				l->invincible = 0;
				
				int tx = x / 40;
				int ty = y / 40;
				l->tile = collisionTiles[tx][ty];
				collisionTiles[tx][ty] = 1;
				
				l->flag = flag;
				
				o->data = l;
				o->objectStep = lockBlockStep;
				o->objectDraw = lockBlockDraw;
				o->type = -1;
				
				objects[i] = o;			
				i = MAX_OBJECTS;
			}
		}
		
	}
}

void lockBlockStep(LockBlock* l)
{
	//Collide with weapons
	if (l->invincible <= 0) {
		Mask mask;
		mask.circle = mask.unused = 0;
		mask.x = l->x;
		mask.y = l->y;
		mask.w = mask.h = 40;
		
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (checkCollision(mask, weapons[i]->weaponMask)) {
					l->invincible = 15;
					//Sound
					weaponHit(weapons[i]);
					PHL_PlaySound(sounds[sndHit03], CHN_WEAPONS);

					i = MAX_WEAPONS;
				}
			}	
		}
	}else{
		l->invincible -= 1;
	}
	
	if (roomSecret == 1) {
		playSecret();
		
		int tx = l->x / 40;
		int ty = l->y / 40;
		collisionTiles[tx][ty] = l->tile;
		flags[l->flag] = 1;
		//Destroy
		objectDestroy(l->id);
		/*
		free(objects[l->id]);
		objects[l->id] = NULL;
		*/
	}
}

void lockBlockDraw(LockBlock* l)
{
	PHL_DrawSurfacePart(l->x, l->y, 120, 400, 40, 40, images[imgMisc20]);
}


//Green light switch
void createSwitch(int x, int y, int flag)
{
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		if (objects[i] == NULL) {
			Object* o = malloc(sizeof *o);
			Switch* s = malloc(sizeof *s);
			
			s->id = i;
			s->x = x;
			s->y = y;
			
			s->imageIndex = 0;
			
			s->flag = flag;
			s->activated = 0;
			
			if (flags[s->flag] == 1) {
				switchResult(s);
				s->activated = 1;
			}
			
			o->data = s;
			o->objectStep = switchStep;
			o->objectDraw = switchDraw;
			o->type = 76;
			
			objects[i] = o;			
			i = MAX_OBJECTS;
		}
	}
}

void switchStep(Switch* s)
{
	if (s->activated == 0) {
		if (btnUp.pressed == 1) {
			Mask mask;
			mask.unused = mask.circle = 0;
			mask.w = 16;
			mask.h = 30;
			mask.x = s->x + 12;
			mask.y = s->y + 10;
			
			if (checkCollision(mask, getHeroMask())) {
				if (switchActivate(s) == 1) {
					PHL_PlaySound(sounds[sndPi02], CHN_SOUND);
					playSecret();
				}
			}
		}
	}else{
		s->imageIndex += 0.2;
		if (s->imageIndex >= 3) {
			s->imageIndex -= 2;
		}
	}
}

int switchActivate(Switch* s)
{
	int success = 0;

	if ((s->flag == 24 && hasItem[23] == 1) || //Switch in level 2
		(s->flag == 26 && hasItem[24] == 1)) { //Switch in level 6
			success = 1;
	}
	
	//Switches in level 7
	if ((s->flag == 44 && hasItem[25] == 1) || //Left Switch
		(s->flag == 43 && hasItem[26] == 1))
	{
		PHL_PlaySound(sounds[sndPi02], CHN_SOUND);
		s->activated = 1;
		flags[s->flag] = 1;
		
		if (flags[44] == 1 && flags[43] == 1) {
			flags[45] = 1;
			roomSecret = 1;
			success = 1;
		}
	}
	
	if (success == 1) {
		switchResult(s);
		s->activated = 1;
		flags[s->flag] = 1;
	}
	
	return success;
}

void switchResult(Switch* s)
{
	if (s->flag == 24) { //Switch in level 2
		createPlatform(0, 320, 240, 320, 360, 1, 0);
		flags[23] = 1;
	}
	
	if (s->flag == 26) { //Switch in level 6
		roomSecret = 1;
	}
}

void switchDraw(Switch* s)
{
	PHL_DrawSurfacePart(s->x, s->y, 240 + (40 * (int)s->imageIndex), 560, 40, 40, images[imgMisc20]);
}

//Gates
void createGate(int x, int y, int col)
{
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		if (objects[i] == NULL) {
			Object* o = malloc(sizeof *o);
			Gate* g = malloc(sizeof *g);
			g->id = i;
			
			g->x = x;
			g->y = y;
			
			g->col = col;
			
			g->imageIndex = 0;
			g->timer = 0;
			g->open = 0;
			
			//g->invincible = 0;
			
			o->data = g;
			o->objectStep = gateStep;
			o->objectDraw = gateDraw;
			o->type = 53;
			
			objects[i] = o;	
			i = MAX_OBJECTS;
		}
	}
}

void gateStep(Gate* g)
{
	//Animate
	{
		if (g->open == 1) {
			if (g->imageIndex < 4) {
				g->imageIndex += 0.1;
			}
		}
	}
	
	//Not (fully) opened
	if (g->imageIndex < 4)
	{
		//Setup Mask
		Mask mask;
		{
			mask.unused = mask.circle = 0;
			mask.x = g->x + 11;
			mask.y = g->y;
			mask.w = 18;
			mask.h = 40;
		}
		
		//Collide with player
		{
			if (checkCollision(mask, getHeroMask())) {
				if (getHeroHsp() < 0) {
					herox = mask.x + mask.w + (getHeroMask().w / 2);
				}
				else if (getHeroHsp() > 0) {
					herox = mask.x - (getHeroMask().w / 2);
				}
			}
		}
		
		//Collide with weapons
		{
			if (g->open == 0) {
				int i;
				for (i = 0; i < MAX_WEAPONS; i++) {
					if (weapons[i] != NULL) {
						if (weapons[i]->cooldown == 0) {
							if (checkCollision(mask, weapons[i]->weaponMask)) {
								PHL_PlaySound(sounds[sndHit03], CHN_WEAPONS);							
								
								//Is a sword
								if (weapons[i]->type == 5) {
									if ((g->col == 0 && hasItem[20] == 1) || (g->col == 1 && hasItem[19] == 1)) {
										g->open = 1;
										PHL_PlaySound(sounds[sndDoor00], CHN_SOUND);
									}
								}
								
								weaponHit(weapons[i]);
								
								i = MAX_WEAPONS;
							}
						}
					}
				}
			}
		}
		
	}
}

void gateDraw(Gate* g)
{
	int cx = (int)g->imageIndex * 40,
		cy = 520;
		
	if (g->col == 0) { //Red Gate
		cx += 200;
	}
	
	PHL_DrawSurfacePart(g->x, g->y, cx, cy, 40, 40, images[imgMisc20]);
}

//Statue
void createStatue(int x, int y, int type)
{
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		if (objects[i] == NULL) {
			Object* o = malloc(sizeof *o);
			Statue* s = malloc(sizeof *s);
			s->id = i;
			
			s->x = x;
			s->y = y;
			
			s->type = type;
			
			s->invincible = 0;
			s->hp = 3;
			
			o->data = s;
			o->objectStep = statueStep;
			o->objectDraw = statueDraw;
			o->type = 54;
			
			objects[i] = o;	
			i = MAX_OBJECTS;
		}
	}
}

void statueStep(Statue* s)
{
	Mask mask;
	mask.unused = mask.circle = 0;
	mask.w = 40;
	mask.h = 40;
	mask.x = s->x + ((40 - mask.w) / 2);
	mask.y = s->y;
	
	//Collide with hero
	if (checkCollision(mask, getHeroMask()) == 1) {
		if (getHeroHsp() < 0) {
			herox = mask.x + mask.w + (getHeroMask().w / 2) + 1;
		}
		if (getHeroHsp() > 0) {
			herox = mask.x - (getHeroMask().w / 2);
		}
	}
	
	//Collide with weapons
	if (s->invincible <= 0) {		
		int i;
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (checkCollision(mask, weapons[i]->weaponMask)) {
					s->invincible = 15;
					//Sound
					weaponHit(weapons[i]);					
					
					//Break if you have the right item
					if ((s->type == 0 && hasItem[22] == 1) || (s->type == 1 && hasItem[21] == 1)) {
						s->hp -= 1;
						
						if (s->hp <= 0) {
							createRockSmash(s->x + 20, s->y + 20);
							objectDestroy(s->id);							
						}
					}else{
						PHL_PlaySound(sounds[sndHit03], CHN_WEAPONS);
					}

					i = MAX_WEAPONS;
				}
			}	
		}
	}else{
		s->invincible -= 1;
	}
}

void statueDraw(Statue* s)
{
	int cx = 200,
		cy = 400;
		
	if (s->type == 1) {
		cx += 200;
	}
		
	PHL_DrawSurfacePart(s->x, s->y, cx, cy, 40, 40, images[imgMisc20]);
}

//Button
void createFloorPad(int x, int y, int flag)
{
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		char found = 0;
		
		if (objects[i] == NULL) {
			Object* o = malloc(sizeof *o);
			FloorPad* f = malloc(sizeof *f);
			f->id = i;
			
			f->x = x;
			f->y = y;
			
			f->pressed = 0;
			f->flag = flag;
			
			if (flag != 0 && flags[flag] == 1) {
				roomSecret = 1;
			}
			
			o->data = f;
			o->objectStep = buttonStep;
			o->objectDraw = buttonDraw;
			o->type = 77;
			
			objects[i] = o;	
			i = MAX_OBJECTS;
			found = 1;
		}
		
		if (found == 1) {
			//Create a breakable block if it's covered by a solid block
			int roundx = x / 40,
				roundy = y / 40;

			if (collisionTiles[roundx][roundy] == 1) {
				createDestroyable(x, y, 2);
			}
		}
	}
}

void buttonStep(FloorPad* f)
{
	if (f->pressed == 0) {
		if (getHeroVsp() > 0) {
			Mask mask;
			mask.unused = mask.circle = 0;
			mask.w = 16;
			mask.h = 10;
			mask.x = f->x + ((40 - mask.w) /2);
			mask.y = f->y + (40 - mask.h);
			
			if (checkCollision(mask, getHeroMask()) == 1) {
				f->pressed = 1;
				PHL_PlaySound(sounds[sndHit02], CHN_SOUND);
				
				//Check if there are other buttons unpressed
				int found = 0;
				int i;
				for (i = 0; i < MAX_OBJECTS; i++) {
					if (objects[i] != NULL) {
						if (objects[i]->type == 77) {
							FloorPad* btemp = objects[i]->data;
							if (btemp->pressed == 0) {
								found = 1;
								i = MAX_OBJECTS;
							}
						}
					}
				}
					
				//Activate flag
				if (found == 0) {
					roomSecret = 1;
					
					if (f->flag != 0 && flags[f->flag] == 0) {
						flags[f->flag] = 1;
					}
				}
			}
			
		}
	}
}

void buttonDraw(FloorPad* f)
{
	char covered = 0;
	
	int roundx = f->x / 40,
		roundy = f->y / 40;
	
	//Covered by breakable block
	if (collisionTiles[roundx][roundy] == 1) {
		covered = 1;
	}
	
	if (covered == 0) {
		PHL_DrawSurfacePart(f->x, f->y, 160 + (f->pressed * 40), 320, 40, 40, images[imgMisc20]);
	}
}

//Ladder
void createLadder(int x, int y, int flag)
{	
	if (flags[flag] == 1) {
		ladderActivate(x, y);
	}else{	
		int i;
		for (i = 0; i < MAX_OBJECTS; i++) {			
			if (objects[i] == NULL) {
				Object* o = malloc(sizeof *o);
				Ladder* l = malloc(sizeof *l);
				l->id = i;
				
				l->x = x;
				l->y = y;

				l->flag = flag;
				
				if (getLevel() != 6) {
					if (hasItem[2] == 1) { //Has Bell
						PHL_PlaySound(sounds[sndBell01], CHN_SOUND);
					}
				}
				
				o->data = l;
				o->objectStep = ladderStep;
				o->objectDraw = nullFunction;
				o->type = 79;
				
				objects[i] = o;	
				i = MAX_OBJECTS;
			}
		}
	}
}

void ladderStep(Ladder* l)
{
	if (roomSecret == 1) {
		playSecret();
		flags[l->flag] = 1;
		ladderActivate(l->x, l->y);
		objectDestroy(l->id);
	}
}

void ladderActivate(int x, int y)
{
	y = (int)(y / 40);
	x = (int)(x / 40);
	
	while (y >= 0) {		
		foreground.tileX[x][y] = 3;
		foreground.tileY[x][y] = 0;
		
		collisionTiles[x][y] = 2;
		y -= 1;
	}
	
	PHL_UpdateBackground(background, foreground);
}

//Generator
void createGenerator(int x, int y, int flag)
{
	if (flags[flag] == 0) {
		int i;
		for (i = 0; i < MAX_OBJECTS; i++) {			
			if (objects[i] == NULL) {
				Object* o = malloc(sizeof *o);
				Generator* g = malloc(sizeof *g);
				g->id = i;
				g->hp = 3;
				g->blink = 0;
				
				g->x = x;
				g->y = y;
				
				g->imageIndex = 0;

				g->flag = flag;
				
				o->data = g;
				o->objectStep = generatorStep;
				o->objectDraw = generatorDraw;
				o->type = 80;
				
				objects[i] = o;	
				i = MAX_OBJECTS;
			}
		}
	}
	
}

void generatorStep(Generator* g)
{	
	//Animate
	{
		g->imageIndex += 0.33;
		if (g->imageIndex >= 2) {
			g->imageIndex -= 2;
		}
		
		if (g->blink > 0) {
			g->blink -= 1;
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 24;
		mask.h = 40;
		mask.x = g->x + ((40 - mask.w) / 2);
		mask.y = g->y;
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
						
						i = MAX_WEAPONS;
					}
				}
			}	
		}
	}
	
	//Destroy
	{
		if (g->hp <= 0) {
			createRockSmash(g->x + 20, g->y + 20);
			flags[g->flag] = 1;
			//if no generators exist
			{
				char found = 0;
				
				int i;
				for (i = 0; i < MAX_OBJECTS; i++) {
					if (i != g->id && objects[i] != NULL) {
						if (objects[i]->type == 80) {
							found = 1;
							i = MAX_OBJECTS;
						}
					}
				}
				
				if (found == 0) {
					roomSecret = 1;
					playSecret();
				}
			}
			objectDestroy(g->id);
		}
	}
	
}

void generatorDraw(Generator* g)
{
	if (g->blink % 2 == 0) {
		PHL_DrawSurfacePart(g->x, g->y, 80 + ((int)g->imageIndex * 40), 600, 40, 40, images[imgMisc20]);
	}
}

//Electric gate
void createShockgate(int x, int y, int flag)
{
	if (flags[flag] == 0) {
		int i;
		for (i = 0; i < MAX_OBJECTS; i++) {			
			if (objects[i] == NULL) {
				Object* o = malloc(sizeof *o);
				Shockgate* s = malloc(sizeof *s);
				s->id = i;
				
				s->x = x;
				s->y = y;
				
				s->imageIndex = 0;

				s->flag = flag;
				
				o->data = s;
				o->objectStep = shockgateStep;
				o->objectDraw = shockgateDraw;
				o->type = 56;
				
				objects[i] = o;	
				i = MAX_OBJECTS;
			}
		}
	}
	
}

void shockgateStep(Shockgate* s)
{
	//Animate
	{
		s->imageIndex += 0.33;
		if (s->imageIndex >= 4) {
			s->imageIndex -= 4;
		}
	}
	
	//Setup Mask
	Mask mask;
	{
		mask.circle = mask.unused = 0;
		mask.w = 40 * 3;
		mask.h = 8;
		mask.x = s->x;
		mask.y = s->y + ((40 - mask.h) / 2);
	}
	
	//Hero Collision
	{
		if (checkCollision(mask, getHeroMask()) == 1) {
			heroy = mask.y - 40;
			setHeroVsp(0);
		}
	}
	
	//Destroy
	if (roomSecret == 1) {
		flags[s->flag] = 1;
		objectDestroy(s->id);
	}
}

void shockgateDraw(Shockgate* s)
{
	int i;
	for (i = 0; i < 3; i++) {
		PHL_DrawSurfacePart(s->x + (40 * i), s->y, 80 + ((int)s->imageIndex * 40), 560, 40, 40, images[imgMisc20]);
	}
}

//Crown
void createCrown(int x, int y)
{
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {			
		if (objects[i] == NULL) {
			Object* o = malloc(sizeof *o);
			Crown* c = malloc(sizeof *c);
			c->id = i;
			
			c->x = x;
			c->y = y;
			
			c->ystart = c->y;
			c->bobRot = 0;
			
			c->imageIndex = 0;
			
			c->visible = 0;
			c->timer = 0;
			
			o->data = c;
			o->objectStep = crownStep;
			o->objectDraw = crownDraw;
			o->type = 57;
			
			objects[i] = o;	
			i = MAX_OBJECTS;
		}
	}
}

void crownStep(Crown* c)
{
	if (c->visible == 0) {
		if (roomSecret == 1) {
			c->visible = 1;
			playSecret();
		}
	}else{
		//Animate
		{
			c->imageIndex += 0.33;
			if (c->imageIndex >= 2) {
				c->imageIndex -= 2;
			}
			
			//Movement
			c->bobRot += 6;
			if (c->bobRot >= 360) {
				c->bobRot -= 360;
			}
			c->y = c->ystart + sin(c->bobRot * 3.14159 / 180) * 5;
		}
		
		//Sparkle
		c->timer -= 1;
		if (c->timer <= 0) {
			createEffectExtra(5, c->x + (rand() % 40) + 1, c->y + 4 + (rand() % 40) + 1, 0, 0, 0);
			c->timer = 12;
		}
		
		//Setup Mask
		Mask mask;
		{
			mask.circle = mask.unused = 0;
			mask.w = 15;
			mask.h = 11;
			mask.x = c->x + 20 - (mask.w / 2);
			mask.y = c->y + 20 - (mask.h / 2);
		}
		
		if (checkCollision(mask, getHeroMask()) == 1) {
			objectDestroy(c->id);
			gameEnding();
		}
		
	}
}

void crownDraw(Crown* c)
{
	if (c->visible == 1) {
		int cropX = (int)c->imageIndex * 40;
		
		PHL_DrawSurfacePart(c->x, c->y, cropX, 200, 40, 40, images[imgHero]);
	}
}