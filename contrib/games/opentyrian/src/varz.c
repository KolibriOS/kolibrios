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
#include "varz.h"

#include "config.h"
#include "editship.h"
#include "episodes.h"
#include "joystick.h"
#include "lds_play.h"
#include "loudness.h"
#include "mainint.h"
#include "mouse.h"
#include "mtrand.h"
#include "network.h"
#include "nortsong.h"
#include "nortvars.h"
#include "opentyr.h"
#include "shots.h"
#include "sprite.h"
#include "vga256d.h"
#include "video.h"

JE_integer tempDat, tempDat2, tempDat3;

const JE_byte SANextShip[SA + 2] /* [0..SA + 1] */ = { 3, 9, 6, 2, 5, 1, 4, 3, 7 }; // 0 -> 3 -> 2 -> 6 -> 4 -> 5 -> 1 -> 9 -> 7
const JE_word SASpecialWeapon[SA] /* [1..SA] */  = { 7, 8, 9, 10, 11, 12, 13 };
const JE_word SASpecialWeaponB[SA] /* [1..SA] */ = {37, 6, 15, 40, 16, 14, 41 };
const JE_byte SAShip[SA] /* [1..SA] */ = { 3, 1, 5, 10, 2, 11, 12 };
const JE_word SAWeapon[SA][5] /* [1..SA, 1..5] */ =
{  /*  R  Bl  Bk  G   P */
	{  9, 31, 32, 33, 34 },  /* Stealth Ship */
	{ 19,  8, 22, 41, 34 },  /* StormWind    */
	{ 27,  5, 20, 42, 31 },  /* Techno       */
	{ 15,  3, 28, 22, 12 },  /* Enemy        */
	{ 23, 35, 25, 14,  6 },  /* Weird        */
	{  2,  5, 21,  4,  7 },  /* Unknown      */
	{ 40, 38, 37, 41, 36 }   /* NortShip Z   */
};

const JE_byte specialArcadeWeapon[PORT_NUM] /* [1..Portnum] */ =
{
	17,17,18,0,0,0,10,0,0,0,0,0,44,0,10,0,19,0,0,-0,0,0,0,0,0,0,
	-0,0,0,0,45,0,0,0,0,0,0,0,0,0,0,0
};

const JE_byte optionSelect[16][3][2] /* [0..15, 1..3, 1..2] */ =
{	/*  MAIN    OPT    FRONT */
	{ { 0, 0},{ 0, 0},{ 0, 0} },  /**/
	{ { 1, 1},{16,16},{30,30} },  /*Single Shot*/
	{ { 2, 2},{29,29},{29,20} },  /*Dual Shot*/
	{ { 3, 3},{21,21},{12, 0} },  /*Charge Cannon*/
	{ { 4, 4},{18,18},{16,23} },  /*Vulcan*/
	{ { 0, 0},{ 0, 0},{ 0, 0} },  /**/
	{ { 6, 6},{29,16},{ 0,22} },  /*Super Missile*/
	{ { 7, 7},{19,19},{19,28} },  /*Atom Bomb*/
	{ { 0, 0},{ 0, 0},{ 0, 0} },  /**/
	{ { 0, 0},{ 0, 0},{ 0, 0} },  /**/
	{ {10,10},{21,21},{21,27} },  /*Mini Missile*/
	{ { 0, 0},{ 0, 0},{ 0, 0} },  /**/
	{ { 0, 0},{ 0, 0},{ 0, 0} },  /**/
	{ {13,13},{17,17},{13,26} },  /*MicroBomb*/
	{ { 0, 0},{ 0, 0},{ 0, 0} },  /**/
	{ {15,15},{15,16},{15,16} }   /*Post-It*/
};

const JE_word PGR[21] /* [1..21] */ =
{
	4,
	1,2,3,
	41-21,57-21,73-21,89-21,105-21,
	121-21,137-21,153-21,
	151,151,151,151,73-21,73-21,1,2,4
	/*151,151,151*/
};
const JE_byte PAni[21] /* [1..21] */ = {1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1};

const JE_word linkGunWeapons[38] /* [1..38] */ =
{
	0,0,0,0,0,0,0,0,444,445,446,447,0,448,449,0,0,0,0,0,450,451,0,506,0,564,
	  445,446,447,448,449,445,446,447,448,449,450,451
};
const JE_word chargeGunWeapons[38] /* [1..38] */ =
{
	0,0,0,0,0,0,0,0,476,458,464,482,0,488,470,0,0,0,0,0,494,500,0,528,0,558,
	  458,458,458,458,458,458,458,458,458,458,458,458
};
const JE_byte randomEnemyLaunchSounds[3] /* [1..3] */ = {13,6,26};

/* YKS: Twiddle cheat sheet:
 * 1: UP
 * 2: DOWN
 * 3: LEFT
 * 4: RIGHT
 * 5: UP+FIRE
 * 6: DOWN+FIRE
 * 7: LEFT+FIRE
 * 8: RIGHT+FIRE
 * 9: Release all keys (directions and fire)
 */
