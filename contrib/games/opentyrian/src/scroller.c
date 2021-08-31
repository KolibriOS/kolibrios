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
#include "scroller.h"

#include "font.h"
#include "joystick.h"
#include "jukebox.h"
#include "keyboard.h"
#include "loudness.h"
#include "mtrand.h"
#include "nortsong.h"
#include "nortvars.h"
#include "opentyr.h"
#include "palette.h"
#include "sprite.h"
#include "varz.h"
#include "vga256d.h"
#include "video.h"

#include <string.h>

const struct about_text_type about_text[] =
{
	{0x30, "----- ~OpenTyrian~ -----"},
	{0x00, ""},
	{0x0b, "...eliminating Microsol,"},
	{0x0b, "one planet at a time..."},
	{0x00, ""},
	{0x00, ""},
	{0x30, "----- ~Developers~ -----"},
	{0x00, ""},
	{0x03, "Carl Reinke // Mindless"},
	{0x07, "Yuri Schlesner // yuriks"},
	{0x04, "Casey McCann // syntaxglitch"},
	{0x00, ""},
	{0x00, ""},
	{0x30, "----- ~Thanks~ -----"},
	{0x00, ""},
	{0x0e, "Thanks to everyone who has"},
	{0x0e, "assisted the developers by testing"},
	{0x0e, "the game and reporting bugs."},
	{0x00, ""},
	{0x00, ""},
	{0x05, "Thanks to ~DOSBox~ for the"},
	{0x05, "FM-Synthesis emulator and"},
	{0x05, "~AdPlug~ for the Loudness player."},
	{0x00, ""},
	{0x00, ""},
	{0x32, "And special thanks to ~Jason Emery~"},
	{0x32, "for making all this possible"},
	{0x32, "by giving Tyrian to its fans."},
	{0x00, ""},
	{0x00, ""},
/*	{0x00, "This is line color test ~0~."},
	{0x01, "This is line color test ~1~."},
	{0x02, "This is line color test ~2~."},
	{0x03, "This is line color test ~3~."},
	{0x04, "This is line color test ~4~."},
	{0x05, "This is line color test ~5~."},
	{0x06, "This is line color test ~6~."},
	{0x07, "This is line color test ~7~."},
	{0x08, "This is line color test ~8~."},
	{0x09, "This is line color test ~9~."},
	{0x0a, "This is line color test ~A~."},
	{0x0b, "This is line color test ~B~."},
	{0x0c, "This is line color test ~C~."},
	{0x0d, "This is line color test ~D~."},
	{0x0e, "This is line color test ~E~."},
	{0x0f, "This is line color test ~F~."},*/
	{0x00, ""},
	{0x00, ""},
	{0x00, ""},
	{0x00, ""},
	{0x00, ""},
	{0x00, ""},
	{0x00, "Press a key to leave."},
	{0x00, NULL}
};

#define LINE_HEIGHT 15

#define MAX_BEER 5
#define BEER_SHAPE 241

struct coin_def_type {
	int shape_num;
	int frame_count;
	bool reverse_anim;
};

#define MAX_COINS 20
struct coin_def_type coin_defs[] =
{
	{1, 6}, {7, 6}, {20, 6}, {26, 6}, // Coins
	{14, 5, true}, {32, 5, true}, {51, 5, true} // Gems
};

