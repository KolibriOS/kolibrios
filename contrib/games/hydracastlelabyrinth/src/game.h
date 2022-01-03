#ifndef GAME_H
#define GAME_H

#include "PHL.h"
#include "enemy.h"
#include "enemies/slime.h"
#include "enemies/bat.h"
#include "enemies/slug.h"
#include "enemies/knight.h"
#include "enemies/heads.h"
#include "enemies/gas.h"
#include "enemies/skull.h"
#include "enemies/fish.h"
#include "enemies/waterjumper.h"
#include "enemies/podoboo.h"
#include "enemies/thwomp.h"
#include "enemies/dodo.h"
#include "enemies/batboss.h"
#include "enemies/crab.h"
#include "enemies/skeleton.h"
#include "enemies/ghoul.h"
#include "enemies/seal.h"
#include "enemies/jellyfish.h"
#include "enemies/wizard.h"
#include "enemies/pendulum.h"
#include "enemies/gyra.h"
#include "enemies/lolidra.h"
#include "enemies/bee.h"
#include "enemies/devil.h"
#include "enemies/firewheel.h"
#include "enemies/boar.h"
#include "enemies/golem.h"
#include "enemies/garm.h"
#include "enemies/poisonknight.h"
#include "enemies/dog.h"
#include "enemies/boomknight.h"
#include "enemies/pumpkin.h"
#include "enemies/hydra.h"
#include "object.h"
#include "effect.h"
#include "weapon.h"
#include "platform.h"

#define TITLE 0
#define	GAME 1
#define INVENTORY 2
#define OPTIONS 3
#define SAVING 4
#define LEVELSTART 5
#define GETITEM 6

//Sound channels
#define CHN_MUSIC 0
#define CHN_SOUND 1 //Various sounds, like menus and fanfares
#define CHN_HERO 2
#define CHN_WEAPONS 3
#define CHN_ENEMIES 4
#define CHN_EFFECTS 5

extern Door* lastDoor;

extern int secretTimer;
extern int levelStartTimer;
extern int saveTimer;

extern int quakeTimer;

extern int bellFlag;
extern int bossFlag;
extern int bossDefeatedFlag;

extern char roomDarkness;

//Used for item get message
extern int itemGotX;
extern int itemGotY;

extern int roomSecret;

extern int collisionTiles[16][12];

//Playtime in frames. At 60 frames per second can hold ~828 1/2 days worth of playtime if my math isn't shit
extern unsigned long playTime;

//Inventory
extern unsigned char hasWeapon[5];
extern unsigned char hasItem[28];
extern unsigned char hasKey[8];

//Save data flags
extern unsigned char flags[60];

extern PHL_Background background,
			   foreground;

//Game assets
extern PHL_Surface images[15];
extern PHL_Music 	bgmMusic;
extern PHL_Music	bgmSecret;
extern PHL_Music	bgmGameover;
extern PHL_Sound	sounds[43];

#define MAX_WEAPONS 5
extern Weapon* weapons[MAX_WEAPONS];

#define MAX_OBJECTS 40
extern Object* objects[MAX_OBJECTS];

#define MAX_ENEMIES 20
extern Enemy* enemies[MAX_ENEMIES];

#define MAX_EFFECTS 30
extern Effect* effects[MAX_EFFECTS];

#define MAX_PLATFORMS 10
extern Platform* platforms[MAX_PLATFORMS];

//Graphic names
#define imgTiles 0
#define imgEnemies 1
#define imgHud 2
#define imgMisc20 3
#define imgMisc32 4
#define imgHero 5
#define imgItems 6
#define imgExplosion 7
#define imgBoss 8
#define imgMisc2040 9
#define imgFontKana 10
#define imgBoldFont 11
#define imgDark 12
#define imgMisc6020 13
#define imgTitle01 14

//Sound names
#define sndBee01 0
#define sndBell01 1
#define sndBom01 2
#define sndBom02 3
#define sndBom03 4
#define sndDoor00 5
#define sndFire01 6
#define sndGas01 7
#define sndGet01 8
#define sndGet02 9
#define sndHit01 10
#define sndHit02 11
#define sndHit03 12
#define sndHit04 13
#define sndHit05 14
#define sndHit06 15
#define sndHit07 16
#define sndJump01 17
#define sndJump02 18
#define sndNg 19
#define sndOk 20
#define sndPi01 21
#define sndPi02 22
#define sndPi03 23
#define sndPi04 24
#define sndPi05 25
#define sndPi06 26
#define sndPi07 27
#define sndPi08 28
#define sndPi09 29
#define sndPi10 30
#define sndPower01 31
#define sndPower02 32
#define sndShot01 33
#define sndShot02 34
#define sndShot03 35
#define sndShot04 36
#define sndShot05 37
#define sndShot06 38
#define sndShot07 39
#define sndStep01 40
#define sndWater01 41
#define sndWolf01 42

#ifdef _SDL
extern char savename[4096];
extern char savemap[4096];
#else
#define savename "data/save.tmp"
#define savemap "map/018.map"
#endif

#ifdef _KOLIBRI
#define KOS_HCL_SAVES_PATH "/tmp0/1/.hydracastlelabyrinth"
#define KOS_TMP_DIR "/tmp0/1"
#endif

void loadImages();
void freeImages();
void loadResources();
void freeResources();

void game();

void gameSetup();
void gameCleanup();

void enterDoor();
#ifdef EMSCRIPTEN
void getItemSetup(int itemNum);
int getItemEMStep();
#else
void getItem(int itemNum);
#endif
void saveScreen();

void gameEnding();

//void enterDoor(Door* d);
void loadScreen();

void changeScreen(int dx, int dy);

int writeSave(char* fname);
void loadSave(char* fname);

int fileExists(char* fpath);

void playSecret();
void secretCountdown();

int getDrawHP();
void setDrawHP(int val);

int getLevel();

void setBossRoom();

void setAutoSave(char val);
char getAutoSave();

#endif