const JE_byte keyboardCombos[26][8] /* [1..26, 1..8] */ =
{
	{ 2, 1,   2,   5, 137,           0, 0, 0}, /*Invulnerability*/
	{ 4, 3,   2,   5, 138,           0, 0, 0}, /*Atom Bomb*/
	{ 3, 4,   6, 139,             0, 0, 0, 0}, /*Seeker Bombs*/
	{ 2, 5, 142,               0, 0, 0, 0, 0}, /*Ice Blast*/
	{ 6, 2,   6, 143,             0, 0, 0, 0}, /*Auto Repair*/
	{ 6, 7,   5,   8,   6,   7,  5, 112     }, /*Spin Wave*/
	{ 7, 8, 101,               0, 0, 0, 0, 0}, /*Repulsor*/
	{ 1, 7,   6, 146,             0, 0, 0, 0}, /*Protron Field*/
	{ 8, 6,   7,   1, 120,           0, 0, 0}, /*Minefield*/
	{ 3, 6,   8,   5, 121,           0, 0, 0}, /*Post-It Blast*/
	{ 1, 2,   7,   8, 119,           0, 0, 0}, /*Drone Ship - TBC*/
	{ 3, 4,   3,   6, 123,           0, 0, 0}, /*Repair Player 2*/
	{ 6, 7,   5,   8, 124,           0, 0, 0}, /*Super Bomb - TBC*/
	{ 1, 6, 125,               0, 0, 0, 0, 0}, /*Hot Dog*/
	{ 9, 5, 126,               0, 0, 0, 0, 0}, /*Lightning UP      */
	{ 1, 7, 127,               0, 0, 0, 0, 0}, /*Lightning UP+LEFT */
	{ 1, 8, 128,               0, 0, 0, 0, 0}, /*Lightning UP+RIGHT*/
	{ 9, 7, 129,               0, 0, 0, 0, 0}, /*Lightning    LEFT */
	{ 9, 8, 130,               0, 0, 0, 0, 0}, /*Lightning    RIGHT*/
	{ 4, 2,   3,   5, 131,           0, 0, 0}, /*Warfly            */
	{ 3, 1,   2,   8, 132,           0, 0, 0}, /*FrontBlaster      */
	{ 2, 4,   5, 133,             0, 0, 0, 0}, /*Gerund            */
	{ 3, 4,   2,   8, 134,           0, 0, 0}, /*FireBomb          */
	{ 1, 4,   6, 135,             0, 0, 0, 0}, /*Indigo            */
	{ 1, 3,   6, 137,             0, 0, 0, 0}, /*Invulnerability [easier] */
	{ 1, 4,   3,   4,   7, 136,         0, 0}  /*D-Media Protron Drone    */
};

const JE_byte shipCombosB[21] /* [1..21] */ =
	{15,16,17,18,19,20,21,22,23,24, 7, 8, 5,25,14, 4, 6, 3, 9, 2,26};
  /*!! SUPER Tyrian !!*/
const JE_byte superTyrianSpecials[4] /* [1..4] */ = {1,2,4,5};

const JE_byte shipCombos[14][3] /* [0..12, 1..3] */ =
{
	{ 5, 4, 7},  /*2nd Player ship*/
	{ 1, 2, 0},  /*USP Talon*/
	{14, 4, 0},  /*Super Carrot*/
	{ 4, 5, 0},  /*Gencore Phoenix*/
	{ 6, 5, 0},  /*Gencore Maelstrom*/
	{ 7, 8, 0},  /*MicroCorp Stalker*/
	{ 7, 9, 0},  /*MicroCorp Stalker-B*/
	{10, 3, 5},  /*Prototype Stalker-C*/
	{ 5, 8, 9},  /*Stalker*/
	{ 1, 3, 0},  /*USP Fang*/
	{ 7,16,17},  /*U-Ship*/
	{ 2,11,12},  /*1st Player ship*/
	{ 3, 8,10},  /*Nort ship*/
	{ 0, 0, 0}   // Dummy entry added for Stalker 21.126
};

/*Street-Fighter Commands*/
JE_byte SFCurrentCode[2][21]; /* [1..2, 1..21] */
JE_byte SFExecuted[2]; /* [1..2] */

/*Special General Data*/
JE_byte lvlFileNum;
JE_word maxEvent, eventLoc;
/*JE_word maxenemies;*/
JE_word tempBackMove, explodeMove; /*Speed of background movement*/
JE_byte levelEnd;
JE_word levelEndFxWait;
JE_shortint levelEndWarp;
JE_boolean endLevel, reallyEndLevel, waitToEndLevel, playerEndLevel,
           normalBonusLevelCurrent, bonusLevelCurrent,
           smallEnemyAdjust, readyToEndLevel, quitRequested;

JE_byte newPL[10]; /* [0..9] */ /*Eventsys event 75 parameter*/
JE_word returnLoc;
JE_boolean returnActive;
JE_word galagaShotFreq;
JE_longint galagaLife;

JE_boolean debug = false; /*Debug Mode*/
Uint32 debugTime, lastDebugTime;
JE_longint debugHistCount;
JE_real debugHist;
JE_word curLoc; /*Current Pixel location of background 1*/

JE_boolean firstGameOver, gameLoaded, enemyStillExploding;


/* Destruction Ratio */
JE_word totalEnemy;
JE_word enemyKilled;

/* Shape/Map Data - All in one Segment! */
struct JE_MegaDataType1 megaData1;
struct JE_MegaDataType2 megaData2;
struct JE_MegaDataType3 megaData3;

