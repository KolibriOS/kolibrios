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

#ifndef	PIG_ENGINE_H
#define	PIG_ENGINE_H

#include "SDL.h"
#include <math.h>
#ifndef M_PI
#	define M_PI 3.14159265358979323846	/* pi */
#endif
#include "dirty.h"


/*----------------------------------------------------------
	Game Engine
----------------------------------------------------------*/

typedef struct PIG_object PIG_object;
typedef struct PIG_engine PIG_engine;


/* Interpolated point */
typedef struct PIG_ipoint
{
	/* From the last logic frame: */
	float	ox, oy;		/* Position */

	/* From the last/current rendered frame: */
	int	gimage;		/* Sprite frame index */
	float	gx, gy;		/* Interpolated position */
} PIG_ipoint;


/*
 * Game logic events
 *
 * PREFRAME:
 *	Occurs once per logic frame, before collision and
 *	off-screen detection, and before timer handlers.
 *
 * TIMERx:
 *	Occurs whenever timer x expires. Timers are one-
 *	shot, but can be reloaded by the handler for
 *	periodic action. Timer events are handled before
 *	before collision and off-screen detection.
 *
 * HIT_TILE:
 *	Occurs when the hot-spot of an object hits a
 *	marked side of a tile, and the corresponding bit
 *	in 'tilemask' is set.
 *
 * HIT_OBJECT:
 *	Occurs when the collision circle of an object
 *	intersects the collision circle of another object,
 *	provided one or more bits in 'hitgroup' of the
 *	other object matches bits in 'hitmask'.
 *
 * OFFSCREEN:
 *	Occurs when an object is off-screen. This takes
 *	in account the hot-spot and bounding rectangle of
 *	the current sprite frame.
 *
 * POSTFRAME:
 *	Occurs once per logic frame, after collision
 *	detection, off-screen detection and all other
 *	events.
 *
 */
#define	PIG_TIMERS	3
typedef enum
{
	PIG_PREFRAME,
	PIG_TIMER0,
	PIG_TIMER1,
	PIG_TIMER2,
	PIG_HIT_TILE,
	PIG_HIT_OBJECT,
	PIG_OFFSCREEN,
	PIG_POSTFRAME
} PIG_events;


typedef enum
{
	PIG_NONE =	0,

	/* Bit positions */
	PIG_TOP_B =	0,
	PIG_BOTTOM_B =	1,
	PIG_LEFT_B =	2,
	PIG_RIGHT_B =	3,

	/* Masks */
	PIG_TOP =	1 << PIG_TOP_B,
	PIG_BOTTOM =	1 << PIG_BOTTOM_B,
	PIG_LEFT =	1 << PIG_LEFT_B,
	PIG_RIGHT =	1 << PIG_RIGHT_B,

	/* Combined masks */
	PIG_TL =	PIG_TOP | PIG_LEFT,
	PIG_TR =	PIG_TOP | PIG_RIGHT,
	PIG_BL =	PIG_BOTTOM | PIG_LEFT,
	PIG_BR =	PIG_BOTTOM | PIG_RIGHT,
	PIG_ALL =	0xf,
} PIG_sides;


typedef enum
{
	PIG_UNCHANGED =	-10000000,
	PIG_MIN =	-10000001,
	PIG_CENTER =	-10000002,
	PIG_MAX =	-10000003
} PIG_values;


/* Collision info */
typedef struct
{
	float		ff;	/* Fractional frame */
	int		x, y;	/* Exact position */
	PIG_sides	sides;	/* Side of tile hit */
} PIG_cinfo;


typedef struct PIG_event
{
	PIG_events	type;

	/* For HIT_TILE, HIT_OBJECT and OFFSCREEN: */
	PIG_cinfo	cinfo;	/* Detailed collision info */

	/* For HIT_OBJECT: */
	PIG_object	*obj;	/* Which object? */
}  PIG_event;


/* Logic object */
struct PIG_object
{
	PIG_engine	*owner;
	PIG_object	*next, *prev;

