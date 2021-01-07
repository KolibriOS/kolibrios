/*
 * globals.h
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

#ifndef __GLOBALS_H
#define __GLOBALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <strings.h>
#endif
#include <time.h>
#include <math.h>
#include <dj.h>

#ifdef DOS
# include <conio.h>
# include <dpmi.h>
# include <sys/nearptr.h>
# include <pc.h>
#endif

#ifdef _MSC_VER
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <sys/stat.h>
# include <io.h>
# include "SDL.h"
# if USE_SDL_MIXER
#  include "SDL_mixer.h"
# endif
#else
# ifdef USE_SDL
#  include <sys/stat.h>
#  include "SDL.h"
#  if USE_SDL_MIXER
#   include "SDL_mixer.h"
#  endif
# endif
#endif

#define JNB_MAX_PLAYERS 4

#define JNB_INETPORT 11111

extern int client_player_num;
void tellServerPlayerMoved(int playerid, int movement_type, int newval);
#define MOVEMENT_LEFT  1
#define MOVEMENT_RIGHT 2
#define MOVEMENT_UP    3

#define JNB_VERSION "1.51"

#define JNB_WIDTH 400
#define JNB_HEIGHT 256

extern int screen_width;
extern int screen_height;
extern int screen_pitch;
extern int scale_up;

extern int ai[JNB_MAX_PLAYERS];

#ifndef USE_SDL
#define KEY_PL1_LEFT 0xcb
#define KEY_PL1_RIGHT	0xcd
#define KEY_PL1_JUMP 0xc8
#define KEY_PL2_LEFT 0x1e
#define KEY_PL2_RIGHT	0x20
#define KEY_PL2_JUMP 0x11
#else
#define KEY_PL1_LEFT SDLK_LEFT
#define KEY_PL1_RIGHT	SDLK_RIGHT
#define KEY_PL1_JUMP SDLK_UP
#define KEY_PL2_LEFT SDLK_a
#define KEY_PL2_RIGHT	SDLK_d
#define KEY_PL2_JUMP SDLK_w
#define KEY_PL3_LEFT SDLK_j
#define KEY_PL3_RIGHT	SDLK_l
#define KEY_PL3_JUMP SDLK_i
#define KEY_PL4_LEFT SDLK_KP4
#define KEY_PL4_RIGHT	SDLK_KP6
#define KEY_PL4_JUMP SDLK_KP8
#endif

#define NUM_POBS 200
#define NUM_OBJECTS 200
#define NUM_FLIES 20
#define NUM_LEFTOVERS 50

#define OBJ_SPRING 0
#define OBJ_SPLASH 1
#define OBJ_SMOKE 2
#define OBJ_YEL_BUTFLY 3
#define OBJ_PINK_BUTFLY 4
#define OBJ_FUR 5
#define OBJ_FLESH 6
#define OBJ_FLESH_TRACE 7

#define OBJ_ANIM_SPRING 0
#define OBJ_ANIM_SPLASH 1
#define OBJ_ANIM_SMOKE 2
#define OBJ_ANIM_YEL_BUTFLY_RIGHT 3
#define OBJ_ANIM_YEL_BUTFLY_LEFT 4
#define OBJ_ANIM_PINK_BUTFLY_RIGHT 5
#define OBJ_ANIM_PINK_BUTFLY_LEFT 6
#define OBJ_ANIM_FLESH_TRACE 7

#define MOD_MENU 0
#define MOD_GAME 1
#define MOD_SCORES 2

#define SFX_JUMP 0
#define SFX_LAND 1
#define SFX_DEATH 2
#define SFX_SPRING 3
#define SFX_SPLASH 4
#define SFX_FLY 5

#define NUM_SFX 6

#define SFX_JUMP_FREQ 15000
#define SFX_LAND_FREQ 15000
#define SFX_DEATH_FREQ 20000
#define SFX_SPRING_FREQ 15000
#define SFX_SPLASH_FREQ 12000
#define SFX_FLY_FREQ 12000

#define BAN_VOID	0
#define BAN_SOLID	1
#define BAN_WATER	2
#define BAN_ICE		3
#define BAN_SPRING	4

#ifndef DATA_PATH
#ifdef __APPLE__
#define	DATA_PATH "data/jumpbump.dat"
#elif _WIN32
#define	DATA_PATH "data/jumpbump.dat"
#else
#define	DATA_PATH "jumpbump.dat"
#endif
#endif

typedef struct {
	int num_images;
	int *width;
	int *height;
	int *hs_x;
	int *hs_y;
	void **data;
	void **orig_data;
} gob_t;

typedef struct {
	int joy_enabled, mouse_enabled;
	int no_sound, music_no_sound, no_gore, fireworks;
	char error_str[256];
	int draw_page, view_page;
	struct {
		int num_pobs;
		struct {
			int x, y;
			int image;
			gob_t *pob_data;
			int back_buf_ofs;
		} pobs[NUM_POBS];
	} page_info[2];
	void *pob_backbuf[2];
} main_info_t;

typedef struct {
	int action_left,action_up,action_right;
	int enabled, dead_flag;
	int bumps;
	int bumped[JNB_MAX_PLAYERS];
	int x, y;
	int x_add, y_add;
	int direction, jump_ready, jump_abort, in_water;
	int anim, frame, frame_tick, image;
} player_t;

typedef struct {
	int num_frames;
	int restart_frame;
	struct {
		int image;
		int ticks;
	} frame[4];
} player_anim_t;

typedef struct {
	int used, type;
	int x, y;
	int x_add, y_add;
	int x_acc, y_acc;
	int anim;
	int frame, ticks;
	int image;
} object_t;

typedef struct {
	int x, y;
	int raw_x, raw_y;
	int but1, but2;
	struct {
		int x1, x2, x3;
		int y1, y2, y3;
	} calib_data;
} joy_t;

typedef struct {
	int but1, but2, but3;
} mouse_t;

extern main_info_t main_info;
extern player_t player[JNB_MAX_PLAYERS];
extern player_anim_t player_anims[7];
extern object_t objects[NUM_OBJECTS];
extern joy_t joy;
extern mouse_t mouse;

extern char datfile_name[2048];

extern char *background_pic;
extern char *mask_pic;

extern gob_t rabbit_gobs;
extern gob_t font_gobs;
extern gob_t object_gobs;
extern gob_t number_gobs;


/* fireworks.c */

