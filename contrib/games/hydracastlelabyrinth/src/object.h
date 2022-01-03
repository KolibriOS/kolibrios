#ifndef OBJECT_H
#define OBJECT_H

#include "collision.h"

typedef struct {
	void* data; //Specific object struct
	void (*objectStep)();
	void (*objectDraw)();
	int type;
} Object;

void objectDestroy(int id);

//Health/Ammo collectables
typedef struct {
	int id, type; //0 for ammo, 1 for heart
	double x, y,
		   vsp, grav;
		   
	int blink, canLand, bounce;
} Ammo;

void spawnCollectable(int x, int y);
void createAmmo(int x, int y, int type);

//Destroyable blocks
typedef struct {
	int id;
	int x, y;
	int hp/*, invulnerable*/;
	int secret;
	Mask mask;
} Destroyable;

void createDestroyable(int x, int y, int secret);

//Secret Trigger
typedef struct {
	int id, flag;
	int type, enemyType;
} SecretTrigger;

void createSecretTrigger(int type, int enemyType, int flag);

//Chest
typedef struct {
	int id;
	int x, y;
	int item, secret;
	int visible;
	int timer;
	
	Mask mask;
} Chest;

void createChest(int x, int y, int item, int secret);

//Save point
typedef struct {
	int id;
	int x, y;
	double imageIndex;
	
	Mask mask;
} SavePoint;

void createSavePoint(int x, int y, int hidden);

//Door
typedef struct {
	int id;
	int x, y;
	int open, secret, visible;
	
	int warplevel, warpcoords;
	int warpx, warpy;
	
	Mask mask;
} Door;

void createDoor(int x, int y, int level, int coords, int warpx, int warpy, int secret);

//Lock Block
typedef struct {
	int id, flag;
	int x, y;
	int tile;
	int invincible;
} LockBlock;

void createLockBlock(int x, int y, int flag);

//Light Switch
typedef struct {
	int id, flag;
	int x, y;
	int activated;
	double imageIndex;
} Switch;

void createSwitch(int x, int y, int flag);

//Blue/Red Gates
typedef struct {
	int id;
	int x, y;
	int col;
	int timer, open;
	//int invincible;
	double imageIndex;	
} Gate;

void createGate(int x, int y, int col);

//Statue
typedef struct {
	int id;
	int x, y;
	int type;
	int invincible;
	int hp;
} Statue;

void createStatue(int x, int y, int type);

//Button
typedef struct {
	int id;
	int x, y;
	int flag;
	int pressed;
} FloorPad;

void createFloorPad(int x, int y, int flag);

//Ladder
typedef struct {
	int id;
	int x, y;
	int flag;
} Ladder;

void createLadder(int x, int y, int flag);

//Generator
typedef struct {
	int id;
	int hp;
	int blink;
	int x, y;
	double imageIndex;
	int flag;
} Generator;

void createGenerator(int x, int y, int flag);

//Electric gate
typedef struct {
	int id;
	int x, y;
	double imageIndex;
	int flag;
} Shockgate;

void createShockgate(int x, int y, int flag);

//Ending crown
typedef struct {
	int id;
	int x, ystart;
	double y;
	double bobRot;
	double imageIndex;
	int timer;
	char visible;
} Crown;

void createCrown(int x, int y);

#endif