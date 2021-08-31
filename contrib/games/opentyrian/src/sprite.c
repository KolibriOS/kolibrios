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
#include "sprite.h"

#include "file.h"
#include "opentyr.h"
#include "video.h"

#include <assert.h>
#include <ctype.h>

Sprite_array sprite_table[SPRITE_TABLES_MAX];

Sprite2_array eShapes[6];
Sprite2_array shapesC1, shapes6, shapes9, shapesW2;

void load_sprites_file( unsigned int table, const char *filename )
{
	free_sprites(table);
	
	FILE *f = dir_fopen_die(data_dir(), filename, "rb");
	
	load_sprites(table, f);
	
	fclose(f);
}

void load_sprites( unsigned int table, FILE *f )
{
	free_sprites(table);
	
	Uint16 temp;
	efread(&temp, sizeof(Uint16), 1, f);
	
	sprite_table[table].count = temp;
	
	assert(sprite_table[table].count <= SPRITES_PER_TABLE_MAX);
	
	for (unsigned int i = 0; i < sprite_table[table].count; ++i)
	{
		Sprite * const cur_sprite = sprite(table, i);
		
		if (!getc(f)) // sprite is empty
			continue;
		
		efread(&cur_sprite->width,  sizeof(Uint16), 1, f);
		efread(&cur_sprite->height, sizeof(Uint16), 1, f);
		efread(&cur_sprite->size,   sizeof(Uint16), 1, f);
		
		cur_sprite->data = malloc(cur_sprite->size);
		
		efread(cur_sprite->data, sizeof(Uint8), cur_sprite->size, f);
	}
}

void free_sprites( unsigned int table )
{
	for (unsigned int i = 0; i < sprite_table[table].count; ++i)
	{
		Sprite * const cur_sprite = sprite(table, i);
		
		cur_sprite->width  = 0;
		cur_sprite->height = 0;
		cur_sprite->size   = 0;
		
		free(cur_sprite->data);
		cur_sprite->data = NULL;
	}
	
	sprite_table[table].count = 0;
}

// does not clip on left or right edges of surface
void blit_sprite( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	const Uint8 *data = cur_sprite->data;
	const Uint8 * const data_ul = data + cur_sprite->size;
	
	const unsigned int width = cur_sprite->width;
	unsigned int x_offset = 0;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *             pixels =    (Uint8 *)surface->pixels + (y * surface->pitch) + x;
	const Uint8 * const pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	            * const pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (; data < data_ul; ++data)
	{
		switch (*data)
		{
		case 255:  // transparent pixels
			data++;  // next byte tells how many
			pixels += *data;
			x_offset += *data;
			break;
			
		case 254:  // next pixel row
			pixels += width - x_offset;
			x_offset = width;
			break;
			
		case 253:  // 1 transparent pixel
			pixels++;
			x_offset++;
			break;
			
		default:  // set a pixel
			if (pixels >= pixels_ul)
				return;
			if (pixels >= pixels_ll)
				*pixels = *data;
			
			pixels++;
			x_offset++;
			break;
		}
		if (x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
		}
	}
}

// does not clip on left or right edges of surface
void blit_sprite_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	const Uint8 *data = cur_sprite->data;
	const Uint8 * const data_ul = data + cur_sprite->size;
	
	const unsigned int width = cur_sprite->width;
	unsigned int x_offset = 0;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *             pixels =    (Uint8 *)surface->pixels + (y * surface->pitch) + x;
	const Uint8 * const pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	            * const pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (; data < data_ul; ++data)
	{
		switch (*data)
		{
		case 255:  // transparent pixels
			data++;  // next byte tells how many
			pixels += *data;
			x_offset += *data;
			break;
			
		case 254:  // next pixel row
			pixels += width - x_offset;
			x_offset = width;
			break;
			
		case 253:  // 1 transparent pixel
			pixels++;
			x_offset++;
			break;
			
		default:  // set a pixel
			if (pixels >= pixels_ul)
				return;
			if (pixels >= pixels_ll)
				*pixels = (*data & 0xf0) | (((*pixels & 0x0f) + (*data & 0x0f)) / 2);
			
			pixels++;
			x_offset++;
			break;
		}
		if (x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
		}
	}
}

