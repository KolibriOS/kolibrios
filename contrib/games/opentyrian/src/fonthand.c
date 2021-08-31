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
#include "fonthand.h"

#include "network.h"
#include "nortsong.h"
#include "nortvars.h"
#include "opentyr.h"
#include "params.h"
#include "sprite.h"
#include "vga256d.h"
#include "video.h"

const int font_ascii[256] =
{
	 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	 -1,  26,  33,  60,  61,  62,  -1,  32,  64,  65,  63,  84,  29,  83,  28,  80, //  !"#$%&'()*+,-./
	 79,  70,  71,  72,  73,  74,  75,  76,  77,  78,  31,  30,  -1,  85,  -1,  27, // 0123456789:;<=>?
	 -1,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14, // @ABCDEFGHIJKLMNO
	 15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  68,  82,  69,  -1,  -1, // PQRSTUVWXYZ[\]^_
	 -1,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48, // `abcdefghijklmno
	 49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  66,  81,  67,  -1,  -1, // pqrstuvwxyz{|}~⌂

	 86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, // ÇüéâäàåçêëèïîìÄÅ
	102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, // ÉæÆôöòûùÿÖÜ¢£¥₧ƒ
	118, 119, 120, 121, 122, 123, 124, 125, 126,  -1,  -1,  -1,  -1,  -1,  -1,  -1, // áíóúñÑªº¿
	 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
};

/* shape constants included in newshape.h */

JE_byte textGlowFont, textGlowBrightness = 6;

JE_boolean levelWarningDisplay;
JE_byte levelWarningLines;
char levelWarningText[10][61]; /* [1..10] of string [60] */
JE_boolean warningRed;

JE_byte warningSoundDelay;
JE_word armorShipDelay;
JE_byte warningCol;
JE_shortint warningColChange;

void JE_dString( SDL_Surface * screen, int x, int y, const char *s, unsigned int font )
{
	const int defaultBrightness = -3;

	int bright = 0;

	for (int i = 0; s[i] != '\0'; ++i)
	{
		int sprite_id = font_ascii[(unsigned char)s[i]];

		switch (s[i])
		{
		case ' ':
			x += 6;
			break;

		case '~':
			bright = (bright == 0) ? 2 : 0;
			break;

		default:
			if (sprite_id != -1)
			{
				blit_sprite_dark(screen, x + 2, y + 2, font, sprite_id, false);
				blit_sprite_hv_unsafe(screen, x, y, font, sprite_id, 0xf, defaultBrightness + bright);

				x += sprite(font, sprite_id)->width + 1;
			}
			break;
		}
	}
}

int JE_fontCenter( const char *s, unsigned int font )
{
	return 160 - (JE_textWidth(s, font) / 2);
}

int JE_textWidth( const char *s, unsigned int font )
{
	int x = 0;

	for (int i = 0; s[i] != '\0'; ++i)
	{
		int sprite_id = font_ascii[(unsigned char)s[i]];

		if (s[i] == ' ')
			x += 6;
		else if (sprite_id != -1)
			x += sprite(font, sprite_id)->width + 1;
	}

	return x;
}

void JE_textShade( SDL_Surface * screen, int x, int y, const char *s, unsigned int colorbank, int brightness, unsigned int shadetype )
{
	switch (shadetype)
	{
		case PART_SHADE:
			JE_outText(screen, x+1, y+1, s, 0, -1);
			JE_outText(screen, x, y, s, colorbank, brightness);
			break;
		case FULL_SHADE:
			JE_outText(screen, x-1, y, s, 0, -1);
			JE_outText(screen, x+1, y, s, 0, -1);
			JE_outText(screen, x, y-1, s, 0, -1);
			JE_outText(screen, x, y+1, s, 0, -1);
			JE_outText(screen, x, y, s, colorbank, brightness);
			break;
		case DARKEN:
			JE_outTextAndDarken(screen, x+1, y+1, s, colorbank, brightness, TINY_FONT);
			break;
		case TRICK:
			JE_outTextModify(screen, x, y, s, colorbank, brightness, TINY_FONT);
			break;
	}
}

void JE_outText( SDL_Surface * screen, int x, int y, const char *s, unsigned int colorbank, int brightness )
{
	int bright = 0;

	for (int i = 0; s[i] != '\0'; ++i)
	{
		int sprite_id = font_ascii[(unsigned char)s[i]];

		switch (s[i])
		{
		case ' ':
			x += 6;
			break;

		case '~':
			bright = (bright == 0) ? 4 : 0;
			break;

		default:
			if (sprite_id != -1 && sprite_exists(TINY_FONT, sprite_id))
			{
				if (brightness >= 0)
					blit_sprite_hv_unsafe(screen, x, y, TINY_FONT, sprite_id, colorbank, brightness + bright);
				else
					blit_sprite_dark(screen, x, y, TINY_FONT, sprite_id, true);

				x += sprite(TINY_FONT, sprite_id)->width + 1;
			}
			break;
		}
	}
}

