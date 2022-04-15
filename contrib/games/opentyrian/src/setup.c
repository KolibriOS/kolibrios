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
#include "setup.h"

#include "joystick.h"
#include "keyboard.h"
#include "network.h"
#include "nortvars.h"
#include "opentyr.h"
#include "mainint.h"
#include "mouse.h"
#include "video.h"

#include "SDL.h"

void JE_textMenuWait( JE_word *waitTime, JE_boolean doGamma )
{
	set_mouse_position(160, 100);
	
	do
	{
		JE_showVGA();
		
		push_joysticks_as_keyboard();
		service_SDL_events(true);
		
		if (doGamma)
			JE_gammaCheck();
		
		inputDetected = newkey | mousedown;
		
		if (lastkey_sym == SDLK_SPACE)
		{
			lastkey_sym = SDLK_RETURN;
		}
		
		if (mousedown)
		{
			newkey = true;
			lastkey_sym = SDLK_RETURN;
		}
		
		if (has_mouse && input_grab_enabled)
		{
			if (abs(mouse_y - 100) > 10)
			{
				inputDetected = true;
				if (mouse_y - 100 < 0)
				{
					lastkey_sym = SDLK_UP;
				} else {
					lastkey_sym = SDLK_DOWN;
				}
				newkey = true;
			}
			if (abs(mouse_x - 160) > 10)
			{
				inputDetected = true;
				if (mouse_x - 160 < 0)
				{
					lastkey_sym = SDLK_LEFT;
				} else {
					lastkey_sym = SDLK_RIGHT;
				}
				newkey = true;
			}
		}
		
		NETWORK_KEEP_ALIVE();
		
		SDL_Delay(16);
		
		if (*waitTime > 0)
		{
			(*waitTime)--;
		}
	} while (!(inputDetected || *waitTime == 1 || haltGame));
}

