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
#include "jukebox.h"

#include "font.h"
#include "joystick.h"
#include "keyboard.h"
#include "lds_play.h"
#include "loudness.h"
#include "mtrand.h"
#include "nortsong.h"
#include "opentyr.h"
#include "palette.h"
#include "sprite.h"
#include "starlib.h"
#include "vga_palette.h"
#include "video.h"

#include <stdio.h>

void jukebox( void )
{
	bool trigger_quit = false,  // true when user wants to quit
	     quitting = false;
	
	bool hide_text = false;

	bool fade_looped_songs = true, fading_song = false;
	bool stopped = false;

	bool fx = false;
	int fx_num = 0;

	int palette_fade_steps = 15;

	int diff[256][3];
	init_step_fade_palette(diff, vga_palette, 0, 255);

	JE_starlib_init();

	int fade_volume = tyrMusicVolume;
	
	for (; ; )
	{
		if (!stopped && !audio_disabled)
		{
			if (songlooped && fade_looped_songs)
				fading_song = true;

			if (fading_song)
			{
				if (fade_volume > 5)
				{
					fade_volume -= 2;
				}
				else
				{
					fade_volume = tyrMusicVolume;

					fading_song = false;
				}

				set_volume(fade_volume, fxVolume);
			}

			if (!playing || (songlooped && fade_looped_songs && !fading_song))
				play_song(mt_rand() % MUSIC_NUM);
		}

		setdelay(1);

		SDL_FillRect(VGAScreenSeg, NULL, 0);

		// starlib input needs to be rewritten
		JE_starlib_main();

		push_joysticks_as_keyboard();
		service_SDL_events(true);

		if (!hide_text)
		{
			char buffer[60];
			
			if (fx)
				snprintf(buffer, sizeof(buffer), "%d %s", fx_num + 1, soundTitle[fx_num]);
			else
				snprintf(buffer, sizeof(buffer), "%d %s", song_playing + 1, musicTitle[song_playing]);
			
			const int x = VGAScreen->w / 2;
			
			draw_font_hv(VGAScreen, x, 170, "Press ESC to quit the jukebox.",           small_font, centered, 1, 0);
			draw_font_hv(VGAScreen, x, 180, "Arrow keys change the song being played.", small_font, centered, 1, 0);
			draw_font_hv(VGAScreen, x, 190, buffer,                                     small_font, centered, 1, 4);
		}

		if (palette_fade_steps > 0)
			step_fade_palette(diff, palette_fade_steps--, 0, 255);
		
		JE_showVGA();

		wait_delay();

		// quit on mouse click
		Uint16 x, y;
		if (JE_mousePosition(&x, &y) > 0)
			trigger_quit = true;

		if (newkey)
		{
			switch (lastkey_sym)
			{
			case SDLK_ESCAPE: // quit jukebox
			case SDLK_q:
				trigger_quit = true;
				break;

			case SDLK_SPACE:
				hide_text = !hide_text;
				break;

			case SDLK_f:
				fading_song = !fading_song;
				break;
			case SDLK_n:
				fade_looped_songs = !fade_looped_songs;
				break;

			case SDLK_SLASH: // switch to sfx mode
				fx = !fx;
				break;
			case SDLK_COMMA:
				if (fx && --fx_num < 0)
					fx_num = SAMPLE_COUNT - 1;
				break;
			case SDLK_PERIOD:
				if (fx && ++fx_num >= SAMPLE_COUNT)
					fx_num = 0;
				break;
			case SDLK_SEMICOLON:
				if (fx)
					JE_playSampleNum(fx_num + 1);
				break;

			case SDLK_LEFT:
			case SDLK_UP:
				play_song((song_playing > 0 ? song_playing : MUSIC_NUM) - 1);
				stopped = false;
				break;
			case SDLK_RETURN:
			case SDLK_RIGHT:
			case SDLK_DOWN:
				play_song((song_playing + 1) % MUSIC_NUM);
				stopped = false;
				break;
			case SDLK_s: // stop song
				stop_song();
				stopped = true;
				break;
			case SDLK_r: // restart song
				restart_song();
				stopped = false;
				break;

			default:
				break;
			}
		}
		
		// user wants to quit, start fade-out
		if (trigger_quit && !quitting)
		{
			palette_fade_steps = 15;
			
			SDL_Color black = { 0, 0, 0 };
			init_step_fade_solid(diff, black, 0, 255);
			
			quitting = true;
		}
		
		// if fade-out finished, we can finally quit
		if (quitting && palette_fade_steps == 0)
			break;
	}

	set_volume(tyrMusicVolume, fxVolume);
}

