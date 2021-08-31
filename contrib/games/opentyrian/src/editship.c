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
#include "editship.h"

#include "config.h"
#include "file.h"
#include "opentyr.h"

#define SAS (sizeof(JE_ShipsType) - 4)

const JE_byte extraCryptKey[10] = { 58, 23, 16, 192, 254, 82, 113, 147, 62, 99 };

JE_boolean extraAvail;
JE_ShipsType extraShips;
void *extraShapes;
JE_word extraShapeSize;

void JE_decryptShips( void )
{
	JE_boolean correct = true;
	JE_ShipsType s2;
	JE_byte y;
	
	for (int x = SAS - 1; x >= 0; x--)
	{
		s2[x] = extraShips[x] ^ extraCryptKey[(x + 1) % 10];
		if (x > 0)
			s2[x] ^= extraShips[x - 1];
	}  /*  <= Key Decryption Test (Reversed key) */
	
	y = 0;
	for (uint x = 0; x < SAS; x++)
		y += s2[x];
	if (extraShips[SAS + 0] != y)
		correct = false;
	
	y = 0;
	for (uint x = 0; x < SAS; x++)
		y -= s2[x];
	if (extraShips[SAS + 1] != y)
		correct = false;
	
	y = 1;
	for (uint x = 0; x < SAS; x++)
		y = y * s2[x] + 1;
	if (extraShips[SAS + 2] != y)
		correct = false;
	
	y = 0;
	for (uint x = 0; x < SAS; x++)
		y ^= s2[x];
	if (extraShips[SAS + 3] != y)
		correct = false;
	
	if (!correct)
		exit(255);
	
	memcpy(extraShips, s2, sizeof(extraShips));
}

void JE_loadExtraShapes( void )
{
	FILE *f = dir_fopen(get_user_directory(), "newsh$.shp", "rb");
	
	if (f)
	{
		extraAvail = true;
		extraShapeSize = ftell_eof(f) - sizeof(extraShips);
		extraShapes = malloc(extraShapeSize);
		efread(extraShapes, extraShapeSize, 1, f);
		efread(extraShips, sizeof(extraShips), 1, f);
		JE_decryptShips();
		fclose(f);
	}
}

