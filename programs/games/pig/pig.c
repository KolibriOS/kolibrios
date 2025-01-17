/*
------------------------------------------------------------
	Fixed Rate Pig - a fixed logic frame rate demo
------------------------------------------------------------
 * Copyright (C) 2004 David Olofson <david@olofson.net>
 *
 * This software is released under the terms of the GPL.
 *
 * Contact author for permission if you want to use this
 * software, or work derived from it, under other terms.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "engine.h"


/* Graphics defines */
#define	SCREEN_W	800
#define	SCREEN_H	600
#define	TILE_W		32
#define	TILE_H		32
#define	MAP_W		25
#define	MAP_H		17
#define	FONT_SPACING	45
#define	PIG_FRAMES	12

/* World/physics constants */
#define	GRAV_ACC	4
#define	JUMP_SPEED	28

/* Sprite collision groups */
#define	GROUP_ENEMY	0x0001
#define	GROUP_POWERUP	0x0002

typedef enum
{
	POWER_LIFE,
	POWER_BONUS1,
	POWER_BONUS2
} POWERUPS;


typedef struct GAMESTATE
{
	/* I/O */
	PIG_engine	*pe;
	Uint8		*keys;
	int		nice;
	int		refresh_screen;
	int		jump;

	/* Sprites */
	int		lifepig;
	int		scorefont;
	int		glassfont;
	int		icons;
	int		stars;
	int		pigframes;
	int		evil;
	int		slime;

	/* Global game state */
	int		running;
	int		level;
	int		lives;
	float		lives_wobble;
	float		lives_wobble_time;
	int		score;
	float		score_wobble;
	float		score_wobble_time;
	float		dashboard_time;
	int		fun_count;
	int		enemycount;
	int		messages;

	/* Objects */
	PIG_object	*player;

	/* Statistics */
	int		logic_frames;
	int		rendered_frames;
} GAMESTATE;


static void add_life(GAMESTATE *gs);
static void remove_life(GAMESTATE *gs);
static void inc_score(GAMESTATE *gs, int v);
static void inc_score_nobonus(GAMESTATE *gs, int v);
static PIG_object *new_player(GAMESTATE *gs);
static void message(GAMESTATE *gs, const char *s);
static PIG_object *new_powerup(GAMESTATE *gs,
		int x, int y, int speed, POWERUPS type);
static PIG_object *new_star(GAMESTATE *gs, int x, int y, int vx, int vy);
static PIG_object *new_evil(GAMESTATE *gs,
		int x, int y, int speed);
static PIG_object *new_slime(GAMESTATE *gs,
		int x, int y, int speed);


/*----------------------------------------------------------
	Init, load stuff etc
----------------------------------------------------------*/

