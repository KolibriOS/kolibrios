#include "weapon.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hero.h"
#include "PHL.h"
#include "game.h"
#include "object.h"

void updateWeaponMask(Weapon* w);

void addWeapon(int type, int x, int y)
{
	if (heroAmmo > 0 || type == SWORD) {
		int i, weaponcount = 0;
		
		//Count
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				if (weapons[i]->type != SWORD) {
					weaponcount += 1;
				}
			}
		}
		
		if (type == BOMB) { //Bombs have one lower limit
			weaponcount += 1;
		}
		
		if (type == SWORD || weaponcount < (2 + hasItem[15])) { //3 projectiles if have green scroll
		
			if (type != SWORD) {
				PHL_PlaySound(sounds[sndShot01], CHN_WEAPONS);
			}
		
			for (i = 0; i < MAX_WEAPONS; i++) {
				if (weapons[i] == NULL) {			
					Weapon* w = malloc(sizeof *w);
					w->id = i;
					w->type = type;

					w->x = x;
					w->y = y;

					w->hsp = 0;
					w->vsp = 0;
					
					w->grav = 0;					
					w->imageIndex = 0;
					
					w->power = 1;
					w->timer = 0;
					w->state = 0;
					w->cooldown = 0;
					w->hitflag = 0;
					
					w->dir = getHeroDirection();
					
					w->weaponMask.circle = 0;
					w->weaponMask.unused = 0;
					w->weaponMask.x = 0;
					w->weaponMask.y = 0;
					w->weaponMask.w = 10;
					w->weaponMask.h = 10;
					
					if (w->type == SWORD)
					{
						w->weaponMask.unused = 1;						
					}else if (w->type == ARROW)
					{
						w->hsp = 8 * getHeroDirection();
						w->weaponMask.y += 16;
						w->weaponMask.w = 32;
						w->weaponMask.h = 6;
					}else if (w->type == AXE)
					{
						w->hsp = 2 * getHeroDirection();
						w->vsp = -6;
						w->grav = 0.2;
						w->weaponMask.w = w->weaponMask.h = 24;
						w->y += (40 - getHeroMask().h) - ((40 - w->weaponMask.h) / 2);
						w->weaponMask.x = w->x + ((40 - w->weaponMask.w) / 2);
						w->weaponMask.y = w->y + ((40 - w->weaponMask.h) / 2);
						
					}else if (w->type == BOOMERANG)
					{
						w->x += 20;
						w->y += 20;
						updateWeaponMask(w);
						w->weaponMask.circle = 1;
						w->weaponMask.w = 16;
						
						w->hsp = 6 * getHeroDirection();
						w->vsp = getHeroDirection(); //Vsp is used for starting direction
						w->timer = 120;
					}else if (w->type == FIREBALL)
					{
						w->vsp = -2;
						w->grav = 0.1;
						
						w->hsp = 2 * getHeroDirection();
						
						w->x += 20;
						w->y += 20;
						
						w->weaponMask.circle = 1;
						w->weaponMask.w = 15;
						updateWeaponMask(w);
						
						Mask tempMask;
						tempMask.circle = tempMask.unused = 0;
						tempMask.y = w->y - w->weaponMask.w;
						tempMask.x = w->x - w->weaponMask.w;
						tempMask.w = tempMask.h = w->weaponMask.w * 2;						
						
						PHL_Rect collide = getTileCollision(1, tempMask);
						if (collide.x == -1) {
							collide = getTileCollision(3, tempMask);
						}
						if (collide.x != -1) {
							w->y = collide.y + 40 + w->weaponMask.w;
						}
						
					}else if (w->type == BOMB)
					{
						w->dir = 0;
						w->vsp = -2;
						w->grav = 0.1;
						w->hsp = getHeroDirection();
						
						w->weaponMask.unused = 1;
						
						Mask tempMask;		
						tempMask.y = w->y + 8;
						tempMask.x = w->x + 8;
						tempMask.w = tempMask.h = 24;
						tempMask.circle = tempMask.unused = 0;
						
						PHL_Rect collide = getTileCollision(1, tempMask);
						if (collide.x == -1) {
							collide = getTileCollision(3, tempMask);
						}
						if (collide.x != -1) {
							w->y = collide.y + 40 - 8;
						}
					}
					
					updateWeaponMask(w);
					
					weapons[i] = w;
					i = MAX_WEAPONS;
					
					if (type != SWORD) {
						heroAmmo -= 1;
					}
				}
			}
		}
	}
}