	int		id;		/* Unique ID. 0 means "free". */

	int		ibase;		/* Sprite frame base index */
	int		image;		/* Sprite frame offset */
	float		x, y;		/* Position */
	float		vx, vy;		/* Speed */
	float		ax, ay;		/* Acceleration */
	PIG_ipoint	ip;
	int		tilemask;	/* Sprite/tile mask [PIG_ALL] */

	int		hitmask;	/* Sprite/sprite mask [0] */
	int		hitgroup;	/* Sprite/sprite group [0] */

	int		timer[PIG_TIMERS];	/* Down-counting timers */
	int		age;		/* Age timer (logic frames) */

	int		score;
	int		power;
	int		target;
	int		state;

	void (*handler)(PIG_object *po, const PIG_event *ev);

	void		*userdata;
};


/* Level map */
typedef struct PIG_map
{
	PIG_engine	*owner;

	int		w, h;		/* Size of map (tiles) */
	unsigned char	*map;		/* 2D aray of tile indices */
	unsigned char	*hit;		/* 2D aray of collision flags */

	int		tw, th;		/* Size of one tile (pixels) */
	SDL_Surface	*tiles;		/* Tile palette image */
	unsigned char	hitinfo[256];	/* Collision info for the tiles */
} PIG_map;


/* Sprite frame */
typedef struct PIG_sprite
{
	int		w, h;		/* Size of sprite (pixels) */
	int		hotx, hoty;	/* Hot-spot offset (pixels) */
	int		radius;		/* Collision zone radius (pixels) */
	SDL_Surface	*surface;
} PIG_sprite;

/* Engine */
struct PIG_engine
{
	/* Video stuff */
	SDL_Surface	*screen;
	SDL_Surface	*buffer;	/* For h/w surface displays */
	SDL_Surface	*surface;	/* Where to render to */
	int		pages;		/* # of display VRAM buffers */
	SDL_Rect	view;		/* Viewport pos & size (pixels) */
	int		page;		/* Current page (double buffer) */
	PIG_dirtytable	*pagedirty[2];	/* One table for each page */
	PIG_dirtytable	*workdirty;	/* The work dirtytable */

	/* "Live" switches */
	int		interpolation;
	int		direct;		/* 1 ==> render directly to screen */
	int		show_dirtyrects;

	/* Time */
	double		time;		/* Logic time (frames) */
	int		frame;		/* Logic time; integer part */

	/* Background graphics */
	PIG_map		*map;

	/* Sprites and stuff */
	PIG_object	*objects;
	PIG_object	*object_pool;
	int		object_id_counter;
	int		nsprites;
	PIG_sprite	**sprites;

	/* Logic frame global handlers */
	void (*before_objects)(PIG_engine *pe);
	void (*after_objects)(PIG_engine *pe);

	/* Space for user data */
	void		*userdata;
};


/*
 * Engine
 */
PIG_engine *pig_open(SDL_Surface *screen);
void pig_close(PIG_engine *pe);

/* Set viewport size and position */
void pig_viewport(PIG_engine *pe, int x, int y, int w, int h);

/* Start engine at logic time 'frame' */
void pig_start(PIG_engine *pe, int frame);

/*
 * Load a sprite palette image. The image is chopped up into
 * sprites, based on 'sw' and 'sh', and added as new frames
 * in the sprite bank. Default values:
 *	Hot-spot:		(sw/2, sh/2)
 *	Collision radius:	0.2 * (sw + sh)
 *
 * Passing 0 for 'sw' and/or 'sh' makes pig_sprites() take
 * the respective value from the image width and/or height.
 *
 * Returns the index of the first frame loaded.
 */
int pig_sprites(PIG_engine *pe, const char *filename, int sw, int sh);

/* Set hot-spot of sprite 'frame' to (hotx, hoty) */
int pig_hotspot(PIG_engine *pe, int frame, int hotx, int hoty);