static int load_level(GAMESTATE *gs, int map)
{
	const char *m;
	const char *k;
	if(map > 4)
		map = 1;
	gs->level = map;
	pig_object_close_all(gs->pe);
	gs->enemycount = 0;
	gs->messages = 0;
	switch(map)
	{
	  case 1:
	  case 2:
	  case 4:
	  	k =	"abcd" "efgh" "ijkl"	/* Red, green, yellov */
			"0123456789ABCDEFG"	/* Sky */
			"xyz";			/* Single R, G, Y */
		break;
	  case 0:
	  case 3:
	  	k =	"abcd" "efgh" "ijkl"	/* Red, green, yellov */
			"................."
			"xyz"			/* Single R, G, Y */
			"-+012345..ABCDEF";	/* Night sky */
		break;
	}
	switch(map)
	{
	  case 0: m =	"-------------ad----------"
			"-abcd-x-ad--ad-abcd-acd--"
			"-x----x--abcd--x----x--x-"
			"-abd--x---ad---abd--x--x-"
			"-x----x--abcd--x----x--x-"
			"-x----x-ad--ad-abcd-abd--"
			"----efhad-eh--egh-efh----"
			"----y--y-y--y--y--y------"
			"++++efh++efgh++y++eh+++++"
			"0123y50y2y45y12y45y123450"
			"ABCDyFAyCyEFyBCyEFeghDEFA"
			"----ijkjl-ijkl--ijkjl----"
			"----il--il-il--il--------"
			"----ijkjl--il--il-ikl----"
			"----il-----il--il--il----"
			"----il----ijkl--ijkjl----"
			"-------------------------";
			break;
	  case 1: m =	"0000000000000000000000000"
			"1111111111111111111111111"
			"2222222222222222222222222"
			"3333333333333333333333333"
			"4444444444444444444444444"
			"5555555555555555555555555"
			"6666666666666666666666666"
			"7777777ijkjkjjkjkl7777777"
			"8888888888888888888888888"
			"9999999999999999999999999"
			"abcdAAAAAAAAAAAAAAAAAabcd"
			"BBBBBBBBBBBBBBBBBBBBBBBBB"
			"CCCCCCCCCCCCCCCCCCCCCCCCC"
			"efgfgffgfgfgfgfggffgfgfgh"
			"EEEEEEEEEEEEEEEEEEEEEEEEE"
			"FFFFFFFFFFFFFFFFFFFFFFFFF"
			"GGGGGGGGGGGGGGGGGGGGGGGGG";
			new_evil(gs, 2, 0, 5);
			new_evil(gs, 22, 0, 5);
			new_evil(gs, 5, 0, 7);
			new_evil(gs, 19, 0, 7);
			break;
	  case 2: m =	"0000000000000000000000000"
			"1111111111111111111111111"
			"2222222222222222222222222"
			"3333333333333333333333333"
			"4444444444xxxxx4444444444"
			"5555555555x555x5555555555"
			"6666666666x666x6666666666"
			"7777777xxxx777xxxx7777777"
			"8888888x888888888x8888888"
			"9999999x999999999x9999999"
			"AAAAAAAxxxxAAAxxxxAAAAAAA"
			"BBBBBBBBBBxBBBxBBBBBBBBBB"
			"CCCCCCCCCCxCCCxCCCCCCCCCC"
			"DDDDDDDDDDxxxxxDDDDDDDDDD"
			"EEEEEEEEEEEEEEEEEEEEEEEEE"
			"ijklFFFFFFFFFFFFFFFFFijkl"
			"GGGijlGilGilGilGilGiklGGG";
			new_slime(gs, 2, 0, -5);
			new_slime(gs, 22, 0, 5);
			new_evil(gs, 8, 0, 7);
			new_evil(gs, 16, 0, -7);
			break;
	  case 3: m =	"-------------------------"
			"-------------------------"
			"-------------------------"
			"-------------------------"
			"ijkl----------efgh-------"
			"-------------------------"
			"-------------------------"
			"z----------------abcbcbbd"
			"+++++++++++++++++++++++++"
			"01z3450123450123450123450"
			"ABCDEFABCefgfgfghFABCDEFA"
			"----z--------------------"
			"-------------------------"
			"------z--------------ijkl"
			"-------------------------"
			"-------------------------"
			"abdefghijkl---efghijklabd";
			new_slime(gs, 5, 0, -5);
			new_slime(gs, 20, 15, -5);
			new_evil(gs, 1, 0, 7);
			new_evil(gs, 20, 0, 10);
			new_evil(gs, 15, 0, 7);
			break;
	  case 4: m =	"0000000000000000000000000"
			"1111111111111111111111111"
			"2222222222222222222222222"
			"3333333333333333333333333"
			"4444444444444444444444444"
			"555555555555z555555555555"
			"66666666666ijl66666666666"
			"7777777777ijlil7777777777"
			"888888888ijlikkl888888888"
			"99999999ijkjklikl99999999"
			"AAAAAAAikjlijkjkjlAAAAAAA"
			"BBBBBBiklijkjlijkjlBBBBBB"
			"CCCCCijkjlikkjklikklCCCCC"
			"DDDDijklijjklikjkjkklDDDD"
			"EEEijkkjkjlikjkjlijjklEEE"
			"FFijkjlilijkjklikjlikklFF"
			"efggfggfgfgfggfgfgfgfgfgh";
			new_evil(gs, 11, 0, 5);
			new_evil(gs, 10, 0, 6);
			new_evil(gs, 9, 0, 7);
			new_evil(gs, 8, 0, 8);
			new_evil(gs, 7, 0, 9);
			new_evil(gs, 6, 0, 10);
			new_evil(gs, 5, 0, 11);
			new_evil(gs, 4, 0, 12);
			new_evil(gs, 3, 0, 13);
			new_slime(gs, 1, 0, 16);
			new_slime(gs, 24, 0, -14);
			break;
	  default:
		return -1;
	}
	pig_map_from_string(gs->pe->map, k, m);
	gs->refresh_screen = gs->pe->pages;
	return 0;
}


