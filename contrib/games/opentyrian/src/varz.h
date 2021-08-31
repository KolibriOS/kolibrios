/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef VARZ_H
#define VARZ_H

#include "episodes.h"
#include "opentyr.h"
#include "player.h"
#include "sprite.h"

#include <stdbool.h>

#define SA 7

enum
{
	SA_NONE = 0,
	SA_NORTSHIPZ = 7,
	
	// only used for code entry
	SA_DESTRUCT = 8,
	SA_ENGAGE = 9,
	
	// only used in pItems[P_SUPERARCADE]
	SA_SUPERTYRIAN = 254,
	SA_ARCADE = 255
};

#define ENEMY_SHOT_MAX  60 /* 60*/

#define CURRENT_KEY_SPEED 1  /*Keyboard/Joystick movement rate*/

#define MAX_EXPLOSIONS           200
#define MAX_REPEATING_EXPLOSIONS 20
#define MAX_SUPERPIXELS          101

struct JE_SingleEnemyType
{
	JE_byte     fillbyte;
	JE_integer  ex, ey;     /* POSITION */
	JE_shortint exc, eyc;   /* CURRENT SPEED */
	JE_shortint exca, eyca; /* RANDOM ACCELERATION */
	JE_shortint excc, eycc; /* FIXED ACCELERATION WAITTIME */
	JE_shortint exccw, eyccw;
	JE_byte     armorleft;
	JE_byte     eshotwait[3], eshotmultipos[3]; /* [1..3] */
	JE_byte     enemycycle;
	JE_byte     ani;
	JE_word     egr[20]; /* [1..20] */
	JE_byte     size;
	JE_byte     linknum;
	JE_byte     aniactive;
	JE_byte     animax;
	JE_byte     aniwhenfire;
	Sprite2_array *sprite2s;
	JE_shortint exrev, eyrev;
	JE_integer  exccadd, eyccadd;
	JE_byte     exccwmax, eyccwmax;
	void       *enemydatofs;
	JE_boolean  edamaged;
	JE_word     enemytype;
	JE_byte     animin;
	JE_word     edgr;
	JE_shortint edlevel;
	JE_shortint edani;
	JE_byte     fill1;
	JE_byte     filter;
	JE_integer  evalue;
	JE_integer  fixedmovey;
	JE_byte     freq[3]; /* [1..3] */
	JE_byte     launchwait;
	JE_word     launchtype;
	JE_byte     launchfreq;
	JE_byte     xaccel;
	JE_byte     yaccel;
	JE_byte     tur[3]; /* [1..3] */
	JE_word     enemydie; /* Enemy created when this one dies */
	JE_boolean  enemyground;
	JE_byte     explonum;
	JE_word     mapoffset;
	JE_boolean  scoreitem;

	JE_boolean  special;
	JE_byte     flagnum;
	JE_boolean  setto;

	JE_byte     iced; /*Duration*/

	JE_byte     launchspecial;

	JE_integer  xminbounce;
	JE_integer  xmaxbounce;
	JE_integer  yminbounce;
	JE_integer  ymaxbounce;
	JE_byte     fill[3]; /* [1..3] */
};

typedef struct JE_SingleEnemyType JE_MultiEnemyType[100]; /* [1..100] */

typedef JE_word JE_DanCShape[(24 * 28) / 2]; /* [1..(24*28) div 2] */

typedef JE_char JE_CharString[256]; /* [1..256] */

typedef JE_byte JE_Map1Buffer[24 * 28 * 13 * 4]; /* [1..24*28*13*4] */

typedef JE_byte *JE_MapType[300][14]; /* [1..300, 1..14] */
typedef JE_byte *JE_MapType2[600][14]; /* [1..600, 1..14] */
typedef JE_byte *JE_MapType3[600][15]; /* [1..600, 1..15] */

struct JE_EventRecType
{
	JE_word     eventtime;
	JE_byte     eventtype;
	JE_integer  eventdat, eventdat2;
	JE_shortint eventdat3, eventdat5, eventdat6;
	JE_byte     eventdat4;
};

struct JE_MegaDataType1
{
	JE_MapType mainmap;
	struct
	{
		JE_DanCShape sh;
	} shapes[72]; /* [0..71] */
	JE_byte tempdat1;
	/*JE_DanCShape filler;*/
};

