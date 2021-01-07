/*
 * interrpt.c
 * Copyright (C) 1998 Brainchild Design - http://brainchilddesign.com/
 * 
 * Copyright (C) 2001 Chuck Mason <cemason@users.sourceforge.net>
 *
 * Copyright (C) 2002 Florian Schulze <crow@icculus.org>
 *
 * This file is part of Jump'n'Bump.
 *
 * Jump'n'Bump is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Jump'n'Bump is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include "globals.h"

#ifdef USE_KAILLERA
#include "SDL_thread.h"
#include "SDL_mutex.h"
#include <kailleraclient.h>

char local_keyb[256];
#endif /* USE_KAILLERA */

char keyb[256];
char last_keys[50];

#ifdef USE_KAILLERA

/* information about the party in this session */
static int my_player = -1;
static int my_numplayers = -1;

/* semaphore for controlling kaillera thread */
static SDL_sem *game_start_sem = NULL;

/* keys supported on my end */
static int my_player_up = -1;
static int my_player_left = -1;
static int my_player_right = 1;

/* values for the kaillera client interface */
static char kaillera_app_name[] = "Jump 'n Bump";
static char kaillera_game_name[] = "Jump 'n Bump\0\0";

static int player_keys[4][3] = {
	{
		KEY_PL1_LEFT,
		KEY_PL1_RIGHT,
		KEY_PL1_JUMP
	},                        
	{
		KEY_PL2_LEFT,
		KEY_PL2_RIGHT,
		KEY_PL2_JUMP
	},
	{
		KEY_PL3_LEFT,
		KEY_PL3_RIGHT,
		KEY_PL3_JUMP
	},
	{
		KEY_PL4_LEFT,
		KEY_PL4_RIGHT,
		KEY_PL4_JUMP
	}
};

static int WINAPI kaillera_game_callback(char *game, int player, int numplayers)
{
	int length;
	int urand;
	unsigned char random[8];

	if (strcmp(game, kaillera_game_name) != 0) {
		printf("unknown game selected: %s\n", game);

		my_player = -1;
		goto release;
	}

	printf("start network game with %d players\n", numplayers);
	printf("I am player %d\n", player);

	my_player = player;
	my_numplayers = numplayers;

	my_player_up = player_keys[player-1][0] & 0xff;
	my_player_left = player_keys[player-1][1] & 0xff;
	my_player_right = player_keys[player-1][2] & 0xff;

	/* initialize randomizer agreed by all players */
	random[0] = time(0) & 0xff;
	random[1] = random[2] = random[3] = 0x00;
	length = kailleraModifyPlayValues(&random, sizeof(random[0]));
	if (length < 0) {
		goto release;
	}

	urand = random[3] << 24 | random[2] << 16 | random[1] << 8 | random[0];
	srand(urand);

release:

	SDL_SemPost(game_start_sem);
	return 0;
}

static kailleraInfos kaillera_data = {
	kaillera_app_name,
	kaillera_game_name,
	kaillera_game_callback,
	NULL,
	NULL,
	NULL
};

static void print_version()
{
	char version[16];

	kailleraGetVersion(version);
	printf("using kaillera version %s\n", version);
}

static int kaillera_thread(void *arg)
{
	kailleraInit();
	
	/* print_version(); */

	kailleraSetInfos(&kaillera_data);

	kailleraSelectServerDialog(0);
	if (SDL_SemValue(game_start_sem) == 0) {
		/* server dialog returned and game didnt start */
		
		/* release blocking thread */
		my_player = -1;
		SDL_SemPost(game_start_sem);
	}

	return 0;
}

static int start_kaillera_thread(void)
{
	SDL_Thread *thread;

	game_start_sem = SDL_CreateSemaphore(0);

	thread = SDL_CreateThread(kaillera_thread, NULL);
	if (!thread) {
		printf("SDL_CreateThread failed\n");
		return -1;
	}
	
	return 0;
}	