/* Secret Level Display */
JE_byte flash;
JE_shortint flashChange;
JE_byte displayTime;

/* Demo Stuff */
bool play_demo = false, record_demo = false, stopped_demo = false;
Uint8 demo_num = 0;
FILE *demo_file = NULL;

Uint8 demo_keys, next_demo_keys;
Uint16 demo_keys_wait;

/* Sound Effects Queue */
JE_byte soundQueue[8]; /* [0..7] */

/*Level Event Data*/
JE_boolean enemyContinualDamage;
JE_boolean enemiesActive;
JE_boolean forceEvents;
JE_boolean stopBackgrounds;
JE_byte stopBackgroundNum;
JE_byte damageRate;  /*Rate at which a player takes damage*/
JE_boolean background3x1;  /*Background 3 enemies use Background 1 X offset*/
JE_boolean background3x1b; /*Background 3 enemies moved 8 pixels left*/

JE_boolean levelTimer;
JE_word    levelTimerCountdown;
JE_word    levelTimerJumpTo;
JE_boolean randomExplosions;

JE_boolean editShip1, editShip2;

JE_boolean globalFlags[10]; /* [1..10] */
JE_byte levelSong;

/* DESTRUCT game */
JE_boolean loadDestruct;

/* MapView Data */
JE_word mapOrigin, mapPNum;
JE_byte mapPlanet[5], mapSection[5]; /* [1..5] */

/* Interface Constants */
JE_boolean moveTyrianLogoUp;
JE_boolean skipStarShowVGA;

/*EnemyData*/
JE_MultiEnemyType enemy;
JE_EnemyAvailType enemyAvail;  /* values: 0: used, 1: free, 2: secret pick-up */
JE_word enemyOffset;
JE_word enemyOnScreen;
JE_byte enemyShapeTables[6]; /* [1..6] */
JE_word superEnemy254Jump;

/*EnemyShotData*/
JE_boolean fireButtonHeld;
JE_boolean enemyShotAvail[ENEMY_SHOT_MAX]; /* [1..Enemyshotmax] */
EnemyShotType enemyShot[ENEMY_SHOT_MAX]; /* [1..Enemyshotmax]  */

/* Player Shot Data */
JE_byte     zinglonDuration;
JE_byte     astralDuration;
JE_word     flareDuration;
JE_boolean  flareStart;
JE_shortint flareColChg;
JE_byte     specialWait;
JE_byte     nextSpecialWait;
JE_boolean  spraySpecial;
JE_byte     doIced;
JE_boolean  infiniteShot;

/*PlayerData*/
JE_boolean allPlayersGone; /*Both players dead and finished exploding*/

const uint shadowYDist = 10;

JE_real optionSatelliteRotate;

JE_integer optionAttachmentMove;
JE_boolean optionAttachmentLinked, optionAttachmentReturn;


JE_byte chargeWait, chargeLevel, chargeMax, chargeGr, chargeGrWait;

JE_word neat;


/*ExplosionData*/
explosion_type explosions[MAX_EXPLOSIONS]; /* [1..ExplosionMax] */
JE_integer explosionFollowAmountX, explosionFollowAmountY;

/*Repeating Explosions*/
rep_explosion_type rep_explosions[MAX_REPEATING_EXPLOSIONS]; /* [1..20] */

/*SuperPixels*/
superpixel_type superpixels[MAX_SUPERPIXELS]; /* [0..MaxSP] */
unsigned int last_superpixel;

/*Temporary Numbers*/
JE_byte temp, temp2, temp3;
JE_word tempX, tempY;
JE_word tempW;

JE_boolean doNotSaveBackup;

JE_word x, y;
JE_integer b;

JE_byte **BKwrap1to, **BKwrap2to, **BKwrap3to,
        **BKwrap1, **BKwrap2, **BKwrap3;

JE_shortint specialWeaponFilter, specialWeaponFreq;
JE_word     specialWeaponWpn;
JE_boolean  linkToPlayer;

JE_word shipGr, shipGr2;
Sprite2_array *shipGrPtr, *shipGr2ptr;

void JE_getShipInfo( void )
{
	JE_boolean extraShip, extraShip2;

	shipGrPtr = &shapes9;
	shipGr2ptr = &shapes9;

	powerAdd  = powerSys[player[0].items.generator].power;

	extraShip = player[0].items.ship > 90;
	if (extraShip)
	{
		JE_byte base = (player[0].items.ship - 91) * 15;
		shipGr = JE_SGr(player[0].items.ship - 90, &shipGrPtr);
		player[0].armor = extraShips[base + 7];
	}
	else
	{
		shipGr = ships[player[0].items.ship].shipgraphic;
		player[0].armor = ships[player[0].items.ship].dmg;
	}

	extraShip2 = player[1].items.ship > 90;
	if (extraShip2)
	{
		JE_byte base2 = (player[1].items.ship - 91) * 15;
		shipGr2 = JE_SGr(player[1].items.ship - 90, &shipGr2ptr);
		player[1].armor = extraShips[base2 + 7]; /* bug? */
	}
	else
	{
		shipGr2 = 0;
		player[1].armor = 10;
	}

	for (uint i = 0; i < COUNTOF(player); ++i)
	{
		player[i].initial_armor = player[i].armor;


		uint temp = ((i == 0 && extraShip) ||
		             (i == 1 && extraShip2)) ? 2 : ships[player[i].items.ship].ani;

		if (temp == 0)
		{
			player[i].shot_hit_area_x = 12;
			player[i].shot_hit_area_y = 10;
		}
		else
		{
			player[i].shot_hit_area_x = 11;
			player[i].shot_hit_area_y = 14;
		}
	}
}

