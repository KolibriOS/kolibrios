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
#include "config.h"

#include "episodes.h"
#include "file.h"
#include "joystick.h"
#include "loudness.h"
#include "mtrand.h"
#include "nortsong.h"
#include "opentyr.h"
#include "player.h"
#include "varz.h"
#include "vga256d.h"
#include "video.h"
#include "video_scale.h"

#include <stdio.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include <direct.h>
#define mkdir _mkdir
#else
#include <unistd.h>
#endif

/* Configuration Load/Save handler */

const JE_byte cryptKey[10] = /* [1..10] */
{
	15, 50, 89, 240, 147, 34, 86, 9, 32, 208
};

const JE_KeySettingType defaultKeySettings =
{
	SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_SPACE, SDLK_RETURN, SDLK_LCTRL, SDLK_LALT
/*	72, 80, 75, 77, 57, 28, 29, 56*/
};

const char defaultHighScoreNames[34][23] = /* [1..34] of string [22] */
{/*1P*/
/*TYR*/   "The Prime Chair", /*13*/
          "Transon Lohk",
          "Javi Onukala",
          "Mantori",
          "Nortaneous",
          "Dougan",
          "Reid",
          "General Zinglon",
          "Late Gyges Phildren",
          "Vykromod",
          "Beppo",
          "Borogar",
          "ShipMaster Carlos",

/*OTHER*/ "Jill", /*5*/
          "Darcy",
          "Jake Stone",
          "Malvineous Havershim",
          "Marta Louise Velasquez",

/*JAZZ*/  "Jazz Jackrabbit", /*3*/
          "Eva Earlong",
          "Devan Shell",

/*OMF*/   "Crystal Devroe", /*11*/
          "Steffan Tommas",
          "Milano Angston",
          "Christian",
          "Shirro",
          "Jean-Paul",
          "Ibrahim Hothe",
          "Angel",
          "Cossette Akira",
          "Raven",
          "Hans Kreissack",

/*DARE*/  "Tyler", /*2*/
          "Rennis the Rat Guard"
};

const char defaultTeamNames[22][25] = /* [1..22] of string [24] */
{
	"Jackrabbits",
	"Team Tyrian",
	"The Elam Brothers",
	"Dare to Dream Team",
	"Pinball Freaks",
	"Extreme Pinball Freaks",
	"Team Vykromod",
	"Epic All-Stars",
	"Hans Keissack's WARriors",
	"Team Overkill",
	"Pied Pipers",
	"Gencore Growlers",
	"Microsol Masters",
	"Beta Warriors",
	"Team Loco",
	"The Shellians",
	"Jungle Jills",
	"Murderous Malvineous",
	"The Traffic Department",
	"Clan Mikal",
	"Clan Patrok",
	"Carlos' Crawlers"
};


const JE_EditorItemAvailType initialItemAvail =
{
	1,1,1,0,0,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0, /* Front/Rear Weapons 1-38  */
	0,0,0,0,0,0,0,0,0,0,1,                                                           /* Fill                     */
	1,0,0,0,0,1,0,0,0,1,1,0,1,0,0,0,0,0,                                             /* Sidekicks          51-68 */
	0,0,0,0,0,0,0,0,0,0,0,                                                           /* Fill                     */
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                                   /* Special Weapons    81-93 */
	0,0,0,0,0                                                                        /* Fill                     */
};

/* Last 2 bytes = Word
 *
 * Max Value = 1680
 * X div  60 = Armor  (1-28)
 * X div 168 = Shield (1-12)
 * X div 280 = Engine (1-06)
 */


JE_boolean smoothies[9] = /* [1..9] */
{ 0, 0, 0, 0, 0, 0, 0, 0, 0 };

JE_byte starShowVGASpecialCode;

/* CubeData */
JE_word lastCubeMax, cubeMax;
JE_word cubeList[4]; /* [1..4] */

/* High-Score Stuff */
JE_boolean gameHasRepeated;  // can only get highscore on first play-through

/* Difficulty */
JE_shortint difficultyLevel, oldDifficultyLevel,
            initialDifficulty;  // can only get highscore on initial episode

