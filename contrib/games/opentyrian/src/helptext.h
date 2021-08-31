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
#ifndef HELPTEXT_H
#define HELPTEXT_H

#include "opentyr.h"

#include "SDL.h"

#include <stdio.h>

#define MENU_MAX 14

#define DESTRUCT_MODES 5

extern const JE_byte menuHelp[MENU_MAX][11];   /* [1..14, 1..11] */

extern JE_byte verticalHeight;
extern JE_byte helpBoxColor, helpBoxBrightness, helpBoxShadeType;

#define HELPTEXT_MISCTEXT_COUNT 68
#define HELPTEXT_MISCTEXTB_COUNT 5
#define HELPTEXT_MISCTEXTB_SIZE 11
#define HELPTEXT_MENUTEXT_SIZE 21
#define HELPTEXT_MAINMENUHELP_COUNT 34
#define HELPTEXT_NETWORKTEXT_COUNT 4
#define HELPTEXT_NETWORKTEXT_SIZE 22
#define HELPTEXT_SUPERSHIPS_COUNT 11
#define HELPTEXT_SPECIALNAME_COUNT 9
#define HELPTEXT_SHIPINFO_COUNT 13

extern char helpTxt[39][231];
extern char pName[21][16];
extern char miscText[HELPTEXT_MISCTEXT_COUNT][42];
extern char miscTextB[HELPTEXT_MISCTEXTB_COUNT][HELPTEXT_MISCTEXTB_SIZE];
extern char keyName[8][18];
extern char menuText[7][HELPTEXT_MENUTEXT_SIZE];
extern char outputs[9][31];
extern char topicName[6][21];
extern char mainMenuHelp[HELPTEXT_MAINMENUHELP_COUNT][66];
extern char inGameText[6][21];
extern char detailLevel[6][13];
extern char gameSpeedText[5][13];
extern char inputDevices[3][13];
extern char networkText[HELPTEXT_NETWORKTEXT_COUNT][HELPTEXT_NETWORKTEXT_SIZE];
extern char difficultyNameB[11][21];
extern char joyButtonNames[5][21];
extern char superShips[HELPTEXT_SUPERSHIPS_COUNT][26];
extern char specialName[HELPTEXT_SPECIALNAME_COUNT][10];
extern char destructHelp[25][22];
extern char weaponNames[17][17];
extern char destructModeName[DESTRUCT_MODES][13];
extern char shipInfo[HELPTEXT_SHIPINFO_COUNT][2][256];
extern char menuInt[MENU_MAX+1][11][18];

void read_encrypted_pascal_string( char *s, int size, FILE *f );
void skip_pascal_string( FILE *f );

void JE_helpBox( SDL_Surface *screen, int x, int y, const char *message, unsigned int boxwidth );
void JE_HBox( SDL_Surface *screen, int x, int y, unsigned int  messagenum, unsigned int boxwidth );
void JE_loadHelpText( void );

#endif /* HELPTEXT_H */