static GAMESTATE *init_all(SDL_Surface *screen)
{
	int i;
	PIG_map *pm;
	GAMESTATE *gs = (GAMESTATE *)calloc(1, sizeof(GAMESTATE));
	if(!gs)
		return NULL;

	gs->running = 1;

	gs->pe = pig_open(screen);
	if(!gs->pe)
	{
		fprintf(stderr, "Could not open the Pig Engine!\n");
		free(gs);
		return NULL;
	}
	gs->pe->userdata = gs;

	pig_viewport(gs->pe, 0, 0, SCREEN_W, MAP_H * TILE_H);

	i = gs->lifepig = pig_sprites(gs->pe, "lifepig.png", 0, 0);
	i |= gs->scorefont = pig_sprites(gs->pe, "font.png", 44, 56);
	i |= gs->glassfont = pig_sprites(gs->pe, "glassfont.png", 60, 60);
	i |= gs->icons = pig_sprites(gs->pe, "icons.png", 48, 48);
	i |= gs->stars = pig_sprites(gs->pe, "stars.png", 32, 32);
	i |= gs->pigframes = pig_sprites(gs->pe, "pigframes.png", 64, 48);
	i |= gs->evil = pig_sprites(gs->pe, "evil.png", 48, 48);
	i |= gs->slime = pig_sprites(gs->pe, "slime.png", 48, 48);
	if(i < 0)
	{
		fprintf(stderr, "Could not load graphics!\n");
		pig_close(gs->pe);
		free(gs);
		return NULL;
	}
	for(i = gs->icons; i < gs->icons + 3*8; ++i)
		pig_hotspot(gs->pe, i, PIG_CENTER, 45);
	for(i = gs->pigframes; i < gs->pigframes + 12; ++i)
		pig_hotspot(gs->pe, i, PIG_CENTER, 43);
	for(i = gs->evil; i < gs->evil + 16; ++i)
		pig_hotspot(gs->pe, i, PIG_CENTER, 46);
	for(i = gs->slime; i < gs->slime + 16; ++i)
		pig_hotspot(gs->pe, i, PIG_CENTER, 46);

	pm = pig_map_open(gs->pe, MAP_W, MAP_H);
	if(!pm)
	{
		fprintf(stderr, "Could not create map!\n");
		pig_close(gs->pe);
		free(gs);
		return NULL;
	}
	if(pig_map_tiles(pm, "tiles.png", TILE_W, TILE_H) < 0)
	{
		fprintf(stderr, "Could not load background graphics!\n");
		pig_close(gs->pe);
		free(gs);
		return NULL;
	}

	/* Mark tiles for collision detection */
	pig_map_collisions(pm, 0, 12, PIG_ALL);	/* Red, green, yellov */
	pig_map_collisions(pm, 12, 17, PIG_NONE);/* Sky */
	pig_map_collisions(pm, 29, 3, PIG_ALL);	/* Single R, G, Y */

	load_level(gs, 0);
	return gs;
}


/*----------------------------------------------------------
	Render the dashboard
----------------------------------------------------------*/
static void dashboard(GAMESTATE *gs)
{
	SDL_Rect r;
	int i, v;
	float x;
	float t = SDL_GetTicks() * 0.001;
	r.x = 0;
	r.y = SCREEN_H - 56;
	r.w = SCREEN_W;
	r.h = 56;
	SDL_SetClipRect(gs->pe->surface, &r);

	/* Render "plasma bar" */
	for(i = 0; i < 56; ++i)
	{
		float f1, f2, m;
		SDL_Rect cr;
		cr.x = 0;
		cr.w = SCREEN_W;
		cr.y = SCREEN_H - 56 + i;
		cr.h = 1;
		f1 = .25 + .25 * sin(t * 1.7 + (float)i / SCREEN_H * 42);
		f1 += .25 + .25 * sin(-t * 2.1 + (float)i / SCREEN_H * 66);
		f2 = .25 + .25 * sin(t * 3.31 + (float)i / SCREEN_H * 90);
		f2 += .25 + .25 * sin(-t * 1.1 + (float)i / SCREEN_H * 154);
		m = sin((float)i * M_PI / 56.0);
		m = sin(m * M_PI * 0.5);
		m = sin(m * M_PI * 0.5);
		SDL_FillRect(gs->pe->surface,
				&cr, SDL_MapRGB(gs->pe->surface->format,
				((int)128.0 * f1 + 64) * m,
				((int)64.0 * f1 * f2 + 64) * m,
				((int)128.0 * f2 + 32) * m
				));
	}

	/* Draw pigs... uh, lives! */
	x = -10;
	for(i = 0; i < gs->lives; ++i)
	{
		x += 48 + gs->lives_wobble *
				sin(gs->lives_wobble_time * 12) * .2;
		pig_draw_sprite(gs->pe, gs->lifepig,
				(int)x + gs->lives_wobble *
				sin(gs->lives_wobble_time * 20 + i * 1.7),
				SCREEN_H - 56/2);
	}

	/* Print score */
	x = SCREEN_W + 5;
	v = gs->score;
	for(i = 9; i >= 0; --i)
	{
		int n = v % 10;
		x -= 39 - gs->score_wobble *
				sin(gs->score_wobble_time * 15 + i * .5);
		pig_draw_sprite(gs->pe, gs->scorefont + n, (int)x,
				SCREEN_H - 56/2);
		v /= 10;
		if(!v)
			break;
	}

	pig_dirty(gs->pe, &r);
}