void fireworks(void);


/* main.c */

void steer_players(void);
void position_player(int player_num);
void fireworks(void);
void add_object(int type, int x, int y, int x_add, int y_add, int anim, int frame);
void update_objects(void);
int add_pob(int page, int x, int y, int image, gob_t *pob_data);
void draw_flies(int page);
void draw_pobs(int page);
void redraw_flies_background(int page);
void redraw_pob_backgrounds(int page);
int add_leftovers(int page, int x, int y, int image, gob_t *pob_data);
void draw_leftovers(int page);
int init_level(int level, char *pal);
void deinit_level(void);
int init_program(int argc, char *argv[], char *pal);
void deinit_program(void);
unsigned short rnd(unsigned short max);
int read_level(void);
unsigned char *dat_open(char *file_name);
int dat_filelen(char *file_name);
void write_calib_data(void);


/* input.c */

void update_player_actions(void);
void init_inputs(void);
int calib_joy(int type);

/* menu.c */

int menu(void);
int menu_init(void);
void menu_deinit(void);


/* gfx.c */

void set_scaling(int scale);
void open_screen(void);
void wait_vrt(int mix);
void draw_begin(void);
void draw_end(void);
void flippage(int page);
void draw_begin(void);
void draw_end(void);
void clear_lines(int page, int y, int count, int color);
int get_color(int color, char pal[768]);
int get_pixel(int page, int x, int y);
void set_pixel(int page, int x, int y, int color);
void setpalette(int index, int count, char *palette);
void fillpalette(int red, int green, int blue);
#ifdef DOS
void get_block(char page, short x, short y, short width, short height, char *buffer);
void put_block(char page, short x, short y, short width, short height, char *buffer);
#else
void get_block(int page, int x, int y, int width, int height, void *buffer);
void put_block(int page, int x, int y, int width, int height, void *buffer);
#endif
void put_text(int page, int x, int y, char *text, int align);
void put_pob(int page, int x, int y, int image, gob_t *gob, int mask, void *mask_pic);
int pob_width(int image, gob_t *gob);
int pob_height(int image, gob_t *gob);
int pob_hs_x(int image, gob_t *gob);
int pob_hs_y(int image, gob_t *gob);
int read_pcx(unsigned char * handle, void *buffer, int buf_len, char *pal);
void register_background(char *pixels, char pal[768]);
int register_gob(unsigned char *handle, gob_t *gob, int len);
void recalculate_gob(gob_t *gob, char pal[768]);
void register_mask(void *pixels);

/* gfx.c */

#ifdef USE_SDL
/* long filelength(int handle); */
void fs_toggle();
int intr_sysupdate();
#endif

/* interrpt.c */

extern char last_keys[50];

int hook_keyb_handler(void);
void remove_keyb_handler(void);
int key_pressed(int key);
int addkey(unsigned int key);

/* sound-linux.c */
#ifdef LINUX


#endif

#ifdef __cplusplus
}
#endif

#endif
