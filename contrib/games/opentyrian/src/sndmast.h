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
#ifndef SNDMAST_H
#define SNDMAST_H

#include "opentyr.h"

#define SAMPLE_COUNT 38

enum
{
	S_NONE             =  0,
	S_WEAPON_1         =  1,
	S_WEAPON_2         =  2,
	S_ENEMY_HIT        =  3,
	S_EXPLOSION_4      =  4,
	S_WEAPON_5         =  5,
	S_WEAPON_6         =  6,
	S_WEAPON_7         =  7,
	S_SELECT           =  8,
	S_EXPLOSION_8      =  8,
	S_EXPLOSION_9      =  9,
	S_WEAPON_10        = 10,
	S_EXPLOSION_11     = 11,
	S_EXPLOSION_12     = 12,
	S_WEAPON_13        = 13,
	S_WEAPON_14        = 14,
	S_WEAPON_15        = 15,
	S_SPRING           = 16,
	S_WARNING          = 17,
	S_ITEM             = 18,
	S_HULL_HIT         = 19,
	S_MACHINE_GUN      = 20,
	S_SOUL_OF_ZINGLON  = 21,
	S_EXPLOSION_22     = 22,
	S_CLINK            = 23,
	S_CLICK            = 24,
	S_WEAPON_25        = 25,
	S_WEAPON_26        = 26,
	S_SHIELD_HIT       = 27,
	S_CURSOR           = 28,
	S_POWERUP          = 29,
	V_CLEARED_PLATFORM = 30,  // "Cleared enemy platform."
	V_BOSS             = 31,  // "Large enemy approaching."
	V_ENEMIES          = 32,  // "Enemies ahead."
	V_GOOD_LUCK        = 33,  // "Good luck."
	V_LEVEL_END        = 34,  // "Level completed."
	V_DANGER           = 35,  // "Danger."
	V_SPIKES           = 36,  // "Warning: spikes ahead."
	V_DATA_CUBE        = 37,  // "Data acquired."
	V_ACCELERATE       = 38,  // "Unexplained speed increase."
};

extern const char soundTitle[SAMPLE_COUNT][9];
extern const JE_byte windowTextSamples[9];

#endif /* SNDMAST_H */

