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
#include "nortvars.h"

#include "file.h"
#include "joystick.h"
#include "keyboard.h"
#include "opentyr.h"
#include "vga256d.h"
#include "video.h"

#include <assert.h>
#include <ctype.h>

JE_boolean inputDetected;

JE_boolean JE_anyButton( void )
{
	poll_joysticks();
	service_SDL_events(true);
	return newkey || mousedown || joydown;
}

void JE_dBar3( SDL_Surface *surface, JE_integer x,  JE_integer y,  JE_integer num,  JE_integer col )
{
	JE_byte z;
	JE_byte zWait = 2;

	col += 2;

	for (z = 0; z <= num; z++)
	{
		JE_rectangle(surface, x, y - 1, x + 8, y, col); /* <MXD> SEGa000 */
		if (zWait > 0)
		{
			zWait--;
		} else {
			col++;
			zWait = 1;
		}
		y -= 2;
	}
}

void JE_barDrawShadow( SDL_Surface *surface, JE_word x, JE_word y, JE_word res, JE_word col, JE_word amt, JE_word xsize, JE_word ysize )
{
	xsize--;
	ysize--;

	for (int z = 1; z <= amt / res; z++)
	{
		JE_barShade(surface, x+2, y+2, x+xsize+2, y+ysize+2);
		fill_rectangle_xy(surface, x, y, x+xsize, y+ysize, col+12);
		fill_rectangle_xy(surface, x, y, x+xsize, y, col+13);
		JE_pix(surface, x, y, col+15);
		fill_rectangle_xy(surface, x, y+ysize, x+xsize, y+ysize, col+11);
		x += xsize + 2;
	}

	amt %= res;
	if (amt > 0)
	{
		JE_barShade(surface, x+2, y+2, x+xsize+2, y+ysize+2);
		fill_rectangle_xy(surface, x,y, x+xsize, y+ysize, col+(12 / res * amt));
	}
}

void JE_wipeKey( void )
{
	// /!\ Doesn't seems to affect anything.
}

