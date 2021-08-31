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
#include "font.h"

#include "fonthand.h"
#include "sprite.h"

/**
 * \file font.c
 * \brief Text drawing routines.
 */

/**
 * \brief Draws text in a color specified by hue and value and with a drop
 *        shadow.
 * 
 * A '~' in the text is not drawn but instead toggles highlighting which
 * increases \c value by 4.
 * 
 * \li like JE_dString()                    if (black == false && shadow_dist == 2 && hue == 15)
 * \li like JE_textShade() with PART_SHADE  if (black == true && shadow_dist == 1)
 * \li like JE_outTextAndDarken()           if (black == false && shadow_dist == 1)
 * \li like JE_outTextAdjust() with shadow  if (black == false && shadow_dist == 2)
 * 
 * @param surface destination surface
 * @param x initial x-position in pixels; which direction(s) the text is drawn
 *        from this position depends on the alignment
 * @param y initial upper y-position in pixels
 * @param text text to be drawn
 * @param font style/size of text
 * @param alignment left_aligned, centered, or right_aligned
 * @param hue hue component of text color
 * @param value value component of text color
 * @param black if true the shadow is drawn as solid black, if false the shadow
 *        is drawn by darkening the pixels of the destination surface
 * @param shadow_dist distance in pixels that the shadow will be drawn away from
 *        the text. (This is added to both the x and y positions, so a value of
 *        1 causes the shadow to be drawn 1 pixel right and 1 pixel lower than
 *        the text.)
 */
void draw_font_hv_shadow( SDL_Surface *surface, int x, int y, const char *text, Font font, FontAlignment alignment, Uint8 hue, Sint8 value, bool black, int shadow_dist )
{
	draw_font_dark(surface, x + shadow_dist, y + shadow_dist, text, font, alignment, black);
	
	draw_font_hv(surface, x, y, text, font, alignment, hue, value);
}

/**
 * \brief Draws text in a color specified by hue and value and with a
 *        surrounding shadow.
 * 
 * A '~' in the text is not drawn but instead toggles highlighting which
 * increases \c value by 4.
 * 
 * \li like JE_textShade() with FULL_SHADE  if (black == true && shadow_dist == 1)
 * 
 * @param surface destination surface
 * @param x initial x-position in pixels; which direction(s) the text is drawn
 *        from this position depends on the alignment
 * @param y initial upper y-position in pixels
 * @param text text to be drawn
 * @param font style/size of text
 * @param alignment left_aligned, centered, or right_aligned
 * @param hue hue component of text color
 * @param value value component of text color
 * @param black if true the shadow is drawn as solid black, if false the shadow
 *        is drawn by darkening the pixels of the destination surface
 * @param shadow_dist distance in pixels that the shadows will be drawn away
 *        from the text. (This distance is separately added to and subtracted
 *        from the x position and y position, resulting in four shadows -- one
 *        in each cardinal direction.  If this shadow distance is small enough,
 *        this produces a shadow that outlines the text.)
 */
void draw_font_hv_full_shadow( SDL_Surface *surface, int x, int y, const char *text, Font font, FontAlignment alignment, Uint8 hue, Sint8 value, bool black, int shadow_dist )
{
	draw_font_dark(surface, x,               y - shadow_dist, text, font, alignment, black);
	draw_font_dark(surface, x + shadow_dist, y,               text, font, alignment, black);
	draw_font_dark(surface, x,               y + shadow_dist, text, font, alignment, black);
	draw_font_dark(surface, x - shadow_dist, y,               text, font, alignment, black);
	
	draw_font_hv(surface, x, y, text, font, alignment, hue, value);
}

/**
 * \brief Draws text in a color specified by hue and value.
 * 
 * A '~' in the text is not drawn but instead toggles highlighting which
 * increases \c value by 4.
 * 
 * \li like JE_outText() with (brightness >= 0)
 * \li like JE_outTextAdjust() without shadow
 * 
 * @param surface destination surface
 * @param x initial x-position in pixels; which direction(s) the text is drawn
 *        from this position depends on the alignment
 * @param y initial upper y-position in pixels
 * @param text text to be drawn
 * @param font style/size of text
 * @param alignment left_aligned, centered, or right_aligned
 * @param hue hue component of text color
 * @param value value component of text color
 */