/*----------------------------------------------------------
	Game logic event handlers
----------------------------------------------------------*/
static void before_objects(PIG_engine *pe)
{
	GAMESTATE *gs = (GAMESTATE *)pe->userdata;
	if(gs->lives_wobble > 0)
	{
		gs->lives_wobble *= 0.95;
		gs->lives_wobble -= 0.3;
		if(gs->lives_wobble < 0)
			gs->lives_wobble = 0;
	}
	if(gs->score_wobble > 0)
	{
		gs->score_wobble *= 0.95;
		gs->score_wobble -= 0.3;
		if(gs->score_wobble < 0)
			gs->score_wobble = 0;
	}
	++gs->logic_frames;

	if(0 == gs->level)
	{
		switch(gs->fun_count % 60)
		{
		  case 17:
			new_powerup(gs, 250, -20, -10, POWER_LIFE);
			break;
		  case 29:
			new_powerup(gs, 550, -20, 10, POWER_LIFE);
			break;
		  case 37:
			new_powerup(gs, 250, -20, 10, POWER_BONUS2);
			break;
		  case 51:
			new_powerup(gs, 550, -20, -10, POWER_BONUS1);
			break;
		}
		if(150 == gs->fun_count % 300)
			message(gs, "Press Space!");
		++gs->fun_count;
	}
}


typedef enum
{
	WAITING,
	WALKING,
	FALLING,
	KNOCKED,
	NEXT_LEVEL,
	DEAD
} OBJECT_states;


static void player_handler(PIG_object *po, const PIG_event *ev)
{
	GAMESTATE *gs = (GAMESTATE *)po->owner->userdata;
	switch(ev->type)
	{
	  case PIG_PREFRAME:
		switch(po->state)
		{
		  case WAITING:
			if(1 == po->age)
				message(gs, "Get ready!");
			else if(po->age > 50)
				po->state = FALLING;
			break;
		  case WALKING:
			if(gs->keys[SDLK_LEFT])
			{
				po->ax = -(20 + po->vx) * .4;
				po->target = 3 + po->age % 4 - 1;
				if(5 == po->target)
					po->target = 3;
			}
			else if(gs->keys[SDLK_RIGHT])
			{
				po->ax = (20 - po->vx) * .4;
				po->target = 9 + po->age % 4 - 1;
				if(11 == po->target)
					po->target = 9;
			}
			else
			{
				po->ax = -po->vx * .8;
				if(po->target >= 6)
					po->target = (po->target + 1) %
							PIG_FRAMES;
				else if(po->target)
					--po->target;
			}
			break;
		  case FALLING:
			if(gs->keys[SDLK_LEFT])
				po->ax = -(20 + po->vx) * .2;
			else if(gs->keys[SDLK_RIGHT])
				po->ax = (20 - po->vx) * .2;
			else
				po->ax = -po->vx * .2;
			po->target = (po->target + 1) % PIG_FRAMES;
			break;
		}
		po->timer[0] = 1;
		break;
	  case PIG_TIMER0:
		if(po->x < 0)
			po->x = 0;
		else if(po->x > po->owner->view.w - 1)
			po->x = po->owner->view.w - 1;
		switch(po->state)
		{
		  case WALKING:
			if(po->power)
				--po->power;
			po->image = po->target % PIG_FRAMES;
			if(!pig_test_map(gs->pe, po->x, po->y + 1))
			{
				po->state = FALLING;
				po->ay = GRAV_ACC;
			}
			if(gs->jump || gs->keys[SDLK_UP])
			{
				po->ay = 0;
				po->vy = -JUMP_SPEED;
				po->state = FALLING;
				gs->jump = 0;
			}
			break;
		  case FALLING:
			if(po->vy > 2)
				po->power = 3;
			po->ay = GRAV_ACC;
			po->image = po->target;
			break;
		  case KNOCKED:
			po->power = 0;
			po->ay = GRAV_ACC;
			po->target = (po->target + 2) % PIG_FRAMES;
			po->image = po->target;
			po->ax = -po->vx * .2;
			break;
		  case NEXT_LEVEL:
			po->vx = (SCREEN_W / 2 - po->x) * .1;
			po->target = (po->target + 1) % PIG_FRAMES;
			po->image = po->target;
			break;
		  case DEAD:
			po->ax = po->ay = 0;
			po->vx = po->vy = 0;
			break;
		}
		if(gs->jump)
			--gs->jump;
		if(NEXT_LEVEL != po->state)
		{
			if(gs->enemycount <= 0)
			{
				message(gs, "Well Done!");
				po->state = NEXT_LEVEL;
				po->vy = 0;
				po->ay = -1;
				po->tilemask = 0;
				po->hitgroup = 0;
				po->timer[2] = 50;
			}
		}
		break;

	  case PIG_TIMER1:
		/* Snap out of KNOCKED mode */
		po->state = FALLING;
		break;

	  case PIG_TIMER2:
		switch(po->state)
		{
		  case NEXT_LEVEL:
			add_life(gs);
			pig_object_close(po);
			load_level(gs, gs->level + 1);
			new_player(gs);
			break;
		  default:
			pig_object_close(po);
			if(!new_player(gs))
				load_level(gs, 0);
			break;
		}
		break;

	  case PIG_HIT_TILE:
		if(KNOCKED == po->state)
			break;

		if(ev->cinfo.sides & PIG_TOP)
		{
			po->y = ev->cinfo.y;
			po->vy = 0;
			po->ay = 0;
		}
		po->state = WALKING;
		break;

	  case PIG_HIT_OBJECT:
		if(KNOCKED == po->state)
			break;

		switch(ev->obj->hitgroup)
		{
		  case GROUP_ENEMY:
			if((po->power && ev->cinfo.sides & PIG_TOP) ||
					(po->vy - ev->obj->vy) >= 15)
			{
				/* Win: Stomp! */
				inc_score(gs, ev->obj->score);
				ev->obj->y = ev->cinfo.y + 10;
				if(po->vy > 0)
					ev->obj->vy = po->vy;
				else
					ev->obj->vy = 10;
				ev->obj->ay = GRAV_ACC;
				ev->obj->tilemask = 0;
				ev->obj->hitgroup = 0;
				if(gs->jump || gs->keys[SDLK_UP])
				{
					/* Mega jump! */
					po->vy = -(JUMP_SPEED + 7);
					gs->jump = 0;
				}
				else
				{
					/* Bounce a little */
					po->vy = -15;
				}
				po->y = ev->cinfo.y;
				po->ay = 0;
				po->state = FALLING;
			}
			else
			{
				/* Lose: Knocked! */
				po->vy = -15;
				po->ay = GRAV_ACC;
				po->state = KNOCKED;
				po->timer[1] = 11;
				new_star(gs, po->x, po->y - 20, -5, 3);
				new_star(gs, po->x, po->y - 20, 2, -6);
				new_star(gs, po->x, po->y - 20, 4, 4);
			}
			break;
		  case GROUP_POWERUP:
			switch(ev->obj->score)
			{
			  case POWER_LIFE:
				add_life(gs);
				message(gs, "Extra Life!");
				break;
			  case POWER_BONUS1:
				/* Double or 100k bonus! */
				if(gs->score < 100000)
				{
					inc_score_nobonus(gs, gs->score);
					message(gs, "Double Score!");
				}
				else
				{
					inc_score_nobonus(gs, 100000);
					message(gs, "100 000!");
				}
				break;
			  case POWER_BONUS2:
				inc_score_nobonus(gs, 1000);
				message(gs, "1000!");
				break;
			}
			ev->obj->state = DEAD;
			ev->obj->tilemask = 0;
			ev->obj->hitgroup = 0;
			ev->obj->vy = -20;
			ev->obj->ay = -2;
			break;
		}
		break;
	  case PIG_OFFSCREEN:
		/*
		 * Dead pigs don't care about being off-screen.
		 * A timer is used to remove them, and to continue
		 * the game with a new life.
		 */
		if(DEAD == po->state)
			break;
		if(po->y < 0)	/* Above the playfield is ok. */
			break;
		if(gs->lives)
			message(gs, "Oiiiiiiink!!!");
		else
			message(gs, "Game Over!");
		po->state = DEAD;
		po->timer[2] = 50;
	  default:
		break;
	}
}