/* Player Stuff */
uint    power, lastPower, powerAdd;
JE_byte shieldWait, shieldT;

JE_byte          shotRepeat[11], shotMultiPos[11];
JE_boolean       portConfigChange, portConfigDone;

/* Level Data */
char    lastLevelName[11], levelName[11]; /* string [10] */
JE_byte mainLevel, nextLevel, saveLevel;   /*Current Level #*/

/* Keyboard Junk */
JE_KeySettingType keySettings;

/* Configuration */
JE_shortint levelFilter, levelFilterNew, levelBrightness, levelBrightnessChg;
JE_boolean  filtrationAvail, filterActive, filterFade, filterFadeStart;

JE_boolean gameJustLoaded;

JE_boolean galagaMode;

JE_boolean extraGame;

JE_boolean twoPlayerMode, twoPlayerLinked, onePlayerAction, superTyrian;
JE_boolean trentWin = false;
JE_byte    superArcadeMode;

JE_byte    superArcadePowerUp;

JE_real linkGunDirec;
JE_byte inputDevice[2] = { 1, 2 }; // 0:any  1:keyboard  2:mouse  3+:joystick

JE_byte secretHint;
JE_byte background3over;
JE_byte background2over;
JE_byte gammaCorrection;
JE_boolean superPause = false;
JE_boolean explosionTransparent,
           youAreCheating,
           displayScore,
           background2, smoothScroll, wild, superWild, starActive,
           topEnemyOver,
           skyEnemyOverAll,
           background2notTransparent;

JE_byte soundEffects; // dummy value for config
JE_byte versionNum;   /* SW 1.0 and SW/Reg 1.1 = 0 or 1
                       * EA 1.2 = 2 */

JE_byte    fastPlay;
JE_boolean pentiumMode;

/* Savegame files */
JE_byte    gameSpeed;
JE_byte    processorType;  /* 1=386 2=486 3=Pentium Hyper */

JE_SaveFilesType saveFiles; /*array[1..saveLevelnum] of savefiletype;*/
JE_SaveGameTemp saveTemp;

JE_word editorLevel;   /*Initial value 800*/

Config opentyrian_config;  // implicitly initialized

bool load_opentyrian_config( void )
{
	// defaults
	fullscreen_enabled = false;
	set_scaler_by_name("Scale2x");

	Config *config = &opentyrian_config;

	FILE *file = dir_fopen_warn(get_user_directory(), "opentyrian.cfg", "r");
	if (file == NULL)
		return false;

	if (!config_parse(config, file))
	{
		fclose(file);

		return false;
	}

	ConfigSection *section;

	section = config_find_section(config, "video", NULL);
	if (section != NULL)
	{
		config_get_bool_option(section, "fullscreen", &fullscreen_enabled);

		const char *scaler;
		if (config_get_string_option(section, "scaler", &scaler))
			set_scaler_by_name(scaler);
	}

	fclose(file);

	return true;
}

bool save_opentyrian_config( void )
{
	Config *config = &opentyrian_config;

	ConfigSection *section;

	section = config_find_or_add_section(config, "video", NULL);
	if (section == NULL)
		exit(EXIT_FAILURE);  // out of memory

	config_set_bool_option(section, "fullscreen", fullscreen_enabled, NO_YES);

	config_set_string_option(section, "scaler", scalers[scaler].name);

#ifndef TARGET_WIN32
	mkdir(get_user_directory(), 0700);
#else
	mkdir(get_user_directory());
#endif
	FILE *file = dir_fopen(get_user_directory(), "opentyrian.cfg", "wb");
	if (file == NULL)
		return false;

	config_write(config, file);
	printf("config write");

#if defined(TARGET_UNIX)
	fsync(fileno(file));
#endif
	fclose(file);

	return true;
}

