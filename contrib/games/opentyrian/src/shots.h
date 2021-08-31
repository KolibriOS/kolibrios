/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2013  The OpenTyrian Development Team
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
#ifndef SHOTS_H
#define SHOTS_H
#include "opentyr.h"

typedef struct {
	JE_integer shotX, shotY, shotXM, shotYM, shotXC, shotYC;
	JE_boolean shotComplicated;
	JE_integer shotDevX, shotDirX, shotDevY, shotDirY, shotCirSizeX, shotCirSizeY;
	JE_byte shotTrail;
	JE_word shotGr, shotAni, shotAniMax;
	Uint8 shotDmg;
	JE_byte shotBlastFilter, chainReaction, playerNumber, aimAtEnemy, aimDelay, aimDelayMax;
} PlayerShotDataType;

#define MAX_PWEAPON     81 /* 81*/
extern PlayerShotDataType playerShotData[MAX_PWEAPON + 1];
extern JE_byte shotAvail[MAX_PWEAPON];

/** Used in the shop to show weapon previews. */
void simulate_player_shots( void );

/** Points shot movement in the specified direction. Used for the turret gun. */
void player_shot_set_direction( JE_integer shot_id, uint weapon_id, JE_real direction );

/** Moves and draws a shot. Does \b not collide it with enemies.
 * \return False if the shot went offscreen, true otherwise.
 */
bool player_shot_move_and_draw(
		int shot_id, bool* out_is_special,
		int* out_shotx, int* out_shoty,
		JE_integer* out_shot_damage, JE_byte* out_blast_filter,
		JE_byte* out_chain, JE_byte* out_playerNum,
		JE_word* out_special_radiusw, JE_word* out_special_radiush );

/** Creates a player shot. */
JE_integer player_shot_create( JE_word portnum, uint shot_i, JE_word px, JE_word py,
                        JE_word mousex, JE_word mousey,
                        JE_word wpnum, JE_byte playernum );

#endif // SHOTS_H