static void powerup_handler(PIG_object *po, const PIG_event *ev)
{
	GAMESTATE *gs = (GAMESTATE *)po->owner->userdata;
	switch(ev->type)
	{
	  case PIG_PREFRAME:
		if(DEAD == po->state)
			break;
		po->ax = (po->target - po->vx) * .3;
		po->ay = GRAV_ACC;
		po->image = po->age % 8;
		++po->power;
		break;
	  case PIG_HIT_TILE:
		if(DEAD == po->state)
			break;
		if(po->power > 2)
			po->target = -po->target;
		po->power = 0;
		po->vy = 0;
		po->ay = 0;
		po->x = ev->cinfo.x + po->vx;
		po->y = ev->cinfo.y;
		break;
	  case PIG_OFFSCREEN:
		if(po->y > SCREEN_H || (po->y < -100))
		{
			pig_object_close(po);
			--gs->enemycount;
		}
	  default:
		break;
	}
}


static void star_handler(PIG_object *po, const PIG_event *ev)
{
	switch(ev->type)
	{
	  case PIG_PREFRAME:
		if(po->age >= 8)
			pig_object_close(po);
		else
			po->image = po->age;
	  default:
		break;
	}
}


static void evil_handler(PIG_object *po, const PIG_event *ev)
{
	GAMESTATE *gs = (GAMESTATE *)po->owner->userdata;
	int look_x;
	switch(ev->type)
	{
	  case PIG_PREFRAME:
		if(DEAD == po->state)
			break;
		po->ax = (po->target - po->vx) * .5;
		po->ay = GRAV_ACC;
		po->image = po->age % 16;
		break;
	  case PIG_HIT_TILE:
		if(DEAD == po->state)
			break;
		po->vy = 0;
		po->ay = 0;
		po->x = ev->cinfo.x + po->vx;
		po->y = ev->cinfo.y;
		break;
	  case PIG_OFFSCREEN:
		if(po->y > SCREEN_H)
		{
			pig_object_close(po);
			--gs->enemycount;
		}
		break;
	  case PIG_POSTFRAME:
		if(DEAD == po->state)
			break;
		look_x = 10 + fabs(po->vx * 2);
		if(po->target < 0)
			look_x = -look_x;
		if(!pig_test_map(po->owner, po->x + look_x, po->y + 1))
			po->target = -po->target;
	  default:
		break;
	}
}