static void playeritems_to_pitems( JE_PItemsType pItems, PlayerItems *items, JE_byte initial_episode_num )
{
	pItems[0]  = items->weapon[FRONT_WEAPON].id;
	pItems[1]  = items->weapon[REAR_WEAPON].id;
	pItems[2]  = items->super_arcade_mode;
	pItems[3]  = items->sidekick[LEFT_SIDEKICK];
	pItems[4]  = items->sidekick[RIGHT_SIDEKICK];
	pItems[5]  = items->generator;
	pItems[6]  = items->sidekick_level;
	pItems[7]  = items->sidekick_series;
	pItems[8]  = initial_episode_num;
	pItems[9]  = items->shield;
	pItems[10] = items->special;
	pItems[11] = items->ship;
}

static void pitems_to_playeritems( PlayerItems *items, JE_PItemsType pItems, JE_byte *initial_episode_num )
{
	items->weapon[FRONT_WEAPON].id  = pItems[0];
	items->weapon[REAR_WEAPON].id   = pItems[1];
	items->super_arcade_mode        = pItems[2];
	items->sidekick[LEFT_SIDEKICK]  = pItems[3];
	items->sidekick[RIGHT_SIDEKICK] = pItems[4];
	items->generator                = pItems[5];
	items->sidekick_level           = pItems[6];
	items->sidekick_series          = pItems[7];
	if (initial_episode_num != NULL)
		*initial_episode_num        = pItems[8];
	items->shield                   = pItems[9];
	items->special                  = pItems[10];
	items->ship                     = pItems[11];
}

void JE_saveGame( JE_byte slot, const char *name )
{
	saveFiles[slot-1].initialDifficulty = initialDifficulty;
	saveFiles[slot-1].gameHasRepeated = gameHasRepeated;
	saveFiles[slot-1].level = saveLevel;

	if (superTyrian)
		player[0].items.super_arcade_mode = SA_SUPERTYRIAN;
	else if (superArcadeMode == SA_NONE && onePlayerAction)
		player[0].items.super_arcade_mode = SA_ARCADE;
	else
		player[0].items.super_arcade_mode = superArcadeMode;

	playeritems_to_pitems(saveFiles[slot-1].items, &player[0].items, initial_episode_num);

	if (twoPlayerMode)
		playeritems_to_pitems(saveFiles[slot-1].lastItems, &player[1].items, 0);
	else
		playeritems_to_pitems(saveFiles[slot-1].lastItems, &player[0].last_items, 0);

	saveFiles[slot-1].score  = player[0].cash;
	saveFiles[slot-1].score2 = player[1].cash;

	memcpy(&saveFiles[slot-1].levelName, &lastLevelName, sizeof(lastLevelName));
	saveFiles[slot-1].cubes  = lastCubeMax;

	if (strcmp(lastLevelName, "Completed") == 0)
	{
		temp = episodeNum - 1;
		if (temp < 1)
		{
			temp = EPISODE_AVAILABLE; /* JE: {Episodemax is 4 for completion purposes} */
		}
		saveFiles[slot-1].episode = temp;
	}
	else
	{
		saveFiles[slot-1].episode = episodeNum;
	}

	saveFiles[slot-1].difficulty = difficultyLevel;
	saveFiles[slot-1].secretHint = secretHint;
	saveFiles[slot-1].input1 = inputDevice[0];
	saveFiles[slot-1].input2 = inputDevice[1];

	strcpy(saveFiles[slot-1].name, name);

	for (uint port = 0; port < 2; ++port)
	{
		// if two-player, use first player's front and second player's rear weapon
		saveFiles[slot-1].power[port] = player[twoPlayerMode ? port : 0].items.weapon[port].power;
	}

	JE_saveConfiguration();
}