struct JE_MegaDataType2
{
	JE_MapType2 mainmap;
	struct
	{
		JE_byte nothing[3]; /* [1..3] */
		JE_byte fill;
		JE_DanCShape sh;
	} shapes[71]; /* [0..70] */
	JE_byte tempdat2;
};

struct JE_MegaDataType3
{
	JE_MapType3 mainmap;
	struct
	{
		JE_byte nothing[3]; /* [1..3] */
		JE_byte fill;
		JE_DanCShape sh;
	} shapes[70]; /* [0..69] */
	JE_byte tempdat3;
};

typedef JE_byte JE_EnemyAvailType[100]; /* [1..100] */

typedef struct {
	JE_integer sx, sy;
	JE_integer sxm, sym;
	JE_shortint sxc, syc;
	JE_byte tx, ty;
	JE_word sgr;
	JE_byte sdmg;
	JE_byte duration;
	JE_word animate;
	JE_word animax;
	JE_byte fill[12];
} EnemyShotType;

typedef struct {
	unsigned int ttl;
	signed int x, y;
	signed int delta_x, delta_y;
	bool fixed_position;
	bool follow_player;
	unsigned int sprite;
} explosion_type;

typedef struct {
	unsigned int delay;
	unsigned int ttl;
	unsigned int x, y;
	bool big;
} rep_explosion_type;

typedef struct {
	unsigned int x, y, z;
	signed int delta_x, delta_y;
	Uint8 color;
} superpixel_type;

extern JE_integer tempDat, tempDat2, tempDat3;
extern const JE_byte SANextShip[SA + 2];
extern const JE_word SASpecialWeapon[SA];
extern const JE_word SASpecialWeaponB[SA];
extern const JE_byte SAShip[SA];
extern const JE_word SAWeapon[SA][5];
extern const JE_byte specialArcadeWeapon[PORT_NUM];
extern const JE_byte optionSelect[16][3][2];
extern const JE_word PGR[21];
extern const JE_byte PAni[21];
extern const JE_word linkGunWeapons[38];
extern const JE_word chargeGunWeapons[38];
extern const JE_byte randomEnemyLaunchSounds[3];
extern const JE_byte keyboardCombos[26][8];
extern const JE_byte shipCombosB[21];
extern const JE_byte superTyrianSpecials[4];
extern const JE_byte shipCombos[14][3];
extern JE_byte SFCurrentCode[2][21];
extern JE_byte SFExecuted[2];
extern JE_byte lvlFileNum;
extern JE_word maxEvent, eventLoc;
extern JE_word tempBackMove, explodeMove;
extern JE_byte levelEnd;
extern JE_word levelEndFxWait;
extern JE_shortint levelEndWarp;
extern JE_boolean endLevel, reallyEndLevel, waitToEndLevel, playerEndLevel, normalBonusLevelCurrent, bonusLevelCurrent, smallEnemyAdjust, readyToEndLevel, quitRequested;
extern JE_byte newPL[10];
extern JE_word returnLoc;
extern JE_boolean returnActive;
extern JE_word galagaShotFreq;
extern JE_longint galagaLife;
extern JE_boolean debug;
extern Uint32 debugTime, lastDebugTime;
extern JE_longint debugHistCount;
extern JE_real debugHist;
extern JE_word curLoc;
extern JE_boolean firstGameOver, gameLoaded, enemyStillExploding;
extern JE_word totalEnemy;
extern JE_word enemyKilled;
extern struct JE_MegaDataType1 megaData1;
extern struct JE_MegaDataType2 megaData2;
extern struct JE_MegaDataType3 megaData3;
extern JE_byte flash;
extern JE_shortint flashChange;
extern JE_byte displayTime;

extern bool play_demo, record_demo, stopped_demo;
extern Uint8 demo_num;
extern FILE *demo_file;

extern Uint8 demo_keys, next_demo_keys;
extern Uint16 demo_keys_wait;