void JE_outTextModify( SDL_Surface * screen, int x, int y, const char *s, unsigned int filter, unsigned int brightness, unsigned int font )
{
	for (int i = 0; s[i] != '\0'; ++i)
	{
		int sprite_id = font_ascii[(unsigned char)s[i]];

		if (s[i] == ' ')
		{
			x += 6;
		}
		else if (sprite_id != -1)
		{
			blit_sprite_hv_blend(screen, x, y, font, sprite_id, filter, brightness);

			x += sprite(font, sprite_id)->width + 1;
		}
	}
}

void JE_outTextAdjust( SDL_Surface * screen, int x, int y, const char *s, unsigned int filter, int brightness, unsigned int font, JE_boolean shadow )
{
	int bright = 0;

	for (int i = 0; s[i] != '\0'; ++i)
	{
		int sprite_id = font_ascii[(unsigned char)s[i]];

		switch (s[i])
		{
		case ' ':
			x += 6;
			break;

		case '~':
			bright = (bright == 0) ? 4 : 0;
			break;

		default:
			if (sprite_id != -1 && sprite_exists(TINY_FONT, sprite_id))
			{
				if (shadow)
					blit_sprite_dark(screen, x + 2, y + 2, font, sprite_id, false);
				blit_sprite_hv(screen, x, y, font, sprite_id, filter, brightness + bright);

				x += sprite(font, sprite_id)->width + 1;
			}
			break;
		}
	}
}

void JE_outTextAndDarken( SDL_Surface * screen, int x, int y, const char *s, unsigned int colorbank, unsigned int brightness, unsigned int font )
{
	int bright = 0;

	for (int i = 0; s[i] != '\0'; ++i)
	{
		int sprite_id = font_ascii[(unsigned char)s[i]];

		switch (s[i])
		{
		case ' ':
			x += 6;
			break;

		case '~':
			bright = (bright == 0) ? 4 : 0;
			break;

		default:
			if (sprite_id != -1 && sprite_exists(TINY_FONT, sprite_id))
			{
				blit_sprite_dark(screen, x + 1, y + 1, font, sprite_id, false);
				blit_sprite_hv_unsafe(screen, x, y, font, sprite_id, colorbank, brightness + bright);

				x += sprite(font, sprite_id)->width + 1;
			}
			break;
		}
	}
}

void JE_updateWarning( SDL_Surface * screen )
{
	if (delaycount2() == 0)
	{ /*Update Color Bars*/

		warningCol += warningColChange;
		if (warningCol > 14 * 16 + 10 || warningCol < 14 * 16 + 4)
		{
			warningColChange = -warningColChange;
		}
		fill_rectangle_xy(screen, 0, 0, 319, 5, warningCol);
		fill_rectangle_xy(screen, 0, 194, 319, 199, warningCol);
		JE_showVGA();

		setjasondelay2(6);

		if (warningSoundDelay > 0)
		{
			warningSoundDelay--;
		}
		else
		{
			warningSoundDelay = 14;
			JE_playSampleNum(S_WARNING);
		}
	}
}

void JE_outTextGlow( SDL_Surface * screen, int x, int y, const char *s )
{
	JE_integer z;
	JE_byte c = 15;

	if (warningRed)
	{
		c = 7;
	}

	JE_outTextAdjust(screen, x - 1, y,     s, 0, -12, textGlowFont, false);
	JE_outTextAdjust(screen, x,     y - 1, s, 0, -12, textGlowFont, false);
	JE_outTextAdjust(screen, x + 1, y,     s, 0, -12, textGlowFont, false);
	JE_outTextAdjust(screen, x,     y + 1, s, 0, -12, textGlowFont, false);
	if (frameCountMax > 0)
		for (z = 1; z <= 12; z++)
		{
			setjasondelay(frameCountMax);
			JE_outTextAdjust(screen, x, y, s, c, z - 10, textGlowFont, false);
			if (JE_anyButton())
			{
				frameCountMax = 0;
			}

			NETWORK_KEEP_ALIVE();

			JE_showVGA();

			wait_delay();
		}
	for (z = (frameCountMax == 0) ? 6 : 12; z >= textGlowBrightness; z--)
	{
		setjasondelay(frameCountMax);
		JE_outTextAdjust(screen, x, y, s, c, z - 10, textGlowFont, false);
		if (JE_anyButton())
		{
			frameCountMax = 0;
		}

		NETWORK_KEEP_ALIVE();

		JE_showVGA();

		wait_delay();
	}
	textGlowBrightness = 6;
}