void JE_loadGame( JE_byte slot )
{
	superTyrian = false;
	onePlayerAction = false;
	twoPlayerMode = false;
	extraGame = false;
	galagaMode = false;

	initialDifficulty = saveFiles[slot-1].initialDifficulty;
	gameHasRepeated   = saveFiles[slot-1].gameHasRepeated;
	twoPlayerMode     = (slot-1) > 10;
	difficultyLevel   = saveFiles[slot-1].difficulty;

	pitems_to_playeritems(&player[0].items, saveFiles[slot-1].items, &initial_episode_num);

	superArcadeMode = player[0].items.super_arcade_mode;

	if (superArcadeMode == SA_SUPERTYRIAN)
		superTyrian = true;
	if (superArcadeMode != SA_NONE)
		onePlayerAction = true;
	if (superArcadeMode > SA_NORTSHIPZ)
		superArcadeMode = SA_NONE;

	if (twoPlayerMode)
	{
		onePlayerAction = false;

		pitems_to_playeritems(&player[1].items, saveFiles[slot-1].lastItems, NULL);
	}
	else
	{
		pitems_to_playeritems(&player[0].last_items, saveFiles[slot-1].lastItems, NULL);
	}

	/* Compatibility with old version */
	if (player[1].items.sidekick_level < 101)
	{
		player[1].items.sidekick_level = 101;
		player[1].items.sidekick_series = player[1].items.sidekick[LEFT_SIDEKICK];
	}

	player[0].cash = saveFiles[slot-1].score;
	player[1].cash = saveFiles[slot-1].score2;

	mainLevel   = saveFiles[slot-1].level;
	cubeMax     = saveFiles[slot-1].cubes;
	lastCubeMax = cubeMax;

	secretHint = saveFiles[slot-1].secretHint;
	inputDevice[0] = saveFiles[slot-1].input1;
	inputDevice[1] = saveFiles[slot-1].input2;

	for (uint port = 0; port < 2; ++port)
	{
		// if two-player, use first player's front and second player's rear weapon
		player[twoPlayerMode ? port : 0].items.weapon[port].power = saveFiles[slot-1].power[port];
	}

	int episode = saveFiles[slot-1].episode;

	memcpy(&levelName, &saveFiles[slot-1].levelName, sizeof(levelName));

	if (strcmp(levelName, "Completed") == 0)
	{
		if (episode == EPISODE_AVAILABLE)
		{
			episode = 1;
		} else if (episode < EPISODE_AVAILABLE) {
			episode++;
		}
		/* Increment episode.  Episode EPISODE_AVAILABLE goes to 1. */
	}

	JE_initEpisode(episode);
	saveLevel = mainLevel;
	memcpy(&lastLevelName, &levelName, sizeof(levelName));
}

void JE_initProcessorType( void )
{
	/* SYN: Originally this proc looked at your hardware specs and chose appropriate options. We don't care, so I'll just set
	   decent defaults here. */

	wild = false;
	superWild = false;
	smoothScroll = true;
	explosionTransparent = true;
	filtrationAvail = false;
	background2 = true;
	displayScore = true;

	switch (processorType)
	{
		case 1: /* 386 */
			background2 = false;
			displayScore = false;
			explosionTransparent = false;
			break;
		case 2: /* 486 - Default */
			break;
		case 3: /* High Detail */
			smoothScroll = false;
			break;
		case 4: /* Pentium */
			wild = true;
			filtrationAvail = true;
			break;
		case 5: /* Nonstandard VGA */
			smoothScroll = false;
			break;
		case 6: /* SuperWild */
			wild = true;
			superWild = true;
			filtrationAvail = true;
			break;
	}

	switch (gameSpeed)
	{
		case 1:  /* Slug Mode */
			fastPlay = 3;
			break;
		case 2:  /* Slower */
			fastPlay = 4;
			break;
		case 3: /* Slow */
			fastPlay = 5;
			break;
		case 4: /* Normal */
			fastPlay = 0;
			break;
		case 5: /* Pentium Hyper */
			fastPlay = 1;
			break;
	}

}

void JE_setNewGameSpeed( void )
{
	pentiumMode = false;

	switch (fastPlay)
	{
	case 0:
		speed = 0x4300;
		smoothScroll = true;
		frameCountMax = 2;
		break;
	case 1:
		speed = 0x3000;
		smoothScroll = true;
		frameCountMax = 2;
		break;
	case 2:
		speed = 0x2000;
		smoothScroll = false;
		frameCountMax = 2;
		break;
	case 3:
		speed = 0x5300;
		smoothScroll = true;
		frameCountMax = 4;
		break;
	case 4:
		speed = 0x4300;
		smoothScroll = true;
		frameCountMax = 3;
		break;
	case 5:
		speed = 0x4300;
		smoothScroll = true;
		frameCountMax = 2;
		pentiumMode = true;
		break;
	}

  frameCount = frameCountMax;
  JE_resetTimerInt();
  JE_setTimerInt();
}