extern JE_byte soundQueue[8];
extern JE_boolean enemyContinualDamage;
extern JE_boolean enemiesActive;
extern JE_boolean forceEvents;
extern JE_boolean stopBackgrounds;
extern JE_byte stopBackgroundNum;
extern JE_byte damageRate;
extern JE_boolean background3x1;
extern JE_boolean background3x1b;
extern JE_boolean levelTimer;
extern JE_word levelTimerCountdown;
extern JE_word levelTimerJumpTo;
extern JE_boolean randomExplosions;
extern JE_boolean editShip1, editShip2;
extern JE_boolean globalFlags[10];
extern JE_byte levelSong;
extern JE_boolean loadDestruct;
extern JE_word mapOrigin, mapPNum;
extern JE_byte mapPlanet[5], mapSection[5];
extern JE_boolean moveTyrianLogoUp;
extern JE_boolean skipStarShowVGA;
extern JE_MultiEnemyType enemy;
extern JE_EnemyAvailType enemyAvail;
extern JE_word enemyOffset;
extern JE_word enemyOnScreen;
extern JE_byte enemyShapeTables[6];
extern JE_word superEnemy254Jump;
extern explosion_type explosions[MAX_EXPLOSIONS];
extern JE_integer explosionFollowAmountX, explosionFollowAmountY;
extern JE_boolean fireButtonHeld;
extern JE_boolean enemyShotAvail[ENEMY_SHOT_MAX];
extern EnemyShotType enemyShot[ENEMY_SHOT_MAX];
extern JE_byte zinglonDuration;
extern JE_byte astralDuration;
extern JE_word flareDuration;
extern JE_boolean flareStart;
extern JE_shortint flareColChg;
extern JE_byte specialWait;
extern JE_byte nextSpecialWait;
extern JE_boolean spraySpecial;
extern JE_byte doIced;
extern JE_boolean infiniteShot;
extern JE_boolean allPlayersGone;
extern const uint shadowYDist;
extern JE_real optionSatelliteRotate;
extern JE_integer optionAttachmentMove;
extern JE_boolean optionAttachmentLinked, optionAttachmentReturn;
extern JE_byte chargeWait, chargeLevel, chargeMax, chargeGr, chargeGrWait;
extern JE_word neat;
extern rep_explosion_type rep_explosions[MAX_REPEATING_EXPLOSIONS];
extern superpixel_type superpixels[MAX_SUPERPIXELS];
extern unsigned int last_superpixel;
extern JE_byte temp, temp2, temp3;
extern JE_word tempX, tempY;
extern JE_word tempW;
extern JE_boolean doNotSaveBackup;
extern JE_word x, y;
extern JE_integer b;
extern JE_byte **BKwrap1to, **BKwrap2to, **BKwrap3to, **BKwrap1, **BKwrap2, **BKwrap3;
extern JE_shortint specialWeaponFilter, specialWeaponFreq;
extern JE_word specialWeaponWpn;
extern JE_boolean linkToPlayer;
extern JE_word shipGr, shipGr2;
extern Sprite2_array *shipGrPtr, *shipGr2ptr;

static const int hud_sidekick_y[2][2] =
{
	{  64,  82 }, // one player HUD
	{ 108, 126 }, // two player HUD
};

void JE_getShipInfo( void );
JE_word JE_SGr( JE_word ship, Sprite2_array **ptr );

void JE_drawOptions( void );

void JE_tyrianHalt( JE_byte code ); /* This ends the game */
void JE_specialComplete( JE_byte playernum, JE_byte specialType );
void JE_doSpecialShot( JE_byte playernum, uint *armor, uint *shield );

void JE_wipeShieldArmorBars( void );
JE_byte JE_playerDamage( JE_byte temp, Player * );

void JE_setupExplosion( signed int x, signed int y, signed int delta_y, unsigned int type, bool fixed_position, bool follow_player );
void JE_setupExplosionLarge( JE_boolean enemyground, JE_byte explonum, JE_integer x, JE_integer y );

void JE_drawShield( void );
void JE_drawArmor( void );

JE_word JE_portConfigs( void );

/*SuperPixels*/
void JE_doSP( JE_word x, JE_word y, JE_word num, JE_byte explowidth, JE_byte color );
void JE_drawSP( void );

void JE_drawOptionLevel( void );


#endif /* VARZ_H */

