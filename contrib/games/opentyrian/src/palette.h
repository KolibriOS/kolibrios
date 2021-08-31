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
#ifndef PALETTE_H
#define PALETTE_H

#include "opentyr.h"

#include "SDL.h"

typedef SDL_Color Palette[256];

extern Palette palettes[];
extern int palette_count;

extern Uint32 rgb_palette[256], yuv_palette[256];

extern Palette colors; // TODO: get rid of this

void JE_loadPals( void );

void set_palette( Palette colors, unsigned int first_color, unsigned int last_color );
void set_colors( SDL_Color color, unsigned int first_color, unsigned int last_color );

void init_step_fade_palette( int diff[256][3], Palette colors, unsigned int first_color, unsigned int last_color );
void init_step_fade_solid( int diff[256][3], SDL_Color color, unsigned int first_color, unsigned int last_color );
void step_fade_palette( int diff[256][3], int steps, unsigned int first_color, unsigned int last_color );

void fade_palette( Palette colors, int steps, unsigned int first_color, unsigned int last_color );
void fade_solid( SDL_Color color, int steps, unsigned int first_color, unsigned int last_color );

void fade_black( int steps );
void fade_white( int steps );

#endif /* PALETTE_H */