// does not clip on left or right edges of surface
// unsafe because it doesn't check that value won't overflow into hue
// we can replace it when we know that we don't rely on that 'feature'
void blit_sprite_hv_unsafe( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	hue <<= 4;
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	const Uint8 *data = cur_sprite->data;
	const Uint8 * const data_ul = data + cur_sprite->size;
	
	const unsigned int width = cur_sprite->width;
	unsigned int x_offset = 0;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *             pixels =    (Uint8 *)surface->pixels + (y * surface->pitch) + x;
	const Uint8 * const pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	            * const pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (; data < data_ul; ++data)
	{
		switch (*data)
		{
		case 255:  // transparent pixels
			data++;  // next byte tells how many
			pixels += *data;
			x_offset += *data;
			break;
			
		case 254:  // next pixel row
			pixels += width - x_offset;
			x_offset = width;
			break;
			
		case 253:  // 1 transparent pixel
			pixels++;
			x_offset++;
			break;
			
		default:  // set a pixel
			if (pixels >= pixels_ul)
				return;
			if (pixels >= pixels_ll)
				*pixels = hue | ((*data & 0x0f) + value);
			
			pixels++;
			x_offset++;
			break;
		}
		if (x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
		}
	}
}

// does not clip on left or right edges of surface
void blit_sprite_hv( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	hue <<= 4;
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	const Uint8 *data = cur_sprite->data;
	const Uint8 * const data_ul = data + cur_sprite->size;
	
	const unsigned int width = cur_sprite->width;
	unsigned int x_offset = 0;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *             pixels =    (Uint8 *)surface->pixels + (y * surface->pitch) + x;
	const Uint8 * const pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	            * const pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (; data < data_ul; ++data)
	{
		switch (*data)
		{
		case 255:  // transparent pixels
			data++;  // next byte tells how many
			pixels += *data;
			x_offset += *data;
			break;
			
		case 254:  // next pixel row
			pixels += width - x_offset;
			x_offset = width;
			break;
			
		case 253:  // 1 transparent pixel
			pixels++;
			x_offset++;
			break;
			
		default:  // set a pixel
			if (pixels >= pixels_ul)
				return;
			if (pixels >= pixels_ll)
			{
				Uint8 temp_value = (*data & 0x0f) + value;
				if (temp_value > 0xf)
					temp_value = (temp_value >= 0x1f) ? 0x0 : 0xf;
				
				*pixels = hue | temp_value;
			}
			
			pixels++;
			x_offset++;
			break;
		}
		if (x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
		}
	}
}

// does not clip on left or right edges of surface
void blit_sprite_hv_blend( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, Uint8 hue, Sint8 value )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	hue <<= 4;
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	const Uint8 *data = cur_sprite->data;
	const Uint8 * const data_ul = data + cur_sprite->size;
	
	const unsigned int width = cur_sprite->width;
	unsigned int x_offset = 0;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *             pixels =    (Uint8 *)surface->pixels + (y * surface->pitch) + x;
	const Uint8 * const pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	            * const pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (; data < data_ul; ++data)
	{
		switch (*data)
		{
		case 255:  // transparent pixels
			data++;  // next byte tells how many
			pixels += *data;
			x_offset += *data;
			break;
			
		case 254:  // next pixel row
			pixels += width - x_offset;
			x_offset = width;
			break;
			
		case 253:  // 1 transparent pixel
			pixels++;
			x_offset++;
			break;
			
		default:  // set a pixel
			if (pixels >= pixels_ul)
				return;
			if (pixels >= pixels_ll)
			{
				Uint8 temp_value = (*data & 0x0f) + value;
				if (temp_value > 0xf)
					temp_value = (temp_value >= 0x1f) ? 0x0 : 0xf;
				
				*pixels = hue | (((*pixels & 0x0f) + temp_value) / 2);
			}
			
			pixels++;
			x_offset++;
			break;
		}
		if (x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
		}
	}
}

// does not clip on left or right edges of surface
void blit_sprite_dark( SDL_Surface *surface, int x, int y, unsigned int table, unsigned int index, bool black )
{
	if (index >= sprite_table[table].count || !sprite_exists(table, index))
	{
		assert(false);
		return;
	}
	
	const Sprite * const cur_sprite = sprite(table, index);
	
	const Uint8 *data = cur_sprite->data;
	const Uint8 * const data_ul = data + cur_sprite->size;
	
	const unsigned int width = cur_sprite->width;
	unsigned int x_offset = 0;
	
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *             pixels =    (Uint8 *)surface->pixels + (y * surface->pitch) + x;
	const Uint8 * const pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	            * const pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	for (; data < data_ul; ++data)
	{
		switch (*data)
		{
		case 255:  // transparent pixels
			data++;  // next byte tells how many
			pixels += *data;
			x_offset += *data;
			break;
			
		case 254:  // next pixel row
			pixels += width - x_offset;
			x_offset = width;
			break;
			
		case 253:  // 1 transparent pixel
			pixels++;
			x_offset++;
			break;
			
		default:  // set a pixel
			if (pixels >= pixels_ul)
				return;
			if (pixels >= pixels_ll)
				*pixels = black ? 0x00 : ((*pixels & 0xf0) | ((*pixels & 0x0f) / 2));
			
			pixels++;
			x_offset++;
			break;
		}
		if (x_offset >= width)
		{
			pixels += surface->pitch - x_offset;
			x_offset = 0;
		}
	}
}


