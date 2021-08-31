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
#ifndef CONFIG_H
#define CONFIG_H

#include "opentyr.h"
#include "config_file.h"

#include "SDL.h"

#include <stdio.h>

#define SAVE_FILES_NUM (11 * 2)

/* These are necessary because the size of the structure has changed from the original, but we
   need to know the original sizes in order to find things in TYRIAN.SAV */
#define SAVE_FILES_SIZE 2398
#define SIZEOF_SAVEGAMETEMP SAVE_FILES_SIZE + 4 + 100
#define SAVE_FILE_SIZE (SIZEOF_SAVEGAMETEMP - 4)

/*#define SAVE_FILES_SIZE (2502 - 4)
#define SAVE_FILE_SIZE (SAVE_FILES_SIZE)*/

typedef SDLKey JE_KeySettingType[8]; /* [1..8] */
typedef JE_byte JE_PItemsType[12]; /* [1..12] */

typedef JE_byte JE_EditorItemAvailType[100]; /* [1..100] */

typedef struct
{
	JE_word       encode;
	JE_word       level;
	JE_PItemsType items;
	JE_longint    score;
	JE_longint    score2;
	char          levelName[11]; /* string [9]; */ /* SYN: Added one more byte to match lastLevelName below */
	JE_char       name[15]; /* [1..14] */ /* SYN: Added extra byte for null */
	JE_byte       cubes;
	JE_byte       power[2]; /* [1..2] */
	JE_byte       episode;
	JE_PItemsType lastItems;
	JE_byte       difficulty;
	JE_byte       secretHint;
	JE_byte       input1;
	JE_byte       input2;
	JE_boolean    gameHasRepeated; /*See if you went from one episode to another*/
	JE_byte       initialDifficulty;

	/* High Scores - Each episode has both sets of 1&2 player selections - with 3 in each */
	JE_longint    highScore1,
	              highScore2;
	char          highScoreName[30]; /* string [29] */
	JE_byte       highScoreDiff;
} JE_SaveFileType;

typedef JE_SaveFileType JE_SaveFilesType[SAVE_FILES_NUM]; /* [1..savefilesnum] */
typedef JE_byte JE_SaveGameTemp[SAVE_FILES_SIZE + 4 + 100]; /* [1..sizeof(savefilestype) + 4 + 100] */

extern const JE_byte cryptKey[10];
extern const JE_KeySettingType defaultKeySettings;
extern const char defaultHighScoreNames[34][23];
extern const char defaultTeamNames[22][25];
extern const JE_EditorItemAvailType initialItemAvail;
extern JE_boolean smoothies[9];
extern JE_byte starShowVGASpecialCode;
extern JE_word lastCubeMax, cubeMax;
extern JE_word cubeList[4];
extern JE_boolean gameHasRepeated;
extern JE_shortint difficultyLevel, oldDifficultyLevel, initialDifficulty;
extern uint power, lastPower, powerAdd;
extern JE_byte shieldWait, shieldT;

enum
{
	SHOT_FRONT,
	SHOT_REAR,
	SHOT_LEFT_SIDEKICK,
	SHOT_RIGHT_SIDEKICK,
	SHOT_MISC,
	SHOT_P2_CHARGE,
	SHOT_P1_SUPERBOMB,
	SHOT_P2_SUPERBOMB,
	SHOT_SPECIAL,
	SHOT_NORTSPARKS,
	SHOT_SPECIAL2
};

extern JE_byte shotRepeat[11], shotMultiPos[11];
extern JE_boolean portConfigChange, portConfigDone;
extern char lastLevelName[11], levelName[11];
extern JE_byte mainLevel, nextLevel, saveLevel;
extern JE_KeySettingType keySettings;
extern JE_shortint levelFilter, levelFilterNew, levelBrightness, levelBrightnessChg;
extern JE_boolean filtrationAvail, filterActive, filterFade, filterFadeStart;
extern JE_boolean gameJustLoaded;
extern JE_boolean galagaMode;
extern JE_boolean extraGame;
extern JE_boolean twoPlayerMode, twoPlayerLinked, onePlayerAction, superTyrian, trentWin;
extern JE_byte superArcadeMode;
extern JE_byte superArcadePowerUp;
extern JE_real linkGunDirec;
extern JE_byte inputDevice[2];
extern JE_byte secretHint;
extern JE_byte background3over;
extern JE_byte background2over;
extern JE_byte gammaCorrection;
extern JE_boolean superPause, explosionTransparent, youAreCheating, displayScore, background2, smoothScroll, wild, superWild, starActive, topEnemyOver, skyEnemyOverAll, background2notTransparent;
extern JE_byte versionNum;
extern JE_byte fastPlay;
extern JE_boolean pentiumMode;
extern JE_byte gameSpeed;
extern JE_byte processorType;
extern JE_SaveFilesType saveFiles;
extern JE_SaveGameTemp saveTemp;
extern JE_word editorLevel;

extern Config opentyrian_config;

void JE_initProcessorType( void );
void JE_setNewGameSpeed( void );
const char *get_user_directory( void );
void JE_loadConfiguration( void );
void JE_saveConfiguration( void );

void JE_saveGame( JE_byte slot, const char *name );
void JE_loadGame( JE_byte slot );

void JE_encryptSaveTemp( void );
void JE_decryptSaveTemp( void );

#endif /* CONFIG_H */

