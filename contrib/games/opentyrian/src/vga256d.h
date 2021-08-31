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
#ifndef VGA256D_H
#define VGA256D_H

#include "opentyr.h"

#include "SDL.h"

void JE_pix( SDL_Surface *surface, int x, int y, JE_byte c );
void JE_pix3( SDL_Surface *surface, int x, int y, JE_byte c );
void JE_rectangle( SDL_Surface *surface, int a, int b, int c, int d, int e );

void fill_rectangle_xy( SDL_Surface *, int x, int y, int x2, int y2, Uint8 color );

void JE_barShade( SDL_Surface *surface, int a, int b, int c, int d );
void JE_barBright( SDL_Surface *surface, int a, int b, int c, int d );

static inline void fill_rectangle_hw( SDL_Surface *surface, int x, int y, uint h, uint w, Uint8 color )
{
	SDL_Rect rect = { x, y, h, w };
	SDL_FillRect(surface, &rect, color);
}

void draw_segmented_gauge( SDL_Surface *surface, int x, int y, Uint8 color, uint segment_width, uint segment_height, uint segment_value, uint value );

#endif /* VGA256D_H */