void JE_loadCompShapes( Sprite2_array *sprite2s, char s )
{
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "newsh%c.shp", tolower((unsigned char)s));
	
	FILE *f = dir_fopen_die(data_dir(), buffer, "rb");
	
	sprite2s->size = ftell_eof(f);
	
	JE_loadCompShapesB(sprite2s, f);
	
	fclose(f);
}

void JE_loadCompShapesB( Sprite2_array *sprite2s, FILE *f )
{
	free_sprite2s(sprite2s);
	
	sprite2s->data = malloc(sizeof(Uint8) * sprite2s->size);
	efread(sprite2s->data, sizeof(Uint8), sprite2s->size, f);
}

void free_sprite2s( Sprite2_array *sprite2s )
{
	free(sprite2s->data);
	sprite2s->data = NULL;
}

// does not clip on left or right edges of surface
void blit_sprite2( SDL_Surface *surface, int x, int y, Sprite2_array sprite2s, unsigned int index )
{
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *             pixels =    (Uint8 *)surface->pixels + (y * surface->pitch) + x;
	const Uint8 * const pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	            * const pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	const Uint8 *data = sprite2s.data + SDL_SwapLE16(((Uint16 *)sprite2s.data)[index - 1]);
	
	for (; *data != 0x0f; ++data)
	{
		pixels += *data & 0x0f;                   // second nibble: transparent pixel count
		unsigned int count = (*data & 0xf0) >> 4; // first nibble: opaque pixel count
		
		if (count == 0) // move to next pixel row
		{
			pixels += VGAScreen->pitch - 12;
		}
		else
		{
			while (count--)
			{
				++data;
				
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll)
					*pixels = *data;
				
				++pixels;
			}
		}
	}
}

// does not clip on left or right edges of surface
void blit_sprite2_blend( SDL_Surface *surface,  int x, int y, Sprite2_array sprite2s, unsigned int index )
{
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *             pixels =    (Uint8 *)surface->pixels + (y * surface->pitch) + x;
	const Uint8 * const pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	            * const pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	const Uint8 *data = sprite2s.data + SDL_SwapLE16(((Uint16 *)sprite2s.data)[index - 1]);
	
	for (; *data != 0x0f; ++data)
	{
		pixels += *data & 0x0f;                   // second nibble: transparent pixel count
		unsigned int count = (*data & 0xf0) >> 4; // first nibble: opaque pixel count
		
		if (count == 0) // move to next pixel row
		{
			pixels += VGAScreen->pitch - 12;
		}
		else
		{
			while (count--)
			{
				++data;
				
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll)
					*pixels = (((*data & 0x0f) + (*pixels & 0x0f)) / 2) | (*data & 0xf0);
				
				++pixels;
			}
		}
	}
}

// does not clip on left or right edges of surface
void blit_sprite2_darken( SDL_Surface *surface, int x, int y, Sprite2_array sprite2s, unsigned int index )
{
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *             pixels =    (Uint8 *)surface->pixels + (y * surface->pitch) + x;
	const Uint8 * const pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	            * const pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	const Uint8 *data = sprite2s.data + SDL_SwapLE16(((Uint16 *)sprite2s.data)[index - 1]);
	
	for (; *data != 0x0f; ++data)
	{
		pixels += *data & 0x0f;                   // second nibble: transparent pixel count
		unsigned int count = (*data & 0xf0) >> 4; // first nibble: opaque pixel count
		
		if (count == 0) // move to next pixel row
		{
			pixels += VGAScreen->pitch - 12;
		}
		else
		{
			while (count--)
			{
				++data;
				
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll)
					*pixels = ((*pixels & 0x0f) / 2) + (*pixels & 0xf0);
				
				++pixels;
			}
		}
	}
}

