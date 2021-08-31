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
#ifndef SPRITE_H
#define SPRITE_H

#include "opentyr.h"

#include "SDL.h"

#include <assert.h>
#include <stdio.h>

#define FONT_SHAPES       0
#define SMALL_FONT_SHAPES 1
#define TINY_FONT         2
#define PLANET_SHAPES     3
#define FACE_SHAPES       4
#define OPTION_SHAPES     5 /*Also contains help shapes*/
#define WEAPON_SHAPES     6
#define EXTRA_SHAPES      7 /*Used for Ending pics*/

#define SPRITE_TABLES_MAX        8
#define SPRITES_PER_TABLE_MAX  151

typedef struct
{
	Uint16 width, height;
	Uint16 size;
	Uint8 *data;
}
Sprite;

typedef struct
{
	unsigned int count;
	Sprite sprite[SPRITES_PER_TABLE_MAX];
}
Sprite_array;

extern Sprite_array sprite_table[SPRITE_TABLES_MAX];

static inline Sprite *sprite( unsigned int table, unsigned int index )
{
	assert(table < COUNTOF(sprite_table));
	assert(index < COUNTOF(sprite_table->sprite));
	return &sprite_table[table].sprite[index];
}

static inline bool sprite_exists( unsigned int table, unsigned int index )
{
	return (sprite(table, index)->data != NULL);
}
static inline Uint16 get_sprite_width( unsigned int table, unsigned int index )
{
	return (sprite_exists(table, index) ? sprite(table, index)->width : 0);
}
static inline Uint16 get_sprite_height( unsigned int table, unsigned int index )
{
	return (sprite_exists(table, index) ? sprite(table, index)->height : 0);
}

void load_sprites_file( unsigned table, const char *filename );
void load_sprites( unsigned int table, FILE *f );
void free_sprites( unsigned int table );

void blit_sprite( SDL_Surface *, int x, int y, unsigned int table, unsigned int index ); // JE_newDrawCShapeNum
void blit_sprite_blend( SDL_Surface *, int x, int y, unsigned int table, unsigned int index ); // JE_newDrawCShapeTrick
void blit_sprite_hv_unsafe( SDL_Surface *, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeBright
void blit_sprite_hv( SDL_Surface *, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeAdjust
void blit_sprite_hv_blend( SDL_Surface *, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value ); // JE_newDrawCShapeModify
void blit_sprite_dark( SDL_Surface *, int x, int y, unsigned int table, unsigned int index, bool black ); // JE_newDrawCShapeDarken, JE_newDrawCShapeShadow

typedef struct
{
	unsigned int size;
	Uint8 *data;
}
Sprite2_array;

extern Sprite2_array eShapes[6];
extern Sprite2_array shapesC1, shapes6, shapes9, shapesW2;

void JE_loadCompShapes( Sprite2_array *, JE_char s );
void JE_loadCompShapesB( Sprite2_array *, FILE *f );
void free_sprite2s( Sprite2_array * );

void blit_sprite2( SDL_Surface *, int x, int y, Sprite2_array, unsigned int index );
void blit_sprite2_blend( SDL_Surface *,  int x, int y, Sprite2_array, unsigned int index );
void blit_sprite2_darken( SDL_Surface *, int x, int y, Sprite2_array, unsigned int index );
void blit_sprite2_filter( SDL_Surface *, int x, int y, Sprite2_array, unsigned int index, Uint8 filter );

void blit_sprite2x2( SDL_Surface *, int x, int y, Sprite2_array, unsigned int index );
void blit_sprite2x2_blend( SDL_Surface *, int x, int y, Sprite2_array, unsigned int index );
void blit_sprite2x2_darken( SDL_Surface *, int x, int y, Sprite2_array, unsigned int index );

void JE_loadMainShapeTables( const char *shpfile );
void free_main_shape_tables( void );

#endif // SPRITE_H