static void slime_handler(PIG_object *po, const PIG_event *ev)
{
	GAMESTATE *gs = (GAMESTATE *)po->owner->userdata;
	int look_x;
	switch(ev->type)
	{
	  case PIG_PREFRAME:
		if(DEAD == po->state)
			break;
		po->ax = (po->target - po->vx) * .2;
		po->ay = GRAV_ACC;
		po->image = po->age % 16;
		break;
	  case PIG_HIT_TILE:
		po->vy = -(JUMP_SPEED + GRAV_ACC);
		po->ay = 0;
		po->y = ev->cinfo.y;
		break;
	  case PIG_OFFSCREEN:
		if(po->y > SCREEN_H)
		{
			pig_object_close(po);
			--gs->enemycount;
		}
		break;
	  case PIG_POSTFRAME:
		if(DEAD == po->state)
			break;
		/* Don't bother looking if we're close to a floor. */
		if(pig_test_map_vector(po->owner,
				po->x, po->y,
				po->x, po->y + 48,
				PIG_TOP, NULL))
			break;
		/* Turn around if there's no floor! */
		look_x = 10 + fabs(po->vx * 4);
		if(po->target < 0)
			look_x = -look_x;
		if(!pig_test_map_vector(po->owner,
				po->x + look_x, po->y,
				po->x + look_x, SCREEN_H,
				PIG_TOP, NULL))
			po->target = -po->target;
	  default:
		break;
	}
}


static void chain_head_handler(PIG_object *po, const PIG_event *ev)
{
	GAMESTATE *gs = (GAMESTATE *)po->owner->userdata;
	switch(ev->type)
	{
	  case PIG_PREFRAME:
		po->vx = (po->target - po->x) * .3;
		po->vy = 15 * cos(po->age * .3) - 9;
		if((gs->messages > 1) && (1 == po->state))
			po->timer[1] = 0;
		if(po->timer[1])
			break;
	  case PIG_TIMER1:
		switch(po->state)
		{
		  case 0:
			po->timer[1] = 35;
			++po->state;
			break;
		  case 1:
			po->target = -SCREEN_W;
			po->timer[1] = 50;
			++po->state;
			if(gs->messages > 0)
				--gs->messages;
			break;
		  case 2:
			pig_object_close(po);
			break;
		}
	  default:
		break;
	}
}

static void chain_link_handler(PIG_object *po, const PIG_event *ev)
{
	PIG_object *target = pig_object_find(po, po->target);
	switch(ev->type)
	{
	  case PIG_PREFRAME:
		if(target)
		{
			po->vx = ((target->x + FONT_SPACING) - po->x) * .6;
			po->vy = (target->y - po->y) * .6 - 9;
		}
		else
			pig_object_close(po);
	  default:
		break;
	}
}


/*----------------------------------------------------------
	Accounting (score, lives etc)
----------------------------------------------------------*/

static void add_life(GAMESTATE *gs)
{
	++gs->lives;
	gs->lives_wobble += 10;
	if(gs->lives_wobble > 15)
		gs->lives_wobble = 15;
	gs->lives_wobble_time = 0;
}


static void remove_life(GAMESTATE *gs)
{
	--gs->lives;
	gs->lives_wobble += 10;
	if(gs->lives_wobble > 15)
		gs->lives_wobble = 15;
	gs->lives_wobble_time = 0;
}


static void inc_score_nobonus(GAMESTATE *gs, int v)
{
	int os = gs->score;
	gs->score += v;
	while(v)
	{
		gs->score_wobble += 1;
		v /= 10;
	}
	if(gs->score_wobble > 15)
		gs->score_wobble = 15;
	gs->score_wobble_time = 0;
	if(os / 10000 != gs->score / 10000)
		new_powerup(gs, SCREEN_W / 2, -20, -4, POWER_LIFE);
}

static void inc_score(GAMESTATE *gs, int v)
{
	int os = gs->score;
	inc_score_nobonus(gs, v);
	if(os / 5000 != gs->score / 5000)
		new_powerup(gs, SCREEN_W / 2, -20, 8, POWER_BONUS1);
	else if(os / 1000 != gs->score / 1000)
		new_powerup(gs, SCREEN_W / 2, -20, -6, POWER_BONUS2);
}


