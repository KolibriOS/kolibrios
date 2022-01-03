#ifndef WEAPON_H
#define WEAPON_H

#include "collision.h"

#define ARROW 0
#define AXE 1
#define BOOMERANG 2
#define FIREBALL 3
#define BOMB 4
#define SWORD 5

typedef struct {
	int id, type;
	
	double x, y;
	double vsp, hsp;	
	double grav;
	double imageIndex;
	int dir;
	
	int power, timer, state, cooldown, hitflag;
		   
	Mask weaponMask;
} Weapon;

void addWeapon(int type, int x, int y);
void weaponStep(Weapon* w);
void weaponDraw(Weapon* w);

void weaponHit(Weapon* w);
void weaponDestroy(int id);

#endif