JE_word JE_SGr( JE_word ship, Sprite2_array **ptr )
{
	const JE_word GR[15] /* [1..15] */ = {233, 157, 195, 271, 81, 0, 119, 5, 43, 81, 119, 157, 195, 233, 271};

	JE_word tempW = extraShips[(ship - 1) * 15];
	if (tempW > 7)
		*ptr = extraShapes;

	return GR[tempW-1];
}

void JE_drawOptions( void )
{
	SDL_Surface *temp_surface = VGAScreen;
	VGAScreen = VGAScreenSeg;

	Player *this_player = &player[twoPlayerMode ? 1 : 0];

	for (uint i = 0; i < COUNTOF(this_player->sidekick); ++i)
	{
		JE_OptionType *this_option = &options[this_player->items.sidekick[i]];

		this_player->sidekick[i].ammo =
		this_player->sidekick[i].ammo_max = this_option->ammo;

		this_player->sidekick[i].ammo_refill_ticks =
		this_player->sidekick[i].ammo_refill_ticks_max = (105 - this_player->sidekick[i].ammo) * 4;

		this_player->sidekick[i].style = this_option->tr;

		this_player->sidekick[i].animation_enabled = (this_option->option == 1);
		this_player->sidekick[i].animation_frame = 0;

		this_player->sidekick[i].charge = 0;
		this_player->sidekick[i].charge_ticks = 20;


		// draw initial sidekick HUD
		const int y = hud_sidekick_y[twoPlayerMode ? 1 : 0][i];

		fill_rectangle_xy(VGAScreenSeg, 284, y, 284 + 28, y + 15, 0);
		if (this_option->icongr > 0)
			blit_sprite(VGAScreenSeg, 284, y, OPTION_SHAPES, this_option->icongr - 1);  // sidekick HUD icon
		draw_segmented_gauge(VGAScreenSeg, 284, y + 13, 112, 2, 2, MAX(1, this_player->sidekick[i].ammo_max / 10), this_player->sidekick[i].ammo);
	}

	VGAScreen = temp_surface;

	JE_drawOptionLevel();
}

void JE_drawOptionLevel( void )
{
	if (twoPlayerMode)
	{
		for (temp = 1; temp <= 3; temp++)
		{
			fill_rectangle_xy(VGAScreenSeg, 268, 127 + (temp - 1) * 6, 269, 127 + 3 + (temp - 1) * 6, 193 + ((player[1].items.sidekick_level - 100) == temp) * 11);
		}
	}
}

void JE_tyrianHalt( JE_byte code )
{
	deinit_audio();
	deinit_video();
	deinit_joysticks();

	/* TODO: NETWORK */

	free_main_shape_tables();

	free_sprite2s(&shapes6);

	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		free(digiFx[i]);
	}

	if (code != 9)
	{
		/*
		TODO?
		JE_drawANSI("exitmsg.bin");
		JE_gotoXY(1,22);*/

		JE_saveConfiguration();
	}

	/* endkeyboard; */

	if (code == 9)
	{
		/* OutputString('call=file0002.EXE' + #0'); TODO? */
	}

	if (code == 5)
	{
		code = 0;
	}

	if (trentWin)
	{
		printf("\n"
		       "\n"
		       "\n"
		       "\n"
		       "Sleep well, Trent, you deserve the rest.\n"
		       "You now have permission to borrow my ship on your next mission.\n"
		       "\n"
		       "Also, you might want to try out the YESXMAS parameter.\n"
		       "  Type: File0001 YESXMAS\n"
		       "\n"
		       "You'll need the 2.1 patch, though!\n"
		       "\n");
	}

	SDL_Quit();
	exit(code);
}

