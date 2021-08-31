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
#include "xmas.h"

#include "font.h"
#include "keyboard.h"
#include "palette.h"
#include "setup.h"
#include "sprite.h"
#include "video.h"

#include <stdio.h>
#include <time.h>

bool xmas = false;

bool xmas_time( void )
{
	time_t now = time(NULL);
	return localtime(&now)->tm_mon == 11;
}

bool xmas_prompt( void )
{
	const char *prompt[] =
	{
		"Christmas has been detected.",
		"Activate Christmas?",
	};
	const char *choice[] =
	{
		"Yes",
		"No",
	};
	
	set_palette(palettes[0], 0, 255);
	
	for (uint i = 0; i < COUNTOF(prompt); ++i)
		draw_font_hv(VGAScreen, 320 / 2, 85 + 15 * i, prompt[i], normal_font, centered, (i % 2) ? 2 : 4, -2);
	
	uint selection = 0;
	
	bool decided = false, quit = false;
	while (!decided)
	{
		for (uint i = 0; i < COUNTOF(choice); ++i)
			draw_font_hv(VGAScreen, 320 / 2 - 20 + 40 * i, 120, choice[i], normal_font, centered, 15, (selection == i) ? -2 : -4);
		
		JE_showVGA();
		
		JE_word temp = 0;
		JE_textMenuWait(&temp, false);
		
		if (newkey)
		{
			switch (lastkey_sym)
			{
				case SDLK_LEFT:
					if (selection == 0)
						selection = 2;
					selection--;
					break;
				case SDLK_RIGHT:
					selection++;
					selection %= 2;
					break;
					
				case SDLK_RETURN:
					decided = true;
					break;
				case SDLK_ESCAPE:
					decided = true;
					quit = true;
					break;
				default:
					break;
			}
		}
	}
	
	fade_black(10);
	
	return (selection == 0 && quit == false);
}
