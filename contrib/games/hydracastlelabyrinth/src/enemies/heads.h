#ifndef HEADS_H
#define HEADS_H

#include "../collision.h"

//Goblin/medusa/dragon head statues
typedef struct {
	int id, type; //0 = Rhyno head | 1 = Goblin | 2 = Dragon | 3 = Demon | 4 = Fireball | 5 = Air Jar
	int state, timer;
	double x, y;
	int dir;
	int hp, invincible;
	int cooloff;
	int counter;
	
	//Mask mask;
} Head;

void createHead(int type, int x, int y, int dir, int offset, int cooloff);

//Bullet from Rhyno statues
typedef struct {
	int id;
	double x, y;
	int hsp;
	double imageIndex;
	
	//Mask mask;
} Bullet;

void createBullet(int x, int y, int dir, int minid); //Minid is the spawner's id

//Fireball
typedef struct {
	int id;
	double x, y;
	int angle;
	int spd;
	double imageIndex;
	
	Mask mask;
} Fireball;

void createFireball(int x, int y, int angle, int minid);

//Medusa lazer
typedef struct {
	int id;
	double x, y;
	int dir;
	double imageIndex;
	
	Mask mask;
} Laser;

void createLaser(int x, int y, int dir);

//Dragon flame
typedef struct {
	int id;
	int x, y;
	int dir;
	int timer;
	double imageIndex;
} Flame;

void createFlame(int x, int y, int dir);

//Demon Boulder
typedef struct {
	int id;
	double x, y;
	double vsp;
	int dir;
	double imageIndex;
} Rock;

void createRock(int x, int y, int dir);

//Air
typedef struct {
	int id;
	double x, y;
	double imageIndex;
} Air;

void createAir(int x, int y);

#endif