void JE_encryptSaveTemp( void )
{
	JE_SaveGameTemp s3;
	JE_word x;
	JE_byte y;

	memcpy(&s3, &saveTemp, sizeof(s3));

	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y += s3[x];
	}
	saveTemp[SAVE_FILE_SIZE] = y;

	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y -= s3[x];
	}
	saveTemp[SAVE_FILE_SIZE+1] = y;

	y = 1;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y = (y * s3[x]) + 1;
	}
	saveTemp[SAVE_FILE_SIZE+2] = y;

	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y = y ^ s3[x];
	}
	saveTemp[SAVE_FILE_SIZE+3] = y;

	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		saveTemp[x] = saveTemp[x] ^ cryptKey[(x+1) % 10];
		if (x > 0)
		{
			saveTemp[x] = saveTemp[x] ^ saveTemp[x - 1];
		}
	}
}

void JE_decryptSaveTemp( void )
{
	JE_boolean correct = true;
	JE_SaveGameTemp s2;
	int x;
	JE_byte y;

	/* Decrypt save game file */
	for (x = (SAVE_FILE_SIZE - 1); x >= 0; x--)
	{
		s2[x] = (JE_byte)saveTemp[x] ^ (JE_byte)(cryptKey[(x+1) % 10]);
		if (x > 0)
		{
			s2[x] ^= (JE_byte)saveTemp[x - 1];
		}

	}

	/* for (x = 0; x < SAVE_FILE_SIZE; x++) printf("%c", s2[x]); */

	/* Check save file for correctitude */
	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y += s2[x];
	}
	if (saveTemp[SAVE_FILE_SIZE] != y)
	{
		correct = false;
		printf("Failed additive checksum: %d vs %d\n", saveTemp[SAVE_FILE_SIZE], y);
	}

	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y -= s2[x];
	}
	if (saveTemp[SAVE_FILE_SIZE+1] != y)
	{
		correct = false;
		printf("Failed subtractive checksum: %d vs %d\n", saveTemp[SAVE_FILE_SIZE+1], y);
	}

	y = 1;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y = (y * s2[x]) + 1;
	}
	if (saveTemp[SAVE_FILE_SIZE+2] != y)
	{
		correct = false;
		printf("Failed multiplicative checksum: %d vs %d\n", saveTemp[SAVE_FILE_SIZE+2], y);
	}

	y = 0;
	for (x = 0; x < SAVE_FILE_SIZE; x++)
	{
		y = y ^ s2[x];
	}
	if (saveTemp[SAVE_FILE_SIZE+3] != y)
	{
		correct = false;
		printf("Failed XOR'd checksum: %d vs %d\n", saveTemp[SAVE_FILE_SIZE+3], y);
	}

	/* Barf and die if save file doesn't validate */
	if (!correct)
	{
		fprintf(stderr, "Error reading save file!\n");
		exit(255);
	}

	/* Keep decrypted version plz */
	memcpy(&saveTemp, &s2, sizeof(s2));
}

const char *get_user_directory( void )
{
	static char user_dir[500] = "";

	if (strlen(user_dir) == 0)
	{
#if defined(TARGET_UNIX)
		char *xdg_config_home = getenv("XDG_CONFIG_HOME");
		if (xdg_config_home != NULL)
		{
			snprintf(user_dir, sizeof(user_dir), "%s/opentyrian", xdg_config_home);
		}
		else
		{
			char *home = getenv("HOME");
			if (home != NULL)
			{
				snprintf(user_dir, sizeof(user_dir), "%s/.config/opentyrian", home);
			}
			else
			{
				strcpy(user_dir, ".");
			}
		}
#else
		strcpy(user_dir, ".");
#endif
	}

	return user_dir;
}

// for compatibility
Uint8 joyButtonAssign[4] = {1, 4, 5, 5};
Uint8 inputDevice_ = 0, jConfigure = 0, midiPort = 1;