/* Set sprite/sprite collision zone radius of 'frame' */
int pig_radius(PIG_engine *pe, int frame, int radius);

/* Advance logic time by 'frames' logic frames */
void pig_animate(PIG_engine *pe, float frames);

/*
 * Manually add a dirtyrect for pig_refresh().
 * 'dr' can be outside the engine viewport.
 */
void pig_dirty(PIG_engine *pe, SDL_Rect *dr);

/*
 * Do what's needed to deal with the dirtyrects
 * and then make the new frame visible.
 */
void pig_flip(PIG_engine *pe);

/*
 * Refresh the viewport and any additional dirtyrects.
 *
 * Note that this does not refresh the entire viewport;
 * only the areas that have actually changed!
 */
void pig_refresh(PIG_engine *pe);

/*
 * Refresh the whole viewport, including sprites.
 */
void pig_refresh_all(PIG_engine *pe);

/* Render a sprite "manually", bypassing the engine */
void pig_draw_sprite(PIG_engine *pe, int frame, int x, int y);

/*
 * Get the collision flags for the tile at (x, y),
 * where the unit of x and y is pixels. The return
 * is the PIG_sides flags for the tile, or PIG_NONE
 * if (x, y) is outside the map.
 */
int pig_test_map(PIG_engine *pe, int x, int y);

/*
 * Find the first "collidable" tile side when going from
 * (x1, y1) to (x2, y2). 'mask' determines which tile sides
 * are considered for collisions.
 *
 * Returns the side(s) hit, if any tile was hit. If the return
 * is non-zero, the PIG_cinfo struct at 'ci' contains detailed
 * information about the collision.
 */
int pig_test_map_vector(PIG_engine *pe, int x1, int y1, int x2, int y2,
		int mask, PIG_cinfo *ci);


/*
 * Map
 */
PIG_map *pig_map_open(PIG_engine *pe, int w, int h);
void pig_map_close(PIG_map *pm);

/* Load a tile palette image */
int pig_map_tiles(PIG_map *pm, const char *filename, int tw, int th);

/*
 * Set tile collision info for 'count' tiles, starting at
 * 'first'. Each tile in the tile palette has a set of
 * PIG_sides flags that determine which sides the tile are
 * considered for sprite/map collisions.
 */
void pig_map_collisions(PIG_map *pm, unsigned first, unsigned count,
		PIG_sides sides);

/*
 * Load a map from a string (one byte/tile). 'trans'
 * is a string used for translating 'data' into integer
 * tile indices. Each position in 'trans' corresponds
 * to one tile in the tile palette.
 */
int pig_map_from_string(PIG_map *pm, const char *trans, const char *data);


/*
 * Object
 */

/*
 * Create an object with the initial position (x, y). If
 * 'last' is 1, the object will end up last in the
 * processing and rendering order, otherwise, first.
 *
 * Note that relative processing order is very important
 * to objects that chase each other and stuff like that!
 * If they're placed in the "wrong" order, the tracking
 * objects get an extra frame of reaction time, which is
 * annoying if it's not what you intend.
 */
PIG_object *pig_object_open(PIG_engine *pe, int x, int y, int last);

/*
 * Delete an object.
 *
 * Note that objects are never actually deleted. Instead,
 * they are placed in a free pool, where pig_object_open()
 * looks for objects to recycle.
 *
 * In fact, they are not even freed when you ask for it,
 * but rather kept around until the next rendered frame,
 * so they can be removed from the screen correctly.
 */
void pig_object_close(PIG_object *po);

/*
 * Close all objects.
 */
void pig_object_close_all(PIG_engine *pe);

/*
 * Find object by 'id', starting at object 'start'.
 *
 * The search starts at 'start' and is done in both
 * directions in parallel, assuming that the matching
 * object is near 'start' in the list. (It usually is
 * when dealing with linked objects.)
 *
 * Returns NULL if the object was not found.
 */
PIG_object *pig_object_find(PIG_object *start, int id);

#endif	/* PIG_ENGINE_H */