static PIG_object *new_player(GAMESTATE *gs)
{
	PIG_object *po;
	if(!gs->lives)
		return NULL;

	po = pig_object_open(gs->pe, SCREEN_W / 2, -50, 1);
	if(!po)
		return NULL;

	remove_life(gs);
	po->ibase = gs->pigframes;
	po->handler = player_handler;
	po->hitmask = GROUP_POWERUP | GROUP_ENEMY;
	return po;
}


static PIG_object *new_powerup(GAMESTATE *gs,
		int x, int y, int speed, POWERUPS type)
{
	PIG_object *po = pig_object_open(gs->pe, x, y, 1);
	if(!po)
		return NULL;

	++gs->enemycount;
	po->score = type;
	po->ibase = gs->icons + 8 * po->score;
	po->target = speed;
	po->handler = powerup_handler;
	po->tilemask = PIG_TOP;
	po->hitgroup = GROUP_POWERUP;
	return po;
}


static PIG_object *new_star(GAMESTATE *gs, int x, int y, int vx, int vy)
{
	PIG_object *po = pig_object_open(gs->pe, x + vx, y + vy, 1);
	if(!po)
		return NULL;

	po->ibase = gs->stars;
	po->ax = -vx * 0.3;
	po->vx = vx * 3;
	po->ay = -vy * 0.3;
	po->vy = vy * 3;
	po->handler = star_handler;
	return po;
}


static PIG_object *new_evil(GAMESTATE *gs,
		int x, int y, int speed)
{
	PIG_object *po = pig_object_open(gs->pe,
			x * TILE_W, y * TILE_H, 1);
	if(!po)
		return NULL;

	++gs->enemycount;
	po->ibase = gs->evil;
	po->target = speed;
	po->handler = evil_handler;
	po->score = 200;
	po->tilemask = PIG_TOP;
	po->hitgroup = GROUP_ENEMY;
	return po;
}


static PIG_object *new_slime(GAMESTATE *gs,
		int x, int y, int speed)
{
	PIG_object *po = pig_object_open(gs->pe,
			x * TILE_W, y * TILE_H, 1);
	if(!po)
		return NULL;

	++gs->enemycount;
	po->ibase = gs->slime;
	po->target = speed;
	po->handler = slime_handler;
	po->score = 300;
	po->tilemask = PIG_TOP;
	po->hitgroup = GROUP_ENEMY;
	return po;
}


static PIG_object *new_chain_head(GAMESTATE *gs,
		int x, int y, int image, int target_x)
{
	PIG_object *po = pig_object_open(gs->pe, x, y, 1);
	if(!po)
		return NULL;

	po->ibase = image;
	po->handler = chain_head_handler;
	po->target = target_x;
	return po;
}


static PIG_object *new_chain_link(GAMESTATE *gs,
		int x, int y, int image, int target)
{
	PIG_object *po = pig_object_open(gs->pe, x, y, 1);
	if(!po)
		return NULL;

	po->ibase = image;
	po->handler = chain_link_handler;
	po->target = target;
	return po;
}


static void message(GAMESTATE *gs, const char *s)
{
	int i = 0;
	const int x = SCREEN_W + FONT_SPACING;
	const int y = MAP_H * TILE_H - 30;
	int tx = (SCREEN_W - ((signed)strlen(s) - 1) * FONT_SPACING) / 2;
	PIG_object *po = NULL;
	while(s[i])
	{
		int c = toupper(s[i]) - 32 + gs->glassfont;
		if(0 == i)
			po = new_chain_head(gs, x, y, c, tx);
		else
			po = new_chain_link(gs, x, y, c, po->id);
		if(!po)
			return;
		++i;
	}
	++gs->messages;
}


static int start_game(GAMESTATE *gs)
{
	if(0 != gs->level)
		return 0;	/* Already playing! --> */

	gs->score = 0;
	gs->lives = 5;

	if(load_level(gs, 1) < 0)
		return -1;

	gs->player = new_player(gs);
	if(!gs->player)
		return -1;

	return 0;
}