void JE_loadConfiguration( void )
{
	FILE *fi;
	int z;
	JE_byte *p;
	int y;

	fi = dir_fopen_warn(get_user_directory(), "tyrian.cfg", "rb");
	if (fi && ftell_eof(fi) == 20 + sizeof(keySettings))
	{
		/* SYN: I've hardcoded the sizes here because the .CFG file format is fixed
		   anyways, so it's not like they'll change. */
		background2 = 0;
		efread(&background2, 1, 1, fi);
		efread(&gameSpeed, 1, 1, fi);

		efread(&inputDevice_, 1, 1, fi);
		efread(&jConfigure, 1, 1, fi);

		efread(&versionNum, 1, 1, fi);

		efread(&processorType, 1, 1, fi);
		efread(&midiPort, 1, 1, fi);
		efread(&soundEffects, 1, 1, fi);
		efread(&gammaCorrection, 1, 1, fi);
		efread(&difficultyLevel, 1, 1, fi);

		efread(joyButtonAssign, 1, 4, fi);

		efread(&tyrMusicVolume, 2, 1, fi);
		efread(&fxVolume, 2, 1, fi);

		efread(inputDevice, 1, 2, fi);

		efread(keySettings, sizeof(*keySettings), COUNTOF(keySettings), fi);

		fclose(fi);
	}
	else
	{
		printf("\nInvalid or missing TYRIAN.CFG! Continuing using defaults.\n\n");

		soundEffects = 1;
		memcpy(&keySettings, &defaultKeySettings, sizeof(keySettings));
		background2 = true;
		tyrMusicVolume = fxVolume = 128;
		gammaCorrection = 0;
		processorType = 3;
		gameSpeed = 4;
	}

	load_opentyrian_config();

	if (tyrMusicVolume > 255)
		tyrMusicVolume = 255;
	if (fxVolume > 255)
		fxVolume = 255;

	JE_calcFXVol();

	set_volume(tyrMusicVolume, fxVolume);

	fi = dir_fopen_warn(get_user_directory(), "tyrian.sav", "rb");
	if (fi)
	{

		fseek(fi, 0, SEEK_SET);
		efread(saveTemp, 1, sizeof(saveTemp), fi);
		JE_decryptSaveTemp();

		/* SYN: The original mostly blasted the save file into raw memory. However, our lives are not so
		   easy, because the C struct is necessarily a different size. So instead we have to loop
		   through each record and load fields manually. *emo tear* :'( */

		p = saveTemp;
		for (z = 0; z < SAVE_FILES_NUM; z++)
		{
			memcpy(&saveFiles[z].encode, p, sizeof(JE_word)); p += 2;
			saveFiles[z].encode = SDL_SwapLE16(saveFiles[z].encode);

			memcpy(&saveFiles[z].level, p, sizeof(JE_word)); p += 2;
			saveFiles[z].level = SDL_SwapLE16(saveFiles[z].level);

			memcpy(&saveFiles[z].items, p, sizeof(JE_PItemsType)); p += sizeof(JE_PItemsType);

			memcpy(&saveFiles[z].score, p, sizeof(JE_longint)); p += 4;
			saveFiles[z].score = SDL_SwapLE32(saveFiles[z].score);

			memcpy(&saveFiles[z].score2, p, sizeof(JE_longint)); p += 4;
			saveFiles[z].score2 = SDL_SwapLE32(saveFiles[z].score2);

			/* SYN: Pascal strings are prefixed by a byte holding the length! */
			memset(&saveFiles[z].levelName, 0, sizeof(saveFiles[z].levelName));
			memcpy(&saveFiles[z].levelName, &p[1], *p);
			p += 10;

			/* This was a BYTE array, not a STRING, in the original. Go fig. */
			memcpy(&saveFiles[z].name, p, 14);
			p += 14;

			memcpy(&saveFiles[z].cubes, p, sizeof(JE_byte)); p++;
			memcpy(&saveFiles[z].power, p, sizeof(JE_byte) * 2); p += 2;
			memcpy(&saveFiles[z].episode, p, sizeof(JE_byte)); p++;
			memcpy(&saveFiles[z].lastItems, p, sizeof(JE_PItemsType)); p += sizeof(JE_PItemsType);
			memcpy(&saveFiles[z].difficulty, p, sizeof(JE_byte)); p++;
			memcpy(&saveFiles[z].secretHint, p, sizeof(JE_byte)); p++;
			memcpy(&saveFiles[z].input1, p, sizeof(JE_byte)); p++;
			memcpy(&saveFiles[z].input2, p, sizeof(JE_byte)); p++;

			/* booleans were 1 byte in pascal -- working around it */
			Uint8 temp;
			memcpy(&temp, p, 1); p++;
			saveFiles[z].gameHasRepeated = temp != 0;

			memcpy(&saveFiles[z].initialDifficulty, p, sizeof(JE_byte)); p++;

			memcpy(&saveFiles[z].highScore1, p, sizeof(JE_longint)); p += 4;
			saveFiles[z].highScore1 = SDL_SwapLE32(saveFiles[z].highScore1);

			memcpy(&saveFiles[z].highScore2, p, sizeof(JE_longint)); p += 4;
			saveFiles[z].highScore2 = SDL_SwapLE32(saveFiles[z].highScore2);

			memset(&saveFiles[z].highScoreName, 0, sizeof(saveFiles[z].highScoreName));
			memcpy(&saveFiles[z].highScoreName, &p[1], *p);
			p += 30;

			memcpy(&saveFiles[z].highScoreDiff, p, sizeof(JE_byte)); p++;
		}

		/* SYN: This is truncating to bytes. I have no idea what this is doing or why. */
		/* TODO: Figure out what this is about and make sure it isn't broked. */
		editorLevel = (saveTemp[SIZEOF_SAVEGAMETEMP - 5] << 8) | saveTemp[SIZEOF_SAVEGAMETEMP - 6];

		fclose(fi);
	} else {
		/* We didn't have a save file! Let's make up random stuff! */
		editorLevel = 800;

		for (z = 0; z < 100; z++)
		{
			saveTemp[SAVE_FILES_SIZE + z] = initialItemAvail[z];
		}

		for (z = 0; z < SAVE_FILES_NUM; z++)
		{
			saveFiles[z].level = 0;

			for (y = 0; y < 14; y++)
			{
				saveFiles[z].name[y] = ' ';
			}
			saveFiles[z].name[14] = 0;

			saveFiles[z].highScore1 = ((mt_rand() % 20) + 1) * 1000;

			if (z % 6 > 2)
			{
				saveFiles[z].highScore2 = ((mt_rand() % 20) + 1) * 1000;
				strcpy(saveFiles[z].highScoreName, defaultTeamNames[mt_rand() % 22]);
			} else {
				strcpy(saveFiles[z].highScoreName, defaultHighScoreNames[mt_rand() % 34]);
			}
		}
	}

	JE_initProcessorType();
}