void JE_specialComplete( JE_byte playerNum, JE_byte specialType )
{
	nextSpecialWait = 0;
	switch (special[specialType].stype)
	{
		/*Weapon*/
		case 1:
			if (playerNum == 1)
				b = player_shot_create(0, SHOT_SPECIAL2, player[0].x, player[0].y, mouseX, mouseY, special[specialType].wpn, playerNum);
			else
				b = player_shot_create(0, SHOT_SPECIAL2, player[1].x, player[1].y, mouseX, mouseY, special[specialType].wpn, playerNum);

			shotRepeat[SHOT_SPECIAL] = shotRepeat[SHOT_SPECIAL2];
			break;
		/*Repulsor*/
		case 2:
			for (temp = 0; temp < ENEMY_SHOT_MAX; temp++)
			{
				if (!enemyShotAvail[temp])
				{
					if (player[0].x > enemyShot[temp].sx)
						enemyShot[temp].sxm--;
					else if (player[0].x < enemyShot[temp].sx)
						enemyShot[temp].sxm++;

					if (player[0].y > enemyShot[temp].sy)
						enemyShot[temp].sym--;
					else if (player[0].y < enemyShot[temp].sy)
						enemyShot[temp].sym++;
				}
			}
			break;
		/*Zinglon Blast*/
		case 3:
			zinglonDuration = 50;
			shotRepeat[SHOT_SPECIAL] = 100;
			soundQueue[7] = S_SOUL_OF_ZINGLON;
			break;
		/*Attractor*/
		case 4:
			for (temp = 0; temp < 100; temp++)
			{
				if (enemyAvail[temp] != 1 && enemy[temp].scoreitem
				    && enemy[temp].evalue != 0)
				{
					if (player[0].x > enemy[temp].ex)
						enemy[temp].exc++;
					else if (player[0].x < enemy[temp].ex)
						enemy[temp].exc--;

					if (player[0].y > enemy[temp].ey)
						enemy[temp].eyc++;
					else if (player[0].y < enemy[temp].ey)
						enemy[temp].eyc--;
				}
			}
			break;
		/*Flare*/
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 16:
			if (flareDuration == 0)
				flareStart = true;

			specialWeaponWpn = special[specialType].wpn;
			linkToPlayer = false;
			spraySpecial = false;
			switch (special[specialType].stype)
			{
				case 5:
					specialWeaponFilter = 7;
					specialWeaponFreq = 2;
					flareDuration = 50;
					break;
				case 6:
					specialWeaponFilter = 1;
					specialWeaponFreq = 7;
					flareDuration = 200 + 25 * player[0].items.weapon[FRONT_WEAPON].power;
					break;
				case 7:
					specialWeaponFilter = 3;
					specialWeaponFreq = 3;
					flareDuration = 50 + 10 * player[0].items.weapon[FRONT_WEAPON].power;
					zinglonDuration = 50;
					shotRepeat[SHOT_SPECIAL] = 100;
					soundQueue[7] = S_SOUL_OF_ZINGLON;
					break;
				case 8:
					specialWeaponFilter = -99;
					specialWeaponFreq = 7;
					flareDuration = 10 + player[0].items.weapon[FRONT_WEAPON].power;
					break;
				case 9:
					specialWeaponFilter = -99;
					specialWeaponFreq = 8;
					flareDuration = 8 + 2 * player[0].items.weapon[FRONT_WEAPON].power;
					linkToPlayer = true;
					nextSpecialWait = special[specialType].pwr;
					break;
				case 10:
					specialWeaponFilter = -99;
					specialWeaponFreq = 8;
					flareDuration = 14 + 4 * player[0].items.weapon[FRONT_WEAPON].power;
					linkToPlayer = true;
					break;
				case 11:
					specialWeaponFilter = -99;
					specialWeaponFreq = special[specialType].pwr;
					flareDuration = 10 + 10 * player[0].items.weapon[FRONT_WEAPON].power;
					astralDuration = 20 + 10 * player[0].items.weapon[FRONT_WEAPON].power;
					break;
				case 16:
					specialWeaponFilter = -99;
					specialWeaponFreq = 8;
					flareDuration = temp2 * 16 + 8;
					linkToPlayer = true;
					spraySpecial = true;
					break;
			}
			break;
		case 12:
			player[playerNum-1].invulnerable_ticks = temp2 * 10;

			if (superArcadeMode > 0 && superArcadeMode <= SA)
			{
				shotRepeat[SHOT_SPECIAL] = 250;
				b = player_shot_create(0, SHOT_SPECIAL2, player[0].x, player[0].y, mouseX, mouseY, 707, 1);
				player[0].invulnerable_ticks = 100;
			}
			break;
		case 13:
			player[0].armor += temp2 / 4 + 1;

			soundQueue[3] = S_POWERUP;
			break;
		case 14:
			player[1].armor += temp2 / 4 + 1;

			soundQueue[3] = S_POWERUP;
			break;

		case 17:  // spawn left or right sidekick
			soundQueue[3] = S_POWERUP;

			if (player[0].items.sidekick[LEFT_SIDEKICK] == special[specialType].wpn)
			{
				player[0].items.sidekick[RIGHT_SIDEKICK] = special[specialType].wpn;
				shotMultiPos[RIGHT_SIDEKICK] = 0;
			}
			else
			{
				player[0].items.sidekick[LEFT_SIDEKICK] = special[specialType].wpn;
				shotMultiPos[LEFT_SIDEKICK] = 0;
			}

			JE_drawOptions();
			break;

		case 18:  // spawn right sidekick
			player[0].items.sidekick[RIGHT_SIDEKICK] = special[specialType].wpn;

			JE_drawOptions();

			soundQueue[4] = S_POWERUP;

			shotMultiPos[RIGHT_SIDEKICK] = 0;
			break;
	}
}