//When a weapon lands a hit
void weaponHit(Weapon* w)
{	
	//w->cooldown = 15;
	w->hitflag = 1;
	
	if (w->type == SWORD) {
		createEffect(1, w->weaponMask.x + (w->weaponMask.w / 2) - 10 + (rand() % 20) - 10, w->weaponMask.y + (w->weaponMask.h / 2) - 10 + (rand() % 20) - 10);
		PHL_PlaySound(sounds[sndHit02], CHN_WEAPONS);
	}else if (w->type == ARROW || w->type == AXE) {
		PHL_PlaySound(sounds[sndPi02], CHN_WEAPONS);
		createEffect(1, w->x, w->y);
		weaponDestroy(w->id);
	}else if (w->type == BOOMERANG) {
		PHL_PlaySound(sounds[sndPi02], CHN_WEAPONS);
		createEffect(1, w->x, w->y - 40 + (rand() % 40) + 1);
	}else if (w->type == FIREBALL) {
		PHL_PlaySound(sounds[sndPi02], CHN_WEAPONS);
		createEffect(1, w->x - 20, w->y - 20);
		//weaponDestroy(w->id);
	}else if (w->type == BOMB) {
		
	}
}

void weaponStep(Weapon* w)
{
	char dead = 0;
	
	if (w->cooldown > 0) {
		w->cooldown -= 1;
	}
	
	if (w->hitflag == 1) {
		w->hitflag = 0;
		w->cooldown = 15;
	}
	
	if (w->type == SWORD)
	{
		double imgind = getHeroImageIndex();
		if ((getHeroState() != 1 && getHeroState() != 5) || imgind >= 4) { //Not slashing
			dead = 1;
		}
		
		w->x = herox;
		w->y = heroy;
		updateWeaponMask(w);
			
	}else if (w->type == ARROW)
	{
		//Yes, this does have to be done before movement
		//Destroy if out of view or collides with wall
		if (w->x > 640 || w->x + 40 < 0 || checkTileCollision(1, w->weaponMask) == 1) {
			weaponHit(w);
		}
		
		w->x += w->hsp;
		updateWeaponMask(w);
	}else if (w->type == AXE)
	{
		w->y += w->vsp;
		w->vsp += w->grav;
		updateWeaponMask(w);
		
		PHL_Rect collide = getTileCollisionWeapon(1, w->weaponMask);
		if (collide.x == -1) {
			collide = getTileCollision(3, w->weaponMask);
		}
		
		if (collide.x != -1) {
			if (w->vsp <= 0) {
				w->y = collide.y + 40 - ((40 - w->weaponMask.h) / 2);
				w->vsp = 0;
			}else{
				weaponHit(w);
				return;
			}
		}
		
		w->x += w->hsp;
		updateWeaponMask(w);
		
		collide = getTileCollisionWeapon(1, w->weaponMask);
		
		if (collide.x != -1) {
			if (w->hsp > 0) {
				w->x = collide.x - 40 + ((40 - w->weaponMask.w) / 2);
			}else{
				w->x = collide.x + 40 - ((40 - w->weaponMask.w) / 2);
			}
		}			
		
		//Animate
		{
			w->imageIndex += 0.33;
			if (w->imageIndex >= 8) {
				w->imageIndex -= 8;
			}
		}
		
		if (w->x > 640 || w->x + 40 < 0 || w->y > 520) {
			dead = 1;
		}
	}else if (w->type == BOOMERANG)
	{
		w->x += w->hsp;
		w->hsp -= 0.125 * (w->vsp); //Vsp is recycled to be the starting direction
		if (w->hsp >= 6) {
			w->hsp = 6;
		}
		if (w->hsp <= -6) {
			w->hsp = -6;
		}
		
		w->dir = 1;
		if (w->hsp < 0) {
			w->dir = -1;
		}
		
		w->y = heroy + 20;
		updateWeaponMask(w);
		
		w->imageIndex -= (0.33 * w->vsp);
		if (w->imageIndex < 0) {
			w->imageIndex += 8;
		}
		if (w->imageIndex >= 8) {
			w->imageIndex -= 8;
		}
		
		w->timer -= 1;
		if (w->timer <= 0) {
			createEffect(5, w->x, w->y);
			dead = 1;
		}
	}else if (w->type == FIREBALL)
	{		
		//Move vertically
		w->y += w->vsp;
		w->vsp += w->grav;

		Mask tempMask;
		tempMask.circle = tempMask.unused = 0;
		tempMask.y = w->y - w->weaponMask.w;
		tempMask.w = tempMask.h = w->weaponMask.w * 2;
		tempMask.x = w->x - w->weaponMask.w;
		
		//Check vertical collision
		PHL_Rect collide = getTileCollisionWeapon(1, tempMask);
		if (collide.x == -1) {
			collide = getTileCollisionWeapon(3, tempMask);
		}
		
		if (collide.x != -1) {
			if (w->vsp <= 0) {
				w->y = collide.y + 40 + w->weaponMask.w;
				w->vsp = 0;
			}else{
				w->y = collide.y - w->weaponMask.w;
				if (w->timer == 0) {
					w->vsp = -2;
					PHL_PlaySound(sounds[sndPi02], 2);
				}else if (w->timer == 1) {
					w->vsp = -1;
					PHL_PlaySound(sounds[sndPi02], 2);
				}else{
					weaponHit(w);
					dead = 1;
				}
				w->timer += 1;			
			}
			
			tempMask.y = w->y - w->weaponMask.w;
		}
		
		//Move horizontally
		w->x += w->hsp;
		tempMask.x = w->x - w->weaponMask.w;
		
		//Check horizontal collision
		collide = getTileCollisionWeapon(1, tempMask);
		if (collide.x != -1) {
			if (w->hsp > 0) {
				w->x = collide.x - (tempMask.w / 2) ;
			}else{
				w->x = collide.x + 40 + (tempMask.w / 2);
			}
			w->hsp *= -1;
			tempMask.x = w->x - w->weaponMask.w;
		
			w->dir = 1;
			if (w->hsp < 0) {
				w->dir = -1;
			}
		}
		
		updateWeaponMask(w);
		
		//animate
		{
			w->imageIndex += 0.5;
			if (w->imageIndex >= 8) {
				w->imageIndex -= 8;
			}
		}
		
		if (w->x > 640 || w->x + 40 < 0 || w->y > 520) {
			dead = 1;
		}
	}else if (w->type == BOMB)
	{
		if (w->state == 0) { //Bouncing bomb
			Mask tempMask;		
			tempMask.y = w->y  + 8;
			tempMask.w = tempMask.h = 24;
			tempMask.circle = tempMask.unused = 0;
			
			w->x += w->hsp;
			tempMask.x = w->x + 8;
			PHL_Rect collide = getTileCollision(1, tempMask);
			if (collide.x != -1) {
				if (w->hsp > 0) {
					w->x = collide.x - 40 + 8;
				}else{
					w->x = collide.x + 40 - 8;
				}
				w->hsp *= -1;
				tempMask.x = w->x + 8;
			}
			
			w->imageIndex -= (0.33 * w->hsp);
			if (w->imageIndex < 0) {
				w->imageIndex += 8;
			}
			if (w->imageIndex >= 8) {
				w->imageIndex -= 8;
			}
			
			w->y += w->vsp;
			w->vsp += w->grav;
			
			tempMask.y = w->y + 8;
			collide = getTileCollision(1, tempMask);
			if (collide.x == -1) {
				collide = getTileCollision(3, tempMask);
			}
			if (collide.x != -1) {
				if (w->vsp <= 0) {
					w->y = collide.y + 40 - 8;
					w->vsp = 0;
				}else{
					w->y = collide.y - 40 + 8;
					if (w->timer == 0) {
						w->vsp = -2;
						PHL_PlaySound(sounds[sndPi02], CHN_WEAPONS);
					}else if (w->timer == 1) {
						w->vsp = -1;
						PHL_PlaySound(sounds[sndPi02], CHN_WEAPONS);
					}else{
						w->state = 1;
						w->imageIndex = 0;
						w->weaponMask.unused = 0;
						PHL_PlaySound(sounds[sndBom03], CHN_WEAPONS);
					}
					w->timer += 1;
				}
			}
			
			updateWeaponMask(w);
				
			if (w->x > 640 || w->x + 40 < 0 || w->y > 520) {
				//weaponDestroy(w->id);
				dead = 1;
			}
		}
		else if (w->state == 1) { //Explosion
			updateWeaponMask(w);
			
			if (checkCollision(getHeroMask(), w->weaponMask) == 1) {
				heroHit(20, w->x + 20);
			}
			
			w->imageIndex += 0.34;
			if (w->imageIndex >= 11) {
				//weaponDestroy(w->id);
				dead = 1;
			}
		}
	}
	
	if (dead == 1) {
		weaponDestroy(w->id);
	}
}