/* Text is an array of strings terminated by a NULL */
void scroller_sine( const struct about_text_type text[] )
{
	bool ale = mt_rand() % 2;

	int visible_lines = vga_height / LINE_HEIGHT + 1;
	int current_line = -visible_lines;
	int y = 0;
	bool fade_in = true;

	struct coin_type { int x, y, vel, type, cur_frame; bool backwards; } coins[MAX_COINS];
	struct { int x, y, ay, vx, vy; } beer[MAX_BEER];

	if (ale)
	{
		memset(beer, 0, sizeof(beer));
	} else {
		for (int i = 0; i < MAX_COINS; i++)
		{
			coins[i].x = mt_rand() % (vga_width - 12);
			coins[i].y = mt_rand() % (vga_height - 20 - 14);

			coins[i].vel = (mt_rand() % 4) + 1;
			coins[i].type = mt_rand() % COUNTOF(coin_defs);
			coins[i].cur_frame = mt_rand() % coin_defs[coins[i].type].frame_count;
			coins[i].backwards = false;
		}
	}

	fade_black(10);

	wait_noinput(true, true, true);

	play_song(40); // BEER

	while (!JE_anyButton())
	{
		setdelay(3);

		JE_clr256(VGAScreen);

		if (!ale)
		{
			for (int i = 0; i < MAX_COINS/2; i++)
			{
				struct coin_type *coin = &coins[i];
				blit_sprite2(VGAScreen, coin->x, coin->y, eShapes[4], coin_defs[coin->type].shape_num + coin->cur_frame);
			}
		}

		for (int i = 0; i < visible_lines; i++)
		{
			if (current_line + i >= 0)
			{
				if (text[current_line + i].text == NULL)
				{
					break;
				}

				int line_x = VGAScreen->w / 2;
				int line_y = i * LINE_HEIGHT - y;

				// smooths edges on sine-wave text
				if (text[i + current_line].effect & 0x20)
				{
					draw_font_hv(VGAScreen, line_x + 1, line_y, text[i + current_line].text, normal_font, centered, text[i + current_line].effect & 0x0f, -10);
					draw_font_hv(VGAScreen, line_x - 1, line_y, text[i + current_line].text, normal_font, centered, text[i + current_line].effect & 0x0f, -10);
				}

				draw_font_hv(VGAScreen, line_x, line_y, text[i + current_line].text, normal_font, centered, text[i + current_line].effect & 0x0f, -4);

				if (text[i + current_line].effect & 0x10)
				{
					for (int j = 0; j < LINE_HEIGHT; j++)
					{
						if (line_y + j >= 10 && line_y + j <= vga_height - 10)
						{
							int waver = sinf((((line_y + j) / 2) % 10) / 5.0f * M_PI) * 3;
							memmove(&((Uint8 *)VGAScreen->pixels)[VGAScreen->pitch * (line_y + j) + waver],
									&((Uint8 *)VGAScreen->pixels)[VGAScreen->pitch * (line_y + j)],
									VGAScreen->pitch);
						}
					}
				}
			}
		}

		if (++y == LINE_HEIGHT)
		{
			y = 0;

			if (current_line < 0 || text[current_line].text != NULL)
				++current_line;
			else
				current_line = -visible_lines;
		}

		if (!ale)
		{
			for (int i = MAX_COINS/2; i < MAX_COINS; i++)
			{
				struct coin_type *coin = &coins[i];
				blit_sprite2(VGAScreen, coin->x, coin->y, eShapes[4], coin_defs[coin->type].shape_num + coin->cur_frame);
			}
		}

		fill_rectangle_xy(VGAScreen, 0, 0, vga_width - 1, 14, 0);
		fill_rectangle_xy(VGAScreen, 0, vga_height - 14, vga_width - 1, vga_height - 1, 0);

		if (!ale)
		{
			for (int i = 0; i < MAX_COINS; i++)
			{
				struct coin_type *coin = &coins[i];

				if (coin->backwards)
				{
					coin->cur_frame--;
				} else {
					coin->cur_frame++;
				}
				if (coin->cur_frame == coin_defs[coin->type].frame_count)
				{
					if (coin_defs[coin->type].reverse_anim)
					{
						coin->backwards = true;
						coin->cur_frame -= 2;
					} else {
						coin->cur_frame = 0;
					}
				}
				if (coin->cur_frame == -1)
				{
					coin->cur_frame = 1;
					coin->backwards = false;
				}

				coin->y += coin->vel;
				if (coin->y > vga_height - 14)
				{
					coin->x = mt_rand() % (vga_width - 12);
					coin->y = 0;

					coin->vel = (mt_rand() % 4) + 1;
					coin->type = mt_rand() % COUNTOF(coin_defs);
					coin->cur_frame = mt_rand() % coin_defs[coin->type].frame_count;
				}
			}
		} else {
			for (uint i = 0; i < COUNTOF(beer); i++)
			{
				while (beer[i].vx == 0)
				{
					beer[i].x = mt_rand() % (vga_width - 24);
					beer[i].y = mt_rand() % (vga_height - 28 - 50);

					beer[i].vx = (mt_rand() % 5) - 2;
				}

				beer[i].vy++;

				if (beer[i].x + beer[i].vx > vga_width - 24 || beer[i].x + beer[i].vx < 0) // check if the beer hit the sides
				{
					beer[i].vx = -beer[i].vx;
				}
				beer[i].x += beer[i].vx;

				if (beer[i].y + beer[i].vy > vga_height - 28) // check if the beer hit the bottom
				{
					if ((beer[i].vy) < 8) // make sure the beer bounces!
					{
						beer[i].vy += mt_rand() % 2;
					} else if (beer[i].vy > 16) { // make sure the beer doesn't bounce too high
						beer[i].vy = 16;
					}
					beer[i].vy = -beer[i].vy + (mt_rand() % 3 - 1);

					beer[i].x += (beer[i].vx > 0 ? 1 : -1) * (i % 2 ? 1 : -1);
				}
				beer[i].y += beer[i].vy;

				blit_sprite2x2(VGAScreen, beer[i].x, beer[i].y, eShapes[4], BEER_SHAPE);
			}
		}

		JE_showVGA();

		if (fade_in)
		{
			fade_in = false;
			fade_palette(colors, 10, 0, 255);

			SDL_Color white = { 255, 255, 255 };
			set_colors(white, 254, 254);
		}

		wait_delay();
	}

	fade_black(10);
}