void JE_doSpecialShot( JE_byte playerNum, uint *armor, uint *shield )
{
	if (player[0].items.special > 0)
	{
		if (shotRepeat[SHOT_SPECIAL] == 0 && specialWait == 0 && flareDuration < 2 && zinglonDuration < 2)
			blit_sprite2(VGAScreen, 47, 4, shapes9, 94);
		else
			blit_sprite2(VGAScreen, 47, 4, shapes9, 93);
	}

	if (shotRepeat[SHOT_SPECIAL] > 0)
	{
		--shotRepeat[SHOT_SPECIAL];
	}
	if (specialWait > 0)
	{
		specialWait--;
	}
	temp = SFExecuted[playerNum-1];
	if (temp > 0 && shotRepeat[SHOT_SPECIAL] == 0 && flareDuration == 0)
	{
		temp2 = special[temp].pwr;

		bool can_afford = true;

		if (temp2 > 0)
		{
			if (temp2 < 98)  // costs some shield
			{
				if (*shield >= temp2)
					*shield -= temp2;
				else
					can_afford = false;
			}
			else if (temp2 == 98)  // costs all shield
			{
				if (*shield < 4)
					can_afford = false;
				temp2 = *shield;
				*shield = 0;
			}
			else if (temp2 == 99)  // costs half shield
			{
				temp2 = *shield / 2;
				*shield = temp2;
			}
			else  // costs some armor
			{
				temp2 -= 100;
				if (*armor > temp2)
					*armor -= temp2;
				else
					can_afford = false;
			}
		}

		shotMultiPos[SHOT_SPECIAL] = 0;
		shotMultiPos[SHOT_SPECIAL2] = 0;

		if (can_afford)
			JE_specialComplete(playerNum, temp);

		SFExecuted[playerNum-1] = 0;

		JE_wipeShieldArmorBars();
		VGAScreen = VGAScreenSeg; /* side-effect of game_screen */
		JE_drawShield();
		JE_drawArmor();
		VGAScreen = game_screen; /* side-effect of game_screen */
	}

	if (playerNum == 1 && player[0].items.special > 0)
	{  /*Main Begin*/

		if (superArcadeMode > 0 && (button[2-1] || button[3-1]))
		{
			fireButtonHeld = false;
		}
		if (!button[1-1] && !(superArcadeMode != SA_NONE && (button[2-1] || button[3-1])))
		{
			fireButtonHeld = false;
		}
		else if (shotRepeat[SHOT_SPECIAL] == 0 && !fireButtonHeld && !(flareDuration > 0) && specialWait == 0)
		{
			fireButtonHeld = true;
			JE_specialComplete(playerNum, player[0].items.special);
		}

	}  /*Main End*/

	if (astralDuration > 0)
		astralDuration--;

	shotAvail[MAX_PWEAPON-1] = 0;
	if (flareDuration > 1)
	{
		if (specialWeaponFilter != -99)
		{
			if (levelFilter == -99 && levelBrightness == -99)
			{
				filterActive = false;
			}
			if (!filterActive)
			{
				levelFilter = specialWeaponFilter;
				if (levelFilter == 7)
				{
					levelBrightness = 0;
				}
				filterActive = true;
			}

			if (mt_rand() % 2 == 0)
				flareColChg = -1;
			else
				flareColChg = 1;

			if (levelFilter == 7)
			{
				if (levelBrightness < -6)
				{
					flareColChg = 1;
				}
				if (levelBrightness > 6)
				{
					flareColChg = -1;
				}
				levelBrightness += flareColChg;
			}
		}

		if ((signed)(mt_rand() % 6) < specialWeaponFreq)
		{
			b = MAX_PWEAPON;

			if (linkToPlayer)
			{
				if (shotRepeat[SHOT_SPECIAL] == 0)
				{
					b = player_shot_create(0, SHOT_SPECIAL, player[0].x, player[0].y, mouseX, mouseY, specialWeaponWpn, playerNum);
				}
			}
			else
			{
				b = player_shot_create(0, SHOT_SPECIAL, mt_rand() % 280, mt_rand() % 180, mouseX, mouseY, specialWeaponWpn, playerNum);
			}

			if (spraySpecial && b != MAX_PWEAPON)
			{
				playerShotData[b].shotXM = (mt_rand() % 5) - 2;
				playerShotData[b].shotYM = (mt_rand() % 5) - 2;
				if (playerShotData[b].shotYM == 0)
				{
					playerShotData[b].shotYM++;
				}
			}
		}

		flareDuration--;
		if (flareDuration == 1)
		{
			specialWait = nextSpecialWait;
		}
	}
	else if (flareStart)
	{
		flareStart = false;
		shotRepeat[SHOT_SPECIAL] = linkToPlayer ? 15 : 200;
		flareDuration = 0;
		if (levelFilter == specialWeaponFilter)
		{
			levelFilter = -99;
			levelBrightness = -99;
			filterActive = false;
		}
	}

	if (zinglonDuration > 1)
	{
		temp = 25 - abs(zinglonDuration - 25);

		JE_barBright(VGAScreen, player[0].x + 7 - temp,     0, player[0].x + 7 + temp,     184);
		JE_barBright(VGAScreen, player[0].x + 7 - temp - 2, 0, player[0].x + 7 + temp + 2, 184);

		zinglonDuration--;
		if (zinglonDuration % 5 == 0)
		{
			shotAvail[MAX_PWEAPON-1] = 1;
		}
	}
}

