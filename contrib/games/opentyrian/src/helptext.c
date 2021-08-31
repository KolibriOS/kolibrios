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
#include "helptext.h"

#include "config.h"
#include "episodes.h"
#include "file.h"
#include "fonthand.h"
#include "menus.h"
#include "opentyr.h"
#include "video.h"

#include <assert.h>
#include <string.h>


const JE_byte menuHelp[MENU_MAX][11] = /* [1..maxmenu, 1..11] */
{
	{  1, 34,  2,  3,  4,  5,                  0, 0, 0, 0, 0 },
	{  6,  7,  8,  9, 10, 11, 11, 12,                0, 0, 0 },
	{ 13, 14, 15, 15, 16, 17, 12,                 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{  4, 30, 30,  3,  5,                   0, 0, 0, 0, 0, 0 },
	{                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 16, 17, 15, 15, 12,                   0, 0, 0, 0, 0, 0 },
	{ 31, 31, 31, 31, 32, 12,                  0, 0, 0, 0, 0 },
	{  4, 34,  3,  5,                    0, 0, 0, 0, 0, 0, 0 }
};

JE_byte verticalHeight = 7;
JE_byte helpBoxColor = 12;
JE_byte helpBoxBrightness = 1;
JE_byte helpBoxShadeType = FULL_SHADE;

char helpTxt[39][231];                                                   /* [1..39] of string [230] */
char pName[21][16];                                                      /* [1..21] of string [15] */
char miscText[HELPTEXT_MISCTEXT_COUNT][42];                              /* [1..68] of string [41] */
char miscTextB[HELPTEXT_MISCTEXTB_COUNT][HELPTEXT_MISCTEXTB_SIZE];       /* [1..5] of string [10] */
char keyName[8][18];                                                     /* [1..8] of string [17] */
char menuText[7][HELPTEXT_MENUTEXT_SIZE];                                /* [1..7] of string [20] */
char outputs[9][31];                                                     /* [1..9] of string [30] */
char topicName[6][21];                                                   /* [1..6] of string [20] */
char mainMenuHelp[HELPTEXT_MAINMENUHELP_COUNT][66];                      /* [1..34] of string [65] */
char inGameText[6][21];                                                  /* [1..6] of string [20] */
char detailLevel[6][13];                                                 /* [1..6] of string [12] */
char gameSpeedText[5][13];                                               /* [1..5] of string [12] */
char inputDevices[3][13];                                                /* [1..3] of string [12] */
char networkText[HELPTEXT_NETWORKTEXT_COUNT][HELPTEXT_NETWORKTEXT_SIZE]; /* [1..4] of string [20] */
char difficultyNameB[11][21];                                            /* [0..9] of string [20] */
char joyButtonNames[5][21];                                              /* [1..5] of string [20] */
char superShips[HELPTEXT_SUPERSHIPS_COUNT][26];                          /* [0..10] of string [25] */
char specialName[HELPTEXT_SPECIALNAME_COUNT][10];                        /* [1..9] of string [9] */
char destructHelp[25][22];                                               /* [1..25] of string [21] */
char weaponNames[17][17];                                                /* [1..17] of string [16] */
char destructModeName[DESTRUCT_MODES][13];                               /* [1..destructmodes] of string [12] */
char shipInfo[HELPTEXT_SHIPINFO_COUNT][2][256];                          /* [1..13, 1..2] of string */
char menuInt[MENU_MAX+1][11][18];                                        /* [0..14, 1..11] of string [17] */


void decrypt_pascal_string( char *s, int len )
{
	static const unsigned char crypt_key[] = { 204, 129, 63, 255, 71, 19, 25, 62, 1, 99 };

	for (int i = len - 1; i >= 0; --i)
	{
		s[i] ^= crypt_key[i % sizeof(crypt_key)];
		if (i > 0)
			s[i] ^= s[i - 1];
	}
}

void read_encrypted_pascal_string( char *s, int size, FILE *f )
{
	int len = getc(f);
	if (len != EOF)
	{
		int skip = MAX((len + 1) - size, 0);
		assert(skip == 0);

		len -= skip;
		efread(s, 1, len, f);
		if (size > 0)
			s[len] = '\0';
		fseek(f, skip, SEEK_CUR);

		decrypt_pascal_string(s, len);
	}
}

void skip_pascal_string( FILE *f )
{
	int len = getc(f);
	fseek(f, len, SEEK_CUR);
}

void JE_helpBox( SDL_Surface *screen,  int x, int y, const char *message, unsigned int boxwidth )
{
	JE_byte startpos, endpos, pos;
	JE_boolean endstring;

	char substring[256];

	if (strlen(message) == 0)
	{
		return;
	}

	pos = 1;
	endpos = 0;
	endstring = false;

	do
	{
		startpos = endpos + 1;

		do
		{
			endpos = pos;
			do
			{
				pos++;
				if (pos == strlen(message))
				{
					endstring = true;
					if ((unsigned)(pos - startpos) < boxwidth)
					{
						endpos = pos + 1;
					}
				}

			} while (!(message[pos-1] == ' ' || endstring));

		} while (!((unsigned)(pos - startpos) > boxwidth || endstring));

		SDL_strlcpy(substring, message + startpos - 1, MIN((size_t)(endpos - startpos + 1), sizeof(substring)));
		JE_textShade(screen, x, y, substring, helpBoxColor, helpBoxBrightness, helpBoxShadeType);

		y += verticalHeight;

	} while (!endstring);

	if (endpos != pos + 1)
	{
		JE_textShade(screen, x, y, message + endpos, helpBoxColor, helpBoxBrightness, helpBoxShadeType);
	}

	helpBoxColor = 12;
	helpBoxShadeType = FULL_SHADE;
}

void JE_HBox( SDL_Surface *screen, int x, int y, unsigned int  messagenum, unsigned int boxwidth )
{
	JE_helpBox(screen, x, y, helpTxt[messagenum-1], boxwidth);
}

void JE_loadHelpText( void )
{
	const unsigned int menuInt_entries[MENU_MAX + 1] = { -1, 7, 9, 8, -1, -1, 11, -1, -1, -1, 6, 4, 6, 7, 5 };
	
	FILE *f = dir_fopen_die(data_dir(), "tyrian.hdt", "rb");
	efread(&episode1DataLoc, sizeof(JE_longint), 1, f);

	/*Online Help*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(helpTxt); ++i)
		read_encrypted_pascal_string(helpTxt[i], sizeof(helpTxt[i]), f);
	skip_pascal_string(f);

	/*Planet names*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(pName); ++i)
		read_encrypted_pascal_string(pName[i], sizeof(pName[i]), f);
	skip_pascal_string(f);

	/*Miscellaneous text*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(miscText); ++i)
		read_encrypted_pascal_string(miscText[i], sizeof(miscText[i]), f);
	skip_pascal_string(f);

	/*Little Miscellaneous text*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(miscTextB); ++i)
		read_encrypted_pascal_string(miscTextB[i], sizeof(miscTextB[i]), f);
	skip_pascal_string(f);

	/*Key names*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[6]; ++i)
		read_encrypted_pascal_string(menuInt[6][i], sizeof(menuInt[6][i]), f);
	skip_pascal_string(f);

	/*Main Menu*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(menuText); ++i)
		read_encrypted_pascal_string(menuText[i], sizeof(menuText[i]), f);
	skip_pascal_string(f);

	/*Event text*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(outputs); ++i)
		read_encrypted_pascal_string(outputs[i], sizeof(outputs[i]), f);
	skip_pascal_string(f);

	/*Help topics*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(topicName); ++i)
		read_encrypted_pascal_string(topicName[i], sizeof(topicName[i]), f);
	skip_pascal_string(f);

	/*Main Menu Help*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(mainMenuHelp); ++i)
		read_encrypted_pascal_string(mainMenuHelp[i], sizeof(mainMenuHelp[i]), f);
	skip_pascal_string(f);

	/*Menu 1 - Main*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[1]; ++i)
		read_encrypted_pascal_string(menuInt[1][i], sizeof(menuInt[1][i]), f);
	skip_pascal_string(f);

	/*Menu 2 - Items*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[2]; ++i)
		read_encrypted_pascal_string(menuInt[2][i], sizeof(menuInt[2][i]), f);
	skip_pascal_string(f);

	/*Menu 3 - Options*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[3]; ++i)
		read_encrypted_pascal_string(menuInt[3][i], sizeof(menuInt[3][i]), f);
	skip_pascal_string(f);

	/*InGame Menu*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(inGameText); ++i)
		read_encrypted_pascal_string(inGameText[i], sizeof(inGameText[i]), f);
	skip_pascal_string(f);

	/*Detail Level*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(detailLevel); ++i)
		read_encrypted_pascal_string(detailLevel[i], sizeof(detailLevel[i]), f);
	skip_pascal_string(f);

	/*Game speed text*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(gameSpeedText); ++i)
		read_encrypted_pascal_string(gameSpeedText[i], sizeof(gameSpeedText[i]), f);
	skip_pascal_string(f);

	// episode names
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(episode_name); ++i)
		read_encrypted_pascal_string(episode_name[i], sizeof(episode_name[i]), f);
	skip_pascal_string(f);

	// difficulty names
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(difficulty_name); ++i)
		read_encrypted_pascal_string(difficulty_name[i], sizeof(difficulty_name[i]), f);
	skip_pascal_string(f);

	// gameplay mode names
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(gameplay_name); ++i)
		read_encrypted_pascal_string(gameplay_name[i], sizeof(gameplay_name[i]), f);
	skip_pascal_string(f);

	/*Menu 10 - 2Player Main*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[10]; ++i)
		read_encrypted_pascal_string(menuInt[10][i], sizeof(menuInt[10][i]), f);
	skip_pascal_string(f);

	/*Input Devices*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(inputDevices); ++i)
		read_encrypted_pascal_string(inputDevices[i], sizeof(inputDevices[i]), f);
	skip_pascal_string(f);

	/*Network text*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(networkText); ++i)
		read_encrypted_pascal_string(networkText[i], sizeof(networkText[i]), f);
	skip_pascal_string(f);

	/*Menu 11 - 2Player Network*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[11]; ++i)
		read_encrypted_pascal_string(menuInt[11][i], sizeof(menuInt[11][i]), f);
	skip_pascal_string(f);

	/*HighScore Difficulty Names*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(difficultyNameB); ++i)
		read_encrypted_pascal_string(difficultyNameB[i], sizeof(difficultyNameB[i]), f);
	skip_pascal_string(f);

	/*Menu 12 - Network Options*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[12]; ++i)
		read_encrypted_pascal_string(menuInt[12][i], sizeof(menuInt[12][i]), f);
	skip_pascal_string(f);

	/*Menu 13 - Joystick*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < menuInt_entries[13]; ++i)
		read_encrypted_pascal_string(menuInt[13][i], sizeof(menuInt[13][i]), f);
	skip_pascal_string(f);

	/*Joystick Button Assignments*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(joyButtonNames); ++i)
		read_encrypted_pascal_string(joyButtonNames[i], sizeof(joyButtonNames[i]), f);
	skip_pascal_string(f);

	/*SuperShips - For Super Arcade Mode*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(superShips); ++i)
		read_encrypted_pascal_string(superShips[i], sizeof(superShips[i]), f);
	skip_pascal_string(f);

	/*SuperShips - For Super Arcade Mode*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(specialName); ++i)
		read_encrypted_pascal_string(specialName[i], sizeof(specialName[i]), f);
	skip_pascal_string(f);

	/*Secret DESTRUCT game*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(destructHelp); ++i)
		read_encrypted_pascal_string(destructHelp[i], sizeof(destructHelp[i]), f);
	skip_pascal_string(f);

	/*Secret DESTRUCT weapons*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(weaponNames); ++i)
		read_encrypted_pascal_string(weaponNames[i], sizeof(weaponNames[i]), f);
	skip_pascal_string(f);

	/*Secret DESTRUCT modes*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(destructModeName); ++i)
		read_encrypted_pascal_string(destructModeName[i], sizeof(destructModeName[i]), f);
	skip_pascal_string(f);

	/*NEW: Ship Info*/
	skip_pascal_string(f);
	for (unsigned int i = 0; i < COUNTOF(shipInfo); ++i)
	{
		read_encrypted_pascal_string(shipInfo[i][0], sizeof(shipInfo[i][0]), f);
		read_encrypted_pascal_string(shipInfo[i][1], sizeof(shipInfo[i][1]), f);
	}
	skip_pascal_string(f);
	
	fclose(f);
}