/*----------------------------------------------------------
	Input; events and game control keys
----------------------------------------------------------*/
static void handle_input(GAMESTATE *gs, SDL_Event *ev)
{
	switch(ev->type)
	{
	  case SDL_MOUSEBUTTONUP:
		break;
	  case SDL_KEYDOWN:
		switch(ev->key.keysym.sym)
		{
		  case SDLK_UP:
			gs->jump = 3;
			break;
		  case SDLK_F1:
			gs->pe->interpolation = !gs->pe->interpolation;
			if(gs->pe->interpolation)
				message(gs, "Interpolation: ON");
			else
				message(gs, "Interpolation: OFF");
			break;
		  case SDLK_F2:
			gs->pe->direct = !gs->pe->direct;
			if(gs->pe->direct)
				message(gs, "Rendering: Direct");
			else
				message(gs, "Rendering: Buffered");
			break;
		  case SDLK_F3:
			gs->pe->show_dirtyrects = !gs->pe->show_dirtyrects;
			if(gs->pe->show_dirtyrects)
				message(gs, "Dirtyrects: ON");
			else
				message(gs, "Dirtyrects: OFF");
			break;
		  case SDLK_F4:
			gs->nice = !gs->nice;
			if(gs->nice)
				message(gs, "Be Nice: ON");
			else
				message(gs, "Be Nice: OFF");
			break;
		  case SDLK_SPACE:
			start_game(gs);
		  default:
			break;
		}
		break;
	  case SDL_KEYUP:
		switch(ev->key.keysym.sym)
		{
		  case SDLK_ESCAPE:
			gs->running = 0;
		  default:
			break;
		}
		break;
	  case SDL_QUIT:
		gs->running = 0;
		break;
	}
}


static void handle_keys(GAMESTATE *gs)
{
}


static int break_received = 0;

#ifndef	RETSIGTYPE
#define	RETSIGTYPE void
#endif
static RETSIGTYPE breakhandler(int sig)
{
	/* For platforms that drop the handlers on the first signal... */
	signal(SIGTERM, breakhandler);
	signal(SIGINT, breakhandler);
	break_received = 1;
#if (RETSIGTYPE != void)
	return 0;
#endif
}


/*----------------------------------------------------------
	main()
----------------------------------------------------------*/
int main(int argc, char* argv[])
{
	SDL_Surface *screen;
	GAMESTATE *gs;
	int i;
	int bpp = 0;
	int last_tick, start_time, end_time;
	int dashframe;
	float logic_fps = 20.0;
	int flags = SDL_DOUBLEBUF | SDL_HWSURFACE;

	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
	signal(SIGTERM, breakhandler);
	signal(SIGINT, breakhandler);

	for(i = 1; i < argc; ++i)
	{
		if(strncmp(argv[i], "-s", 2) == 0)
			flags &= ~SDL_DOUBLEBUF;
		else if(strncmp(argv[i], "-f", 2) == 0)
			flags |= SDL_FULLSCREEN;
		else
			bpp = atoi(&argv[i][1]);
	}

	screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, bpp, flags);
	if(!screen)
	{
		fprintf(stderr, "Failed to open screen!\n");
		return 1;
	}

	SDL_WM_SetCaption("Fixed Rate Pig", "Pig");
	SDL_ShowCursor(0);

	gs = init_all(screen);
	if(!gs)
		return 1;

	gs->keys = SDL_GetKeyState(&i);

	gs->logic_frames = 0;
	gs->rendered_frames = 0;
	gs->pe->before_objects = before_objects;

	pig_start(gs->pe, 0);
	gs->refresh_screen = gs->pe->pages;
	start_time = last_tick = SDL_GetTicks();
	while(gs->running)
	{
		int tick;
		float frames, dt;
		SDL_Event ev;

		/* Handle input */
		while(SDL_PollEvent(&ev) > 0)
			handle_input(gs, &ev);
		handle_keys(gs);
		if(break_received)
			gs->running = 0;

		/* Calculate time since last update */
		tick = SDL_GetTicks();
		dt = (tick - last_tick) * 0.001;
		frames = dt * logic_fps;

		/* Run the game logic */
		pig_animate(gs->pe, frames);

		/*
		 * Limit the dashboard frame rate to 15 fps
		 * when there's no wobbling going on.
		 *
		 * The 'dashframe' deal is about keeping the
		 * pages in sync on a double buffered display.
		 */
		if(gs->lives_wobble || gs->score_wobble ||
				(gs->dashboard_time > 1.0/15.0))
		{
			dashframe = gs->pe->pages;
			gs->dashboard_time = 0;
		}
		if(dashframe)
		{
			--dashframe;
			dashboard(gs);
		}

		/* Update sprites */
		if(gs->refresh_screen)
		{
			--gs->refresh_screen;
			pig_refresh_all(gs->pe);
		}
		else
			pig_refresh(gs->pe);

		/* Make the new frame visible */
		pig_flip(gs->pe);

		/* Update statistics, timers and stuff */
		++gs->rendered_frames;
		gs->lives_wobble_time += dt;
		gs->score_wobble_time += dt;
		gs->dashboard_time += dt;

		last_tick = tick;
		if(gs->nice)
			SDL_Delay(10);
	}

	/* Print some statistics */
	end_time = SDL_GetTicks();
	i = end_time - start_time;
	printf("          Total time running: %d ms\n", i);
	if(!i)
		i = 1;
	printf("Average rendering frame rate: %.2f fps\n",
			gs->rendered_frames * 1000.0 / i);
	printf("    Average logic frame rate: %.2f fps\n",
			gs->logic_frames * 1000.0 / i);

	pig_close(gs->pe);
	return 0;
}
