#ifndef HERO_H
#define HERO_H

#include "PHL.h"
#include "collision.h"

extern double herox, heroy;
extern double herohp, maxhp;
extern int heroAmmo, maxAmmo;
extern int heroWeapon;

extern Mask heroMask;
extern Mask shieldMask;

void heroSetup();
void heroCleanup();
int heroStep();
void heroDraw();

int heroHit(int damage, int centerx);

void heroPoison();
void heroStone();

Mask getHeroMask();

int getHeroState();
void setHeroState(int s);

int getHeroInvincible();

int getHeroDirection();
void setHeroDirection(int d);

double getHeroImageIndex();
void setHeroImageIndex(double index);

double getHeroVsp();
double getHeroHsp();

void setHeroHsp(double newHsp);
void setHeroVsp(double newVsp);

int getHeroOnground();
void setHeroOnground(int val);

void setHeroTimer(int t);

int getHeroPoisoned();

void heroStun();

void setHeroCanjump(int set);

#endif