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
#ifndef BACKGRND_H
#define BACKGRND_H

#include "opentyr.h"

#include "SDL.h"

#include <stdint.h>

extern JE_word backPos, backPos2, backPos3;
extern JE_word backMove, backMove2, backMove3;
extern JE_word mapX, mapY, mapX2, mapX3, mapY2, mapY3;
extern JE_byte **mapYPos, **mapY2Pos, **mapY3Pos;
extern JE_word mapXPos, oldMapXOfs, mapXOfs, mapX2Ofs, mapX2Pos, mapX3Pos, oldMapX3Ofs, mapX3Ofs, tempMapXOfs;
extern intptr_t mapXbpPos, mapX2bpPos, mapX3bpPos;
extern JE_byte map1YDelay, map1YDelayMax, map2YDelay, map2YDelayMax;
extern JE_boolean anySmoothies;  // if yes, I want one :D
extern JE_byte smoothie_data[9];

extern int starfield_speed;

void JE_darkenBackground( JE_word neat );

void blit_background_row( SDL_Surface *surface, int x, int y, Uint8 **map );
void blit_background_row_blend( SDL_Surface *surface, int x, int y, Uint8 **map );

void draw_background_1( SDL_Surface *surface );
void draw_background_2( SDL_Surface *surface );
void draw_background_2_blend( SDL_Surface *surface );
void draw_background_3( SDL_Surface *surface );

void JE_filterScreen( JE_shortint col, JE_shortint generic_int );

void JE_checkSmoothies( void );
void lava_filter( SDL_Surface *dst, SDL_Surface *src );
void water_filter( SDL_Surface *dst, SDL_Surface *src );
void iced_blur_filter( SDL_Surface *dst, SDL_Surface *src );
void blur_filter( SDL_Surface *dst, SDL_Surface *src );
/*smoothies #5 is used for 3*/
/*smoothies #9 is a vertical flip*/

void initialize_starfield( void );
void update_and_draw_starfield( SDL_Surface* surface, int move_speed );

#endif /* BACKGRND_H */