void JE_setupExplosion( signed int x, signed int y, signed int delta_y, unsigned int type, bool fixed_position, bool follow_player )
{
	const struct {
		JE_word sprite;
		JE_byte ttl;
	} explosion_data[53] /* [1..53] */ = {
		{ 144,  7 },
		{ 120, 12 },
		{ 190, 12 },
		{ 209, 12 },
		{ 152, 12 },
		{ 171, 12 },
		{ 133,  7 },   /*White Smoke*/
		{   1, 12 },
		{  20, 12 },
		{  39, 12 },
		{  58, 12 },
		{ 110,  3 },
		{  76,  7 },
		{  91,  3 },
/*15*/	{ 227,  3 },
		{ 230,  3 },
		{ 233,  3 },
		{ 252,  3 },
		{ 246,  3 },
/*20*/	{ 249,  3 },
		{ 265,  3 },
		{ 268,  3 },
		{ 271,  3 },
		{ 236,  3 },
/*25*/	{ 239,  3 },
		{ 242,  3 },
		{ 261,  3 },
		{ 274,  3 },
		{ 277,  3 },
/*30*/	{ 280,  3 },
		{ 299,  3 },
		{ 284,  3 },
		{ 287,  3 },
		{ 290,  3 },
/*35*/	{ 293,  3 },
		{ 165,  8 },   /*Coin Values*/
		{ 184,  8 },
		{ 203,  8 },
		{ 222,  8 },
		{ 168,  8 },
		{ 187,  8 },
		{ 206,  8 },
		{ 225, 10 },
		{ 169, 10 },
		{ 188, 10 },
		{ 207, 20 },
		{ 226, 14 },
		{ 170, 14 },
		{ 189, 14 },
		{ 208, 14 },
		{ 246, 14 },
		{ 227, 14 },
		{ 265, 14 }
	};

	if (y > -16 && y < 190)
	{
		for (int i = 0; i < MAX_EXPLOSIONS; i++)
		{
			if (explosions[i].ttl == 0)
			{
				explosions[i].x = x;
				explosions[i].y = y;
				if (type == 6)
				{
					explosions[i].y += 12;
					explosions[i].x += 2;
				} else if (type == 98)
				{
					type = 6;
				}
				explosions[i].sprite = explosion_data[type].sprite;
				explosions[i].ttl = explosion_data[type].ttl;
				explosions[i].follow_player = follow_player;
				explosions[i].fixed_position = fixed_position;
				explosions[i].delta_x = 0;
				explosions[i].delta_y = delta_y;
				break;
			}
		}
	}
}

void JE_setupExplosionLarge( JE_boolean enemyGround, JE_byte exploNum, JE_integer x, JE_integer y )
{
	if (y >= 0)
	{
		if (enemyGround)
		{
			JE_setupExplosion(x - 6, y - 14, 0,  2, false, false);
			JE_setupExplosion(x + 6, y - 14, 0,  4, false, false);
			JE_setupExplosion(x - 6, y,      0,  3, false, false);
			JE_setupExplosion(x + 6, y,      0,  5, false, false);
		} else {
			JE_setupExplosion(x - 6, y - 14, 0,  7, false, false);
			JE_setupExplosion(x + 6, y - 14, 0,  9, false, false);
			JE_setupExplosion(x - 6, y,      0,  8, false, false);
			JE_setupExplosion(x + 6, y,      0, 10, false, false);
		}

		bool big;

		if (exploNum > 10)
		{
			exploNum -= 10;
			big = true;
		}
		else
		{
			big = false;
		}

		if (exploNum)
		{
			for (int i = 0; i < MAX_REPEATING_EXPLOSIONS; i++)
			{
				if (rep_explosions[i].ttl == 0)
				{
					rep_explosions[i].ttl = exploNum;
					rep_explosions[i].delay = 2;
					rep_explosions[i].x = x;
					rep_explosions[i].y = y;
					rep_explosions[i].big = big;
					break;
				}
			}
		}
	}
}

void JE_wipeShieldArmorBars( void )
{
	if (!twoPlayerMode || galagaMode)
	{
		fill_rectangle_xy(VGAScreenSeg, 270, 137, 278, 194 - player[0].shield * 2, 0);
	}
	else
	{
		fill_rectangle_xy(VGAScreenSeg, 270, 60 - 44, 278, 60, 0);
		fill_rectangle_xy(VGAScreenSeg, 270, 194 - 44, 278, 194, 0);
	}
	if (!twoPlayerMode || galagaMode)
	{
		fill_rectangle_xy(VGAScreenSeg, 307, 137, 315, 194 - player[0].armor * 2, 0);
	}
	else
	{
		fill_rectangle_xy(VGAScreenSeg, 307, 60 - 44, 315, 60, 0);
		fill_rectangle_xy(VGAScreenSeg, 307, 194 - 44, 315, 194, 0);
	}
}