void draw_font_hv( SDL_Surface *surface, int x, int y, const char *text, Font font, FontAlignment alignment, Uint8 hue, Sint8 value )
{
	switch (alignment)
	{
	case left_aligned:
		break;
	case centered:
		x -= JE_textWidth(text, font) / 2;
		break;
	case right_aligned:
		x -= JE_textWidth(text, font);
		break;
	}
	
	bool highlight = false;
	
	for (; *text != '\0'; ++text)
	{
		int sprite_id = font_ascii[(unsigned char)*text];
		
		switch (*text)
		{
		case ' ':
			x += 6;
			break;
			
		case '~':
			highlight = !highlight;
			if (highlight)
				value += 4;
			else
				value -= 4;
			break;
			
		default:
			if (sprite_id != -1 && sprite_exists(font, sprite_id))
			{
				blit_sprite_hv(surface, x, y, font, sprite_id, hue, value);
				
				x += sprite(font, sprite_id)->width + 1;
			}
			break;
		}
	}
}

/**
 * \brief Draws blended text in a color specified by hue and value.
 * 
 * Corresponds to blit_sprite_hv_blend()
 * 
 * \li like JE_outTextModify()
 * 
 * @param surface destination surface
 * @param x initial x-position in pixels; which direction(s) the text is drawn
 *        from this position depends on the alignment
 * @param y initial upper y-position in pixels
 * @param text text to be drawn
 * @param font style/size of text
 * @param alignment left_aligned, centered, or right_aligned
 * @param hue hue component of text color
 * @param value value component of text color
 */
void draw_font_hv_blend( SDL_Surface *surface, int x, int y, const char *text, Font font, FontAlignment alignment, Uint8 hue, Sint8 value )
{
	switch (alignment)
	{
	case left_aligned:
		break;
	case centered:
		x -= JE_textWidth(text, font) / 2;
		break;
	case right_aligned:
		x -= JE_textWidth(text, font);
		break;
	}
	
	for (; *text != '\0'; ++text)
	{
		int sprite_id = font_ascii[(unsigned char)*text];
		
		switch (*text)
		{
		case ' ':
			x += 6;
			break;
			
		case '~':
			break;
			
		default:
			if (sprite_id != -1 && sprite_exists(font, sprite_id))
			{
				blit_sprite_hv_blend(surface, x, y, font, sprite_id, hue, value);
				
				x += sprite(font, sprite_id)->width + 1;
			}
			break;
		}
	}
}

/**
 * \brief Draws darkened text.
 * 
 * Corresponds to blit_sprite_dark()
 * 
 * \li like JE_outText() with (brightness < 0)  if (black == true)
 * 
 * @param surface destination surface
 * @param x initial x-position in pixels; which direction(s) the text is drawn
 *        from this position depends on the alignment
 * @param y initial upper y-position in pixels
 * @param text text to be drawn
 * @param font style/size of text
 * @param alignment left_aligned, centered, or right_aligned
 * @param black if true text is drawn as solid black, if false text is drawn by
 *        darkening the pixels of the destination surface
 */
void draw_font_dark( SDL_Surface *surface, int x, int y, const char *text, Font font, FontAlignment alignment, bool black )
{
	switch (alignment)
	{
	case left_aligned:
		break;
	case centered:
		x -= JE_textWidth(text, font) / 2;
		break;
	case right_aligned:
		x -= JE_textWidth(text, font);
		break;
	}
	
	for (; *text != '\0'; ++text)
	{
		int sprite_id = font_ascii[(unsigned char)*text];
		
		switch (*text)
		{
		case ' ':
			x += 6;
			break;
			
		case '~':
			break;
			
		default:
			if (sprite_id != -1 && sprite_exists(font, sprite_id))
			{
				blit_sprite_dark(surface, x, y, font, sprite_id, black);
				
				x += sprite(font, sprite_id)->width + 1;
			}
			break;
		}
	}
}