// does not clip on left or right edges of surface
void blit_sprite2_filter( SDL_Surface *surface, int x, int y, Sprite2_array sprite2s, unsigned int index, Uint8 filter )
{
	assert(surface->format->BitsPerPixel == 8);
	Uint8 *             pixels =    (Uint8 *)surface->pixels + (y * surface->pitch) + x;
	const Uint8 * const pixels_ll = (Uint8 *)surface->pixels,  // lower limit
	            * const pixels_ul = (Uint8 *)surface->pixels + (surface->h * surface->pitch);  // upper limit
	
	const Uint8 *data = sprite2s.data + SDL_SwapLE16(((Uint16 *)sprite2s.data)[index - 1]);
	
	for (; *data != 0x0f; ++data)
	{
		pixels += *data & 0x0f;                   // second nibble: transparent pixel count
		unsigned int count = (*data & 0xf0) >> 4; // first nibble: opaque pixel count
		
		if (count == 0) // move to next pixel row
		{
			pixels += VGAScreen->pitch - 12;
		}
		else
		{
			while (count--)
			{
				++data;
				
				if (pixels >= pixels_ul)
					return;
				if (pixels >= pixels_ll)
					*pixels = filter | (*data & 0x0f);
				
				++pixels;
			}
		}
	}
}

// does not clip on left or right edges of surface
void blit_sprite2x2( SDL_Surface *surface, int x, int y, Sprite2_array sprite2s, unsigned int index )
{
	blit_sprite2(surface, x,      y,      sprite2s, index);
	blit_sprite2(surface, x + 12, y,      sprite2s, index + 1);
	blit_sprite2(surface, x,      y + 14, sprite2s, index + 19);
	blit_sprite2(surface, x + 12, y + 14, sprite2s, index + 20);
}

// does not clip on left or right edges of surface
void blit_sprite2x2_blend( SDL_Surface *surface, int x, int y, Sprite2_array sprite2s, unsigned int index )
{
	blit_sprite2_blend(surface, x,      y,      sprite2s, index);
	blit_sprite2_blend(surface, x + 12, y,      sprite2s, index + 1);
	blit_sprite2_blend(surface, x,      y + 14, sprite2s, index + 19);
	blit_sprite2_blend(surface, x + 12, y + 14, sprite2s, index + 20);
}

// does not clip on left or right edges of surface
void blit_sprite2x2_darken( SDL_Surface *surface, int x, int y, Sprite2_array sprite2s, unsigned int index )
{
	blit_sprite2_darken(surface, x,      y,      sprite2s, index);
	blit_sprite2_darken(surface, x + 12, y,      sprite2s, index + 1);
	blit_sprite2_darken(surface, x,      y + 14, sprite2s, index + 19);
	blit_sprite2_darken(surface, x + 12, y + 14, sprite2s, index + 20);
}


void JE_loadMainShapeTables( const char *shpfile )
{
	enum { SHP_NUM = 12 };
	
	FILE *f = dir_fopen_die(data_dir(), shpfile, "rb");
	
	JE_word shpNumb;
	JE_longint shpPos[SHP_NUM + 1]; // +1 for storing file length
	
	efread(&shpNumb, sizeof(JE_word), 1, f);
	assert(shpNumb + 1u == COUNTOF(shpPos));
	
	for (unsigned int i = 0; i < shpNumb; ++i)
		efread(&shpPos[i], sizeof(JE_longint), 1, f);
	
	fseek(f, 0, SEEK_END);
	for (unsigned int i = shpNumb; i < COUNTOF(shpPos); ++i)
		shpPos[i] = ftell(f);
	
	int i;
	// fonts, interface, option sprites
	for (i = 0; i < 7; i++)
	{
		fseek(f, shpPos[i], SEEK_SET);
		load_sprites(i, f);
	}
	
	// player shot sprites
	shapesC1.size = shpPos[i + 1] - shpPos[i];
	JE_loadCompShapesB(&shapesC1, f);
	i++;
	
	// player ship sprites
	shapes9.size = shpPos[i + 1] - shpPos[i];
	JE_loadCompShapesB(&shapes9 , f);
	i++;
	
	// power-up sprites
	eShapes[5].size = shpPos[i + 1] - shpPos[i];
	JE_loadCompShapesB(&eShapes[5], f);
	i++;
	
	// coins, datacubes, etc sprites
	eShapes[4].size = shpPos[i + 1] - shpPos[i];
	JE_loadCompShapesB(&eShapes[4], f);
	i++;
	
	// more player shot sprites
	shapesW2.size = shpPos[i + 1] - shpPos[i];
	JE_loadCompShapesB(&shapesW2, f);
	
	fclose(f);
}

void free_main_shape_tables( void )
{
	for (uint i = 0; i < COUNTOF(sprite_table); ++i)
		free_sprites(i);
	
	free_sprite2s(&shapesC1);
	free_sprite2s(&shapes9);
	free_sprite2s(&eShapes[5]);
	free_sprite2s(&eShapes[4]);
	free_sprite2s(&shapesW2);
}