int addkey(unsigned int key)
{
	/* it doesnt matter if a player presses keys
	 * that control other bunnies. whatever is sent 
	 * is packed by pack_keys()
	 */
	if (!(key & 0x8000)) {
		local_keyb[key & 0x7fff] = 1;
	} else
		local_keyb[key & 0x7fff] = 0;
	return 0;
}

void remove_keyb_handler(void)
{
	kailleraShutdown();
}

int pack_keys(void)
{
	int rv;

	rv = local_keyb[my_player_up];
	rv |= local_keyb[my_player_left] << 1;
	rv |= local_keyb[my_player_right] << 2;
	rv |= local_keyb[1] << 3;
	return rv;
}

void unpack_keys(int player, char value)
{
	keyb[player_keys[player][0] & 0xff] = (value >> 0) & 1;
	keyb[player_keys[player][1] & 0xff] = (value >> 1) & 1;
	keyb[player_keys[player][2] & 0xff] = (value >> 2) & 1;

	/* escape key is shared among all users */
	keyb[1] |= (value >> 3) & 1;
}

int update_kaillera_keys(void)
{
	char keys[8];
	int length;
	int player;

	keys[0] = pack_keys();
	length = kailleraModifyPlayValues(&keys, sizeof(keys[0]));

	if (length < 0) {
		/* terminate session */
		printf("** LOST CONNECTION **\n");
		kailleraEndGame();
		my_player = -1;
		return -1;
	}

	for (player=0; player<length; player++) {
		unpack_keys(player, keys[player]);
	}

	return 0;
}

int hook_keyb_handler(void)
{
	SDL_EnableUNICODE(1);
	memset((void *) last_keys, 0, sizeof(last_keys));

	start_kaillera_thread();
	SDL_SemWait(game_start_sem);
	if (my_player < 0) {
		printf("GAME ABORTED!\n");
		return -1;
	}

	printf("GAME STARTS!\n");
	return 0;
}

int key_pressed(int key)
{
	if (key == 1 && my_player < 0) {
		/* if game completed or aborted, post ESC */
		return 1;
	}

	return keyb[(unsigned char) key];
}

#else /* USE_KAILLERA */

int addkey(unsigned int key)
{
	int c1;

	if (!(key & 0x8000)) {
		keyb[key & 0x7fff] = 1;
		for (c1 = 48; c1 > 0; c1--)
			last_keys[c1] = last_keys[c1 - 1];
		last_keys[0] = key & 0x7fff;
	} else
		keyb[key & 0x7fff] = 0;
	return 0;
}

void remove_keyb_handler(void)
{
}

int hook_keyb_handler(void)
{
	SDL_EnableUNICODE(1);
	memset((void *) last_keys, 0, sizeof(last_keys));

	return 0;
}

int key_pressed(int key)
{
	return keyb[(unsigned char) key];
}


#endif /* USE_KAILLERA */