JE_byte JE_playerDamage( JE_byte temp,
                         Player *this_player )
{
	int playerDamage = 0;
	soundQueue[7] = S_SHIELD_HIT;

	/* Player Damage Routines */
	if (this_player->shield < temp)
	{
		playerDamage = temp;
		temp -= this_player->shield;
		this_player->shield = 0;

		if (temp > 0)
		{
			/*Through Shields - Now Armor */

			if (this_player->armor < temp)
			{
				temp -= this_player->armor;
				this_player->armor = 0;

				if (this_player->is_alive && !youAreCheating)
				{
					levelTimer = false;
					this_player->is_alive = false;
					this_player->exploding_ticks = 60;
					levelEnd = 40;
					tempVolume = tyrMusicVolume;
					soundQueue[1] = S_EXPLOSION_22;
				}
			}
			else
			{
				this_player->armor -= temp;
				soundQueue[7] = S_HULL_HIT;
			}
		}
	}
	else
	{
		this_player->shield -= temp;

		JE_setupExplosion(this_player->x - 17, this_player->y - 12, 0, 14, false, !twoPlayerMode);
		JE_setupExplosion(this_player->x - 5 , this_player->y - 12, 0, 15, false, !twoPlayerMode);
		JE_setupExplosion(this_player->x + 7 , this_player->y - 12, 0, 16, false, !twoPlayerMode);
		JE_setupExplosion(this_player->x + 19, this_player->y - 12, 0, 17, false, !twoPlayerMode);

		JE_setupExplosion(this_player->x - 17, this_player->y + 2, 0,  18, false, !twoPlayerMode);
		JE_setupExplosion(this_player->x + 19, this_player->y + 2, 0,  19, false, !twoPlayerMode);

		JE_setupExplosion(this_player->x - 17, this_player->y + 16, 0, 20, false, !twoPlayerMode);
		JE_setupExplosion(this_player->x - 5 , this_player->y + 16, 0, 21, false, !twoPlayerMode);
		JE_setupExplosion(this_player->x + 7 , this_player->y + 16, 0, 22, false, !twoPlayerMode);
	}

	JE_wipeShieldArmorBars();
	VGAScreen = VGAScreenSeg; /* side-effect of game_screen */
	JE_drawShield();
	JE_drawArmor();
	VGAScreen = game_screen; /* side-effect of game_screen */

	return playerDamage;
}

JE_word JE_portConfigs( void )
{
	const uint player_index = twoPlayerMode ? 1 : 0;
	return tempW = weaponPort[player[player_index].items.weapon[REAR_WEAPON].id].opnum;
}

void JE_drawShield( void )
{
	if (twoPlayerMode && !galagaMode)
	{
		for (uint i = 0; i < COUNTOF(player); ++i)
			JE_dBar3(VGAScreen, 270, 60 + 134 * i, roundf(player[i].shield * 0.8f), 144);
	}
	else
	{
		JE_dBar3(VGAScreen, 270, 194, player[0].shield, 144);
		if (player[0].shield != player[0].shield_max)
		{
			const uint y = 193 - (player[0].shield_max * 2);
			JE_rectangle(VGAScreen, 270, y, 278, y, 68); /* <MXD> SEGa000 */
		}
	}
}

void JE_drawArmor( void )
{
	for (uint i = 0; i < COUNTOF(player); ++i)
		if (player[i].armor > 28)
			player[i].armor = 28;

	if (twoPlayerMode && !galagaMode)
	{
		for (uint i = 0; i < COUNTOF(player); ++i)
			JE_dBar3(VGAScreen, 307, 60 + 134 * i, roundf(player[i].armor * 0.8f), 224);
	}
	else
	{
		JE_dBar3(VGAScreen, 307, 194, player[0].armor, 224);
	}
}

void JE_doSP( JE_word x, JE_word y, JE_word num, JE_byte explowidth, JE_byte color ) /* superpixels */
{
	for (temp = 0; temp < num; temp++)
	{
		JE_real tempr = mt_rand_lt1() * (2 * M_PI);
		signed int tempy = roundf(cosf(tempr) * mt_rand_1() * explowidth);
		signed int tempx = roundf(sinf(tempr) * mt_rand_1() * explowidth);

		if (++last_superpixel >= MAX_SUPERPIXELS)
			last_superpixel = 0;
		superpixels[last_superpixel].x = tempx + x;
		superpixels[last_superpixel].y = tempy + y;
		superpixels[last_superpixel].delta_x = tempx;
		superpixels[last_superpixel].delta_y = tempy + 1;
		superpixels[last_superpixel].color = color;
		superpixels[last_superpixel].z = 15;
	}
}

void JE_drawSP( void )
{
	for (int i = MAX_SUPERPIXELS; i--; )
	{
		if (superpixels[i].z)
		{
			superpixels[i].x += superpixels[i].delta_x;
			superpixels[i].y += superpixels[i].delta_y;

			if (superpixels[i].x < (unsigned)VGAScreen->w && superpixels[i].y < (unsigned)VGAScreen->h)
			{
				Uint8 *s = (Uint8 *)VGAScreen->pixels; /* screen pointer, 8-bit specific */
				s += superpixels[i].y * VGAScreen->pitch;
				s += superpixels[i].x;

				*s = (((*s & 0x0f) + superpixels[i].z) >> 1) + superpixels[i].color;
				if (superpixels[i].x > 0)
					*(s - 1) = (((*(s - 1) & 0x0f) + (superpixels[i].z >> 1)) >> 1) + superpixels[i].color;
				if (superpixels[i].x < VGAScreen->w - 1u)
					*(s + 1) = (((*(s + 1) & 0x0f) + (superpixels[i].z >> 1)) >> 1) + superpixels[i].color;
				if (superpixels[i].y > 0)
					*(s - VGAScreen->pitch) = (((*(s - VGAScreen->pitch) & 0x0f) + (superpixels[i].z >> 1)) >> 1) + superpixels[i].color;
				if (superpixels[i].y < VGAScreen->h - 1u)
					*(s + VGAScreen->pitch) = (((*(s + VGAScreen->pitch) & 0x0f) + (superpixels[i].z >> 1)) >> 1) + superpixels[i].color;
			}

			superpixels[i].z--;
		}
	}
}