void weaponDraw(Weapon* w)
{
	if (w->type == SWORD) {		
		//PHL_DrawMask(w->weaponMask);
		
		//Draw Sword
		double imgind = getHeroImageIndex();
		int dir = getHeroDirection();
		if (imgind < 4) {
			int swordx = 0, swordy = 0;
			int scropx = 40 * (int)floor(imgind),
				scropy = 240;
			if (imgind < 1) {
				swordy = -16;
				swordx = -8;
			}else if (imgind < 2) {
				swordy = -8;
				swordx = 24;
			}else if (imgind < 4) {
				swordy = 14;
				swordx = 26;
			}
			
			if (dir == -1) {
				swordx *= -1;
				scropx += 160;
			}
			
			if (getHeroInvincible() % 2 == 0) {
				PHL_DrawSurfacePart(herox - 20 + swordx, heroy + swordy, scropx, scropy, 40, 40, images[imgHero]);
			}
			
			//PHL_DrawSurfacePart(w->x, w->y, 0, 240, 40, 40, images[imgHero]);
		}
		
	}
	else if (w->type == ARROW)
	{
		int dx = 240;
		if (w->hsp <= 0) {
			dx += 40;
		}
		PHL_DrawSurfacePart(w->x, w->y, dx, 200, 40, 40, images[imgMisc20]);
	}else if (w->type == AXE)
	{
		int dx = 0;
		if (w->hsp <= 0) {
			dx = 320;
		}
		PHL_DrawSurfacePart(w->x, w->y, dx + ((int)w->imageIndex * 40), 240, 40, 40, images[imgMisc20]);
	}else if (w->type == BOOMERANG)
	{
		PHL_DrawSurfacePart(w->x - 20, w->y - 20, 320 + ((int)w->imageIndex * 40), 160, 40, 40, images[imgMisc20]);
	}else if (w->type == FIREBALL)
	{
		int dx = 0;
		if (w->hsp <= 0) {
			dx = 320;
		}
		PHL_DrawSurfacePart(w->x - 20, w->y - 20, dx + ((int)w->imageIndex * 40), 280, 40, 40, images[imgMisc20]);
	}else if (w->type == BOMB)
	{
		if (w->state == 0) {
			PHL_DrawSurfacePart(w->x, w->y, (int)w->imageIndex * 40, 160, 40, 40, images[imgMisc20]);
		}
		else if (w->state == 1) {
			//PHL_DrawMask(w->weaponMask);
			
			int cx = (int)w->imageIndex * 128;
			int cy = 0;
			while (cx >= 640) {
				cx -= 640;
				cy += 96;
			}
			PHL_DrawSurfacePart(w->weaponMask.x, w->weaponMask.y, cx, cy, 128, 96, images[imgExplosion]);
		}
	}
	//PHL_DrawMask(w->weaponMask);
}