int intr_sysupdate()
{
	SDL_Event e;
	int i = 0;
	static int last_time = 0;
	int now, time_diff;

	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if(e.button.state == SDL_PRESSED &&
					((key_pressed(KEY_PL3_LEFT) && e.button.button == SDL_BUTTON_RIGHT) ||
					(key_pressed(KEY_PL3_RIGHT) && e.button.button == SDL_BUTTON_LEFT) ||
					(e.button.button == SDL_BUTTON_LEFT && e.button.button == SDL_BUTTON_RIGHT) ||
          e.button.button == SDL_BUTTON_MIDDLE))
				{
				addkey(KEY_PL3_JUMP & 0x7f);
				}
			else if(e.button.state == SDL_RELEASED &&
					((key_pressed(KEY_PL3_LEFT) && e.button.button == SDL_BUTTON_RIGHT) ||
					(key_pressed(KEY_PL3_RIGHT) && e.button.button == SDL_BUTTON_LEFT) ||
          e.button.button == SDL_BUTTON_MIDDLE))
				{
				addkey((KEY_PL3_JUMP & 0x7f) | 0x8000);
				}

			if(e.button.button == SDL_BUTTON_LEFT)
				{
				SDLKey sym = KEY_PL3_LEFT;
				sym &= 0x7f;
				if(e.button.state == SDL_RELEASED)
					{
					if(key_pressed(KEY_PL3_JUMP) && (SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(SDL_BUTTON_RIGHT)))
						addkey(KEY_PL3_RIGHT & 0x7f);
					else
						sym |= 0x8000;
					}
				addkey(sym);
				}
			else if(e.button.button == SDL_BUTTON_RIGHT)
				{
				SDLKey sym = KEY_PL3_RIGHT;
				sym &= 0x7f;
				if (e.button.state == SDL_RELEASED)
					{
					if(key_pressed(KEY_PL3_JUMP) && (SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(SDL_BUTTON_LEFT)))
						addkey(KEY_PL3_LEFT & 0x7f);
					else
						sym |= 0x8000;
					}
				addkey(sym);
				}
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			switch (e.key.keysym.sym) {
			case SDLK_F12:
				if (e.type == SDL_KEYDOWN) {
					SDL_Quit();
					exit(1);
				}
				break;
			case SDLK_F10:
				if (e.type == SDL_KEYDOWN) {
					fs_toggle();
				}
				break;
			case SDLK_1:
				if (e.type == SDL_KEYUP)
					ai[0] = !ai[0];

				/* Release keys, otherwise it will continue moving that way */
				addkey((KEY_PL1_LEFT & 0x7f) | 0x8000);
				addkey((KEY_PL1_RIGHT & 0x7f) | 0x8000);
				addkey((KEY_PL1_JUMP & 0x7f) | 0x8000);
				break;
			case SDLK_2:
				if (e.type == SDL_KEYUP)
					ai[1] = !ai[1];

				/* Release keys, otherwise it will continue moving that way */
				addkey((KEY_PL2_LEFT & 0x7f) | 0x8000);
				addkey((KEY_PL2_RIGHT & 0x7f) | 0x8000);
				addkey((KEY_PL2_JUMP & 0x7f) | 0x8000);
				break;
			case SDLK_3:
				if (e.type == SDL_KEYUP)
					ai[2] = !ai[2];

				/* Release keys, otherwise it will continue moving that way */
				addkey((KEY_PL3_LEFT & 0x7f) | 0x8000);
				addkey((KEY_PL3_RIGHT & 0x7f) | 0x8000);
				addkey((KEY_PL3_JUMP & 0x7f) | 0x8000);
				break;
			case SDLK_4:
				if (e.type == SDL_KEYUP)
					ai[3] = !ai[3];

				/* Release keys, otherwise it will continue moving that way */
				addkey((KEY_PL4_LEFT & 0x7f) | 0x8000);
				addkey((KEY_PL4_RIGHT & 0x7f) | 0x8000);
				addkey((KEY_PL4_JUMP & 0x7f) | 0x8000);
				break;
			case SDLK_ESCAPE:
				if (e.type == SDL_KEYUP)
					addkey(1 | 0x8000);
				else
					addkey(1 & 0x7f);
				break;
			default:
				e.key.keysym.sym &= 0x7f;
				if (e.type == SDL_KEYUP)
					e.key.keysym.sym |= 0x8000;
				addkey(e.key.keysym.sym);

				break;
			}
			break;
		default:
			break;
		}
		i++;
	}

	SDL_Delay(1);
	now = SDL_GetTicks();
	time_diff = now - last_time;
	if (time_diff>0) {
		i = time_diff / (1000 / 60);
		if (i) {
			last_time = now;
		} else {
			int tmp;

			tmp = (1000/60) - i - 10;
			if (tmp>0)
				SDL_Delay(tmp);
		}
	}
/*
	if (!then)
		SDL_Delay(1);
	else {
		then = (1000 / 60) - (now - then);
		if (then > 0 && then < 1000)
			SDL_Delay(then);
	}
	then = now;
*/

#ifdef USE_KAILLERA
	if (my_player >= 0) {
		update_kaillera_keys();
		i=1;
	}
#endif /* USE_KAILLERA */

	return i;
}
