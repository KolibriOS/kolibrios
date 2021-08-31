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
#ifndef MUSMAST_H
#define MUSMAST_H

#include "opentyr.h"


#define DEFAULT_SONG_BUY 2
#define SONG_LEVELEND    9
#define SONG_GAMEOVER    10
#define SONG_MAPVIEW     19
#define SONG_ENDGAME1    7
#define SONG_ZANAC       31
#define SONG_TITLE       29

#define MUSIC_NUM 41

extern JE_byte songBuy;
extern const char musicFile[MUSIC_NUM][13];
extern const char musicTitle[MUSIC_NUM][48];
extern JE_boolean musicFade;

#endif /* MUSMAST_H */