void updateWeaponMask(Weapon* w)
{
	if (w->type == SWORD) {
		//Sword mask
		//swordMask.unused = 0;
		double imgind = getHeroImageIndex();
		int dir = getHeroDirection();
		w->weaponMask.unused = 0;
		
		if (imgind < 1) {
			/*w->weaponMask.w = 8;
			w->weaponMask.h = 24;
			w->weaponMask.x = herox - 4 + (dir * -8); //herox - 20 + (direction * -8) + 16
			w->weaponMask.y = heroy - 8; //heroy - 16 + 8
			*/
			w->weaponMask.unused = 1;
		}else if (imgind < 2) {
			w->weaponMask.w = 32;
			w->weaponMask.h = 38;
			w->weaponMask.x = herox - 20 + (dir * 28) + 4;
			w->weaponMask.y = heroy - 6;
		}else if (imgind < 3) {
			w->weaponMask.w = 24;
			w->weaponMask.h = 20;
			w->weaponMask.x = herox - 20 + (dir * 26) + 8;
			w->weaponMask.y = heroy + 18;
		}else if (imgind < 4) {
			w->weaponMask.w = 24;
			w->weaponMask.h = 6;
			w->weaponMask.x = herox - 20 + (dir * 26) + 8;
			w->weaponMask.y = heroy + 30;
		}
	}else if (w->type == ARROW) {
		w->weaponMask.x = w->x;
		w->weaponMask.y = w->y + 16;
		if (w->hsp > 0) {
			w->weaponMask.x += 8;
		}
		
	}else if (w->type == AXE) {
		w->weaponMask.x = w->x + ((40 - w->weaponMask.w) / 2);
		w->weaponMask.y = w->y + ((40 - w->weaponMask.h) / 2);
	}else if (w->type == BOOMERANG) {
		w->weaponMask.x = w->x;
		w->weaponMask.y = w->y;
	}else if (w->type == FIREBALL) {
		w->weaponMask.x = w->x;
		w->weaponMask.y = w->y;
	}else if (w->type == BOMB) {
		if (w->state == 1) { //Update mask on explosion
			w->weaponMask.x = w->x - 44;
			w->weaponMask.y = w->y - 44 - 8;
			w->weaponMask.w = 128;
			w->weaponMask.h = 90; //Hits blocks below
		}
	}
}

void weaponDestroy(int id)
{	
	if (weapons[id] != NULL) {
		free(weapons[id]);
	}
	weapons[id] = NULL;
}