void JE_saveConfiguration( void )
{
	FILE *f;
	JE_byte *p;
	int z;

	p = saveTemp;
	for (z = 0; z < SAVE_FILES_NUM; z++)
	{
		JE_SaveFileType tempSaveFile;
		memcpy(&tempSaveFile, &saveFiles[z], sizeof(tempSaveFile));

		tempSaveFile.encode = SDL_SwapLE16(tempSaveFile.encode);
		memcpy(p, &tempSaveFile.encode, sizeof(JE_word)); p += 2;

		tempSaveFile.level = SDL_SwapLE16(tempSaveFile.level);
		memcpy(p, &tempSaveFile.level, sizeof(JE_word)); p += 2;

		memcpy(p, &tempSaveFile.items, sizeof(JE_PItemsType)); p += sizeof(JE_PItemsType);

		tempSaveFile.score = SDL_SwapLE32(tempSaveFile.score);
		memcpy(p, &tempSaveFile.score, sizeof(JE_longint)); p += 4;

		tempSaveFile.score2 = SDL_SwapLE32(tempSaveFile.score2);
		memcpy(p, &tempSaveFile.score2, sizeof(JE_longint)); p += 4;

		/* SYN: Pascal strings are prefixed by a byte holding the length! */
		memset(p, 0, sizeof(tempSaveFile.levelName));
		*p = strlen(tempSaveFile.levelName);
		memcpy(&p[1], &tempSaveFile.levelName, *p);
		p += 10;

		/* This was a BYTE array, not a STRING, in the original. Go fig. */
		memcpy(p, &tempSaveFile.name, 14);
		p += 14;

		memcpy(p, &tempSaveFile.cubes, sizeof(JE_byte)); p++;
		memcpy(p, &tempSaveFile.power, sizeof(JE_byte) * 2); p += 2;
		memcpy(p, &tempSaveFile.episode, sizeof(JE_byte)); p++;
		memcpy(p, &tempSaveFile.lastItems, sizeof(JE_PItemsType)); p += sizeof(JE_PItemsType);
		memcpy(p, &tempSaveFile.difficulty, sizeof(JE_byte)); p++;
		memcpy(p, &tempSaveFile.secretHint, sizeof(JE_byte)); p++;
		memcpy(p, &tempSaveFile.input1, sizeof(JE_byte)); p++;
		memcpy(p, &tempSaveFile.input2, sizeof(JE_byte)); p++;

		/* booleans were 1 byte in pascal -- working around it */
		Uint8 temp = tempSaveFile.gameHasRepeated != false;
		memcpy(p, &temp, 1); p++;

		memcpy(p, &tempSaveFile.initialDifficulty, sizeof(JE_byte)); p++;

		tempSaveFile.highScore1 = SDL_SwapLE32(tempSaveFile.highScore1);
		memcpy(p, &tempSaveFile.highScore1, sizeof(JE_longint)); p += 4;

		tempSaveFile.highScore2 = SDL_SwapLE32(tempSaveFile.highScore2);
		memcpy(p, &tempSaveFile.highScore2, sizeof(JE_longint)); p += 4;

		memset(p, 0, sizeof(tempSaveFile.highScoreName));
		*p = strlen(tempSaveFile.highScoreName);
		memcpy(&p[1], &tempSaveFile.highScoreName, *p);
		p += 30;

		memcpy(p, &tempSaveFile.highScoreDiff, sizeof(JE_byte)); p++;
	}

	saveTemp[SIZEOF_SAVEGAMETEMP - 6] = editorLevel >> 8;
	saveTemp[SIZEOF_SAVEGAMETEMP - 5] = editorLevel;

	JE_encryptSaveTemp();

#ifndef TARGET_WIN32
	mkdir(get_user_directory(), 0700);
#else
	mkdir(get_user_directory());
#endif

	f = dir_fopen_warn(get_user_directory(), "tyrian.sav", "wb");
	if (f != NULL)
	{
		efwrite(saveTemp, 1, sizeof(saveTemp), f);

#if defined(TARGET_UNIX)
		fsync(fileno(f));
#endif
		fclose(f);
	}

	JE_decryptSaveTemp();

	f = dir_fopen_warn(get_user_directory(), "tyrian.cfg", "wb");
	if (f != NULL)
	{
		efwrite(&background2, 1, 1, f);
		efwrite(&gameSpeed, 1, 1, f);

		efwrite(&inputDevice_, 1, 1, f);
		efwrite(&jConfigure, 1, 1, f);

		efwrite(&versionNum, 1, 1, f);
		efwrite(&processorType, 1, 1, f);
		efwrite(&midiPort, 1, 1, f);
		efwrite(&soundEffects, 1, 1, f);
		efwrite(&gammaCorrection, 1, 1, f);
		efwrite(&difficultyLevel, 1, 1, f);
		efwrite(joyButtonAssign, 1, 4, f);

		efwrite(&tyrMusicVolume, 2, 1, f);
		efwrite(&fxVolume, 2, 1, f);

		efwrite(inputDevice, 1, 2, f);

		efwrite(keySettings, sizeof(*keySettings), COUNTOF(keySettings), f);

#if defined(TARGET_UNIX)
		fsync(fileno(f));
#endif
		fclose(f);
	}

	save_opentyrian_config();
}

