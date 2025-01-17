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

#include <stdlib.h>
#include <string.h>
#include "engine.h"
#include "SDL_image.h"

/* Size of sprite frame table */
#define	PIG_MAX_SPRITES		1024


/*
 * Actually remove an objects. Used internally,
 * to remove objects that have been marked for
 * destruction.
 */
static void close_object(PIG_object *po);


/*----------------------------------------------------------
	Engine
----------------------------------------------------------*/
PIG_engine *pig_open(SDL_Surface *screen)
{
	PIG_engine *pe = (PIG_engine *)calloc(1, sizeof(PIG_engine));
	if(!pe)
		return NULL;

	pe->screen = screen;
	if(!pe->screen)
	{
		pig_close(pe);
		return NULL;
	}
	if((pe->screen->flags & SDL_HWSURFACE) == SDL_HWSURFACE)
	{
		pe->buffer = SDL_CreateRGBSurface(SDL_SWSURFACE,
				screen->w, screen->h,
				screen->format->BitsPerPixel,
				screen->format->Rmask,
				screen->format->Gmask,
				screen->format->Bmask,
				screen->format->Amask);
		if(!pe->buffer)
		{
			pig_close(pe);
			return NULL;
		}
		pe->surface = pe->buffer;
	}
	else
		pe->surface = screen;

	pe->pages = 1 + ((screen->flags & SDL_DOUBLEBUF) == SDL_DOUBLEBUF);

	pe->interpolation = 1;
	pe->time = 0.0;
	pe->view.w = pe->surface->w;
	pe->view.h = pe->surface->h;

	pe->sprites = (PIG_sprite **)calloc(PIG_MAX_SPRITES,
			sizeof(PIG_sprite *));
	if(!pe->sprites)
	{
		pig_close(pe);
		return NULL;
	}

	pe->pagedirty[0] = pig_dirty_open(128);
	pe->workdirty = pig_dirty_open(256);
	if(!pe->pagedirty[0] || !pe->workdirty)
	{
		pig_close(pe);
		return NULL;
	}
	if(pe->pages > 1)
	{
		pe->pagedirty[1] = pig_dirty_open(128);
		if(!pe->pagedirty[1])
		{
			pig_close(pe);
			return NULL;
		}
	}

	return pe;
}


void pig_close(PIG_engine *pe)
{
	if(pe->sprites)
	{
		int i;
		for(i = 0; i < pe->nsprites; ++i)
			if(pe->sprites[i])
			{
				if(pe->sprites[i]->surface)
					SDL_FreeSurface(pe->sprites[i]->surface);
				free(pe->sprites[i]);
			}
		free(pe->sprites);
	}
	while(pe->objects)
		close_object(pe->objects);
	if(pe->map)
		pig_map_close(pe->map);
	if(pe->buffer)
		SDL_FreeSurface(pe->buffer);
	if(pe->pagedirty[0])
		pig_dirty_close(pe->pagedirty[0]);
	if(pe->pagedirty[1])
		pig_dirty_close(pe->pagedirty[1]);
	if(pe->workdirty)
		pig_dirty_close(pe->workdirty);
	free(pe);
}


void pig_viewport(PIG_engine *pe, int x, int y, int w, int h)
{
	pe->view.x = x;
	pe->view.y = y;
	pe->view.w = w;
	pe->view.h = h;
}


int pig_sprites(PIG_engine *pe, const char *filename, int sw, int sh)
{
	int x, y, count, handle;
	SDL_Surface *tmp = IMG_Load(filename);
	if(!tmp)
	{
		fprintf(stderr, "Could not load '%s'!\n", filename);
		return -1;
	}

	handle = pe->nsprites;

	if(!sw)
		sw = tmp->w;
	if(!sh)
		sh = tmp->h;

	/* Disable blending, so we get the alpha channel COPIED! */
	SDL_SetAlpha(tmp, 0, 0);

	count = 0;
	for(y = 0; y <= tmp->h - sh; y += sh)
		for(x = 0; x <= tmp->w - sw; x += sw)
		{
			SDL_Rect r;
			SDL_Surface *tmp2;
			PIG_sprite *s;
			if(pe->nsprites >= PIG_MAX_SPRITES)
			{
				fprintf(stderr, "Sprite bank full!\n");
				return -1;
			}
			s = (PIG_sprite *)calloc(1, sizeof(PIG_sprite));
			if(!s)
				return -1;
			s->w = sw;
			s->h = sh;
			s->hotx = sw / 2;
			s->hoty = sh / 2;
			s->radius = (sw + sh) / 5;
			tmp2 = SDL_CreateRGBSurface(SDL_SWSURFACE,
					sw, sh, 32,
					0xff000000, 0x00ff0000,
					0x0000ff00, 0x000000ff);
			SDL_SetAlpha(tmp2, 0, 0);
			r.x = x;
			r.y = y;
			r.w = sw;
			r.h = sh;
			SDL_BlitSurface(tmp, &r, tmp2, NULL);
			SDL_SetAlpha(tmp2, SDL_SRCALPHA | SDL_RLEACCEL,
					SDL_ALPHA_OPAQUE);
			s->surface = SDL_DisplayFormatAlpha(tmp2);
			if(!s->surface)
			{
				fprintf(stderr, "Could not convert sprite %d"
						" of '%s'!\n",
						count, filename);
				return -1;
			}
			SDL_FreeSurface(tmp2);
			pe->sprites[pe->nsprites] = s;
			++pe->nsprites;
			++count;
		}

	SDL_FreeSurface(tmp);
	return handle;
}


int pig_hotspot(PIG_engine *pe, int frame, int hotx, int hoty)
{
	if((frame < 0 ) || (frame >= pe->nsprites))
		return -1;

	switch(hotx)
	{
	  case PIG_UNCHANGED:
		break;
	  case PIG_MIN:
		pe->sprites[frame]->hotx = 0;
		break;
	  case PIG_CENTER:
		pe->sprites[frame]->hotx = pe->sprites[frame]->w / 2;
		break;
	  case PIG_MAX:
		pe->sprites[frame]->hotx = pe->sprites[frame]->w;
		break;
	  default:
		pe->sprites[frame]->hotx = hotx;
		break;
	}
	switch(hoty)
	{
	  case PIG_UNCHANGED:
		break;
	  case PIG_MIN:
		pe->sprites[frame]->hoty = 0;
		break;
	  case PIG_CENTER:
		pe->sprites[frame]->hoty = pe->sprites[frame]->h / 2;
		break;
	  case PIG_MAX:
		pe->sprites[frame]->hoty = pe->sprites[frame]->h;
		break;
	  default:
		pe->sprites[frame]->hoty = hoty;
		break;
	}
	return 0;
}


int pig_radius(PIG_engine *pe, int frame, int radius)
{
	if((frame < 0 ) || (frame >= pe->nsprites))
		return -1;

	pe->sprites[frame]->radius = radius;
	return 0;
}


void pig_start(PIG_engine *pe, int frame)
{
	PIG_object *po = pe->objects;
	pe->time = (double)frame;
	pe->frame = frame;
	while(po)
	{
		po->ip.gx = po->ip.ox = po->x;
		po->ip.gy = po->ip.oy = po->y;
		po->ip.gimage = po->ibase + po->image;
		po = po->next;
	}
}


static void run_timers(PIG_engine *pe, PIG_object *po)
{
	int i;
	for(i = 0; i < PIG_TIMERS; ++i)
		if(po->timer[i])
		{
			--po->timer[i];
			if(!po->timer[i])
			{
				PIG_event ev;
				ev.type = PIG_TIMER0 + i;
				po->handler(po, &ev);
				if(!po->id)
					return;
			}
		}
}


static void test_offscreen(PIG_engine *pe, PIG_object *po, PIG_sprite *s)
{
	PIG_event ev;
	int hx, hy, w, h;
	if(s)
	{
		hx = s->hotx;
		hy = s->hoty;
		w = s->w;
		h = s->h;
	}
	else
		hx = hy = w = h = 0;
	ev.cinfo.sides = (po->y - hy < -h) << PIG_TOP_B;
	ev.cinfo.sides |= (po->y - hy >= pe->view.h) << PIG_BOTTOM_B;
	ev.cinfo.sides |= (po->x - hx < -w) << PIG_LEFT_B;
	ev.cinfo.sides |= (po->x - hx >= pe->view.w) << PIG_RIGHT_B;
	if(ev.cinfo.sides)
	{
		float dx = po->x - po->ip.ox;
		float dy = po->y - po->ip.oy;
		if(ev.cinfo.sides & PIG_TOP)
		{
			ev.cinfo.y = 0;
			if(dy)
				ev.cinfo.x = po->ip.ox - dx * po->ip.oy / dy;
		}
		else if(ev.cinfo.sides & PIG_BOTTOM)
		{
			ev.cinfo.y = pe->view.h - 1;
			if(dy)
				ev.cinfo.x = po->ip.ox + dx *
						(ev.cinfo.y - po->ip.oy) / dy;
		}
		if(ev.cinfo.sides & PIG_LEFT)
		{
			ev.cinfo.x = 0;
			if(dx)
				ev.cinfo.y = po->ip.oy - dy * po->ip.ox / dx;
		}
		else if(ev.cinfo.sides & PIG_RIGHT)
		{
			ev.cinfo.x = pe->view.w - 1;
			if(dx)
				ev.cinfo.y = po->ip.oy + dy *
						(ev.cinfo.x - po->ip.ox) / dx;
		}
		ev.type = PIG_OFFSCREEN;
		po->handler(po, &ev);
	}
}


/* Test for stationary sprite/sprite collision */
static void sprite_sprite_one(PIG_object *po, PIG_object *po2, float t, float hitdist)
{
	float dx, dy, dsquare;
	PIG_event ev;
	int sides;
	float ix = po->ip.ox * (1 - t) + po->x * t;
	float iy = po->ip.oy * (1 - t) + po->y * t;
	float ix2 = po2->ip.ox * (1 - t) + po2->x * t;
	float iy2 = po2->ip.oy * (1 - t) + po2->y * t;
	dx = ix - ix2;
	dy = iy - iy2;
	dsquare = dx*dx + dy*dy;
	if(dsquare >= hitdist*hitdist)
		return;		/* Nothing... --> */

	if(fabs(dsquare) < 1)
		sides = PIG_ALL;
	else
	{
		float d = sqrt(dsquare);
		dx /= d;
		dy /= d;
		if(dx < -0.707)
			sides = PIG_LEFT;
		else if((dx > 0.707))
			sides = PIG_RIGHT;
		else
			sides = 0;
		if(dy < -0.707)
			sides |= PIG_TOP;
		else if((dy > 0.707))
			sides |= PIG_BOTTOM;
	}
	ev.type = PIG_HIT_OBJECT;
	ev.cinfo.ff = 0.0;

	ev.cinfo.x = ix;
	ev.cinfo.y = iy;
	ev.cinfo.sides = sides;
	if(po->hitmask & po2->hitgroup)
	{
		ev.obj = po2;
		po->handler(po, &ev);
	}

	if(po2->id && (po2->hitmask & po->hitgroup))
	{
		int s;
		ev.cinfo.x = ix2;
		ev.cinfo.y = iy2;
		s = ((sides >> PIG_LEFT_B) & 1) << PIG_RIGHT_B;
		s |= ((sides >> PIG_RIGHT_B) & 1) << PIG_LEFT_B;
		s |= ((sides >> PIG_TOP_B) & 1) << PIG_BOTTOM_B;
		s |= ((sides >> PIG_BOTTOM_B) & 1) << PIG_TOP_B;
		ev.cinfo.sides = s;
		ev.obj = po;
		po2->handler(po2, &ev);
	}
}


/*
 * Check 'po' against all subsequent objects in the list.
 * The testing is step size limited so that neither object
 * moves more than 25% of the collision distance between tests.
 * (25% should be sufficient for correct direction flags.)
 */
static void test_sprite_sprite(PIG_engine *pe, PIG_object *po, PIG_sprite *s)
{
	int image;
	PIG_object *po2, *next2;
	for(po2 = po->next; po2; po2 = next2)
	{
		float hitdist, d, dmax, t, dt;
		next2 = po2->next;
		if(!po->id || !po2->id)
			break;

		/* Check collision groups and masks */
		if(!(po->hitmask & po2->hitgroup) &&
				!(po2->hitmask & po->hitgroup))
			continue;

		/* Calculate minimum distance */
		hitdist = s ? s->radius : 0;
		image = po2->ibase + po2->image;
		if((image >= 0) && (image < pe->nsprites))
			hitdist += pe->sprites[image]->radius;
		if(hitdist < 1)
			hitdist = 1;

		/* Calculate number of testing steps */
		dmax = fabs(po->ip.ox - po->x);
		d = fabs(po->ip.oy - po->y);
		dmax = d > dmax ? d : dmax;
		d = fabs(po2->ip.ox - po2->x);
		dmax = d > dmax ? d : dmax;
		d = fabs(po2->ip.oy - po2->y);
		dmax = d > dmax ? d : dmax;
		if(dmax > 1)
			dt = hitdist / (dmax * 4);
		else
			dt = 1;

		/* Sweep test! */
		for(t = 0; t < 1; t += dt)
			sprite_sprite_one(po, po2, t, hitdist);
	}
}


/*
 * Returns a non-zero value if the tile at (x, y) is marked for
 * collisions on the side indicated by 'mask'.
 */
static __inline__ int check_tile(PIG_map *m, int x, int y, int mask)
{
	int mx, my;
	/*
	 * Must check < 0 first! (Division rounds
	 * towards zero - not downwards.)
	 */
	if(x < 0 || y < 0)
		return PIG_NONE;

	mx = x / m->tw;
	my = y / m->th;
	if(mx >= m->w || my >= m->h)
		return PIG_NONE;

	return m->hit[my * m->w + mx] & mask;
}


int pig_test_map(PIG_engine *pe, int x, int y)
{
	int mx, my;
	if(x < 0 || y < 0)
		return PIG_NONE;

	mx = x / pe->map->tw;
	my = y / pe->map->th;
	if(mx >= pe->map->w || my >= pe->map->h)
		return PIG_NONE;

	return pe->map->hit[my * pe->map->w + mx];
}


/*
 * Simple implementation that checks only for top edge collisions.
 * (Full top/bottom/left/right checks with proper handling of
 * corners and rows of tiles is a lot more complicated, so I'll
 * leave that out for now, rather than hacking something simple
 * but incorrect.)
 */
int pig_test_map_vector(PIG_engine *pe, int x1, int y1, int x2, int y2,
		int mask, PIG_cinfo *ci)
{
	PIG_cinfo lci;
	PIG_map *m = pe->map;
	int x, y;
	int dist = 2000000000L;
	if(!ci)
		ci = &lci;
	ci->sides = 0;
	if((mask & PIG_TOP) && (y1 < y2))
	{
		/* Test for tiles that can be hit from the top */
		for(y = y1 + m->th - y1 % m->th; y <= y2; y += m->th)
		{
			x = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
			if(check_tile(m, x, y + 1, PIG_TOP))
			{
				dist = (x-x1) * (x-x1) + (y-y1) * (y-y1);
				ci->x = x;
				ci->y = y - 1;
				ci->sides |= PIG_TOP;
				break;
			}
		}
	}
	if(ci->sides)
		ci->ff = sqrt((x2 - x1) * (x2 - x1) +
				(y2 - y1) * (y2 - y1) / dist);
	return ci->sides;
}


static void test_sprite_map(PIG_engine *pe, PIG_object *po, PIG_sprite *s)
{
	PIG_event ev;
	if(pig_test_map_vector(pe, po->ip.ox, po->ip.oy, po->x, po->y,
			po->tilemask, &ev.cinfo))
	{
		ev.type = PIG_HIT_TILE;
		po->handler(po, &ev);
	}
}


static void run_logic(PIG_engine *pe)
{
	PIG_object *po, *next;
	int image;

	/* Shift logic coordinates */
	for(po = pe->objects; po; po = po->next)
	{
		po->ip.ox = po->x;
		po->ip.oy = po->y;
	}

	if(pe->before_objects)
		pe->before_objects(pe);

	for(po = pe->objects; po; po = next)
	{
		PIG_event ev;
		/*
		 * We must grab the next pointer before
		 * we call any event handlers, as they
		 * may cause objects to remove themselves!
		 */
		next = po->next;
		ev.type = PIG_PREFRAME;
		po->handler(po, &ev);
	}

	for(po = pe->objects; po; po = next)
	{
		PIG_sprite *s;
		next = po->next;
		image = po->ibase + po->image;
		if((image >= 0) && (image < pe->nsprites))
			s = pe->sprites[image];
		else
			s = NULL;

		/* Move! */
		po->vx += po->ax;
		po->vy += po->ay;
		po->x += po->vx;
		po->y += po->vy;

		/* Check and handle events */
		if(po->handler)
		{
			run_timers(pe, po);
			if(po->id)
				test_offscreen(pe, po, s);
			if(po->id && (po->hitmask || po->hitgroup))
				test_sprite_sprite(pe, po, s);
			if(po->id && po->tilemask)
				test_sprite_map(pe, po, s);

		}
	}

	for(po = pe->objects; po; po = next)
	{
		next = po->next;
		if(po->id)
		{
			PIG_event ev;
			ev.type = PIG_POSTFRAME;
			po->handler(po, &ev);
			++po->age;
		}
	}

	if(pe->after_objects)
		pe->after_objects(pe);
}


void pig_animate(PIG_engine *pe, float frames)
{
	/* Advance logic time */
	int i = floor(pe->time + frames) - floor(pe->time);
	while(i--)
	{
		run_logic(pe);
		++pe->frame;
	}
	pe->time += frames;
}


void pig_dirty(PIG_engine *pe, SDL_Rect *dr)
{
	SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = pe->surface->w;
	r.h = pe->surface->h;
	if(dr)
		pig_intersectrect(dr, &r);
	if(r.w && r.h)
		pig_dirty_add(pe->pagedirty[pe->page], &r);
}


static void tile_area(PIG_engine *pe, SDL_Rect *r)
{
	SDL_Rect cr;
	int x, y, startx, starty, maxx, maxy, tilesperrow;
	cr = *r;
	cr.x += pe->view.x;
	cr.y += pe->view.y;
	SDL_SetClipRect(pe->surface, &cr);

	startx = r->x / pe->map->tw;
	starty = r->y / pe->map->th;
	maxx = (r->x + r->w + pe->map->tw - 1) / pe->map->tw;
	maxy = (r->y + r->h + pe->map->th - 1) / pe->map->th;
	if(maxx > pe->map->w - 1)
		maxx = pe->map->w - 1;
	if(maxy > pe->map->h - 1)
		maxy = pe->map->h - 1;
	tilesperrow = pe->map->tiles->w / pe->map->tw;

	for(y = starty; y <= maxy; ++y)
		for(x = startx; x <= maxx; ++x)
		{
			SDL_Rect from, to;
			int c = pe->map->map[y * pe->map->w + x];
			from.x = c % tilesperrow * pe->map->tw;
			from.y = c / tilesperrow * pe->map->th;
			from.w = pe->map->tw;
			from.h = pe->map->th;
			to.x = pe->view.x + x * pe->map->tw;
			to.y = pe->view.y + y * pe->map->th;
			SDL_BlitSurface(pe->map->tiles, &from,
					pe->surface, &to);
		}
}


void remove_sprites(PIG_engine *pe)
{
	SDL_Rect r;
	PIG_sprite *s;
	PIG_object *po, *next;

	/*
	 * Remove all objects, using the information that
	 * remains from the last frame. The actual removal
	 * is done by drawing over the sprites with tiles
	 * from the map.
	 *
	 * We assume that most objects don't overlap. If
	 * they do that a lot, we could use a "dirty map"
	 * to avoid rendering the same tiles multiple times
	 * in the overlapping areas.
	 */
	for(po = pe->objects; po; po = next)
	{
		next = po->next;
		if((po->ip.gimage < 0) || (po->ip.gimage >= pe->nsprites))
			continue;
		s = pe->sprites[po->ip.gimage];
		r.x = po->ip.gx - s->hotx;
		r.y = po->ip.gy - s->hoty;
		r.w = s->w;
		r.h = s->h;
		pig_intersectrect(&pe->view, &r);
		if(r.w && r.h)
			tile_area(pe, &r);

		/*
		 * Delete dead objects *after* they've
		 * been removed from the rendering buffer!
		 */
		if(!po->id)
			close_object(po);
	}
}


void draw_sprites(PIG_engine *pe)
{
	PIG_dirtytable *pdt;
	PIG_sprite *s;
	PIG_object *po;
	float fframe = pe->time - floor(pe->time);
	SDL_SetClipRect(pe->surface, &pe->view);

	/* Swap the work and display/back page dirtytables */
	pdt = pe->workdirty;
	pe->workdirty = pe->pagedirty[pe->page];
	pe->pagedirty[pe->page] = pdt;

	/* Clear the display/back page dirtytable */
	pdt->count = 0;

	/* Update positions and render all objects */
	po = pe->objects;
	while(po)
	{
		/* Calculate graphic coordinates */
		if(pe->interpolation)
		{
			po->ip.gx = po->ip.ox * (1 - fframe) + po->x * fframe;
			po->ip.gy = po->ip.oy * (1 - fframe) + po->y * fframe;
		}
		else
		{
			po->ip.gx = po->x;
			po->ip.gy = po->y;
		}
		po->ip.gimage = po->ibase + po->image;

		/* Render the sprite! */
		if((po->ip.gimage >= 0) && (po->ip.gimage < pe->nsprites))
		{
			SDL_Rect dr;
			s = pe->sprites[po->ip.gimage];
			dr.x = po->ip.gx - s->hotx + pe->view.x;
			dr.y = po->ip.gy - s->hoty + pe->view.y;
			SDL_BlitSurface(pe->sprites[po->ip.gimage]->surface,
				NULL, pe->surface, &dr);
			/*
			 * We use the clipped rect for the dirtyrect!
			 */
			if(dr.w && dr.h)
				pig_dirty_add(pdt, &dr);
		}
		po = po->next;
	}

	/* Merge the display/back page table into the work table */
	pig_dirty_merge(pe->workdirty, pdt);
}


void pig_refresh(PIG_engine *pe)
{
	remove_sprites(pe);
	draw_sprites(pe);
}


void pig_refresh_all(PIG_engine *pe)
{
	tile_area(pe, &pe->view);
	pig_dirty(pe, NULL);
	draw_sprites(pe);
}


static void show_rects(PIG_engine *pe, PIG_dirtytable *pdt)
{
	int i;
	Uint32 color;
	if(!pe->buffer)
	{
		pe->buffer = SDL_CreateRGBSurface(SDL_SWSURFACE,
				pe->screen->w, pe->screen->h,
				pe->screen->format->BitsPerPixel,
				pe->screen->format->Rmask,
				pe->screen->format->Gmask,
				pe->screen->format->Bmask,
				pe->screen->format->Amask);
		if(!pe->buffer)
			return;
		pe->surface = pe->buffer;
		tile_area(pe, &pe->view);
	}
	if(!pe->buffer)
		return;

	pe->direct = 0;

	for(i = 0; i < pdt->count; ++i)
	{
		SDL_Rect r;
		r = pdt->rects[i];
		r.x -= 32;
		r.y -= 32;
		r.w += 64;
		r.h += 64;
		SDL_BlitSurface(pe->buffer, &r, pe->screen, &r);
	}

	color = SDL_MapRGB(pe->screen->format, 255, 0, 255);
	for(i = 0; i < pdt->count; ++i)
	{
		SDL_Rect r;
		r = pdt->rects[i];
		r.h = 1;
		SDL_FillRect(pe->screen, &r, color);
		r.y += pdt->rects[i].h - 1;
		SDL_FillRect(pe->screen, &r, color);
		r = pdt->rects[i];
		r.w = 1;
		SDL_FillRect(pe->screen, &r, color);
		r.x += pdt->rects[i].w - 1;
		SDL_FillRect(pe->screen, &r, color);
	}
}


void pig_flip(PIG_engine *pe)
{
	PIG_dirtytable *pdt = pe->workdirty;
	int i;
	SDL_SetClipRect(pe->surface, NULL);

	if(pe->show_dirtyrects)
	{
		show_rects(pe, pdt);
		for(i = 0; i < pdt->count; ++i)
		{
			pdt->rects[i].x -= 32;
			pdt->rects[i].y -= 32;
			pdt->rects[i].w += 64;
			pdt->rects[i].h += 64;
			pig_intersectrect(&pe->buffer->clip_rect, &pdt->rects[i]);
		}
	}
	else if(pe->surface == pe->buffer)
		for(i = 0; i < pdt->count; ++i)
			SDL_BlitSurface(pe->buffer, pdt->rects + i,
					pe->screen, pdt->rects + i);

	if((pe->screen->flags & SDL_HWSURFACE) == SDL_HWSURFACE)
	{
		SDL_Flip(pe->screen);
		if(pe->pages > 1)
			pe->page = 1 - pe->page;
	}
	else
		SDL_UpdateRects(pe->screen, pdt->count, pdt->rects);

	if(pe->direct)
		pe->surface = pe->screen;
	else
		pe->surface = pe->buffer ? pe->buffer : pe->screen;
}


void pig_draw_sprite(PIG_engine *pe, int frame, int x, int y)
{
	SDL_Rect dr;
	if(frame >= pe->nsprites)
		return;
	dr.x = x - pe->sprites[frame]->hotx + pe->view.x;
	dr.y = y - pe->sprites[frame]->hoty + pe->view.y;
	SDL_BlitSurface(pe->sprites[frame]->surface, NULL,
			pe->surface, &dr);
}


/*----------------------------------------------------------
	Map
----------------------------------------------------------*/
PIG_map *pig_map_open(PIG_engine *pe, int w, int h)
{
	if(pe->map)
		pig_map_close(pe->map);

	pe->map = (PIG_map *)calloc(1, sizeof(PIG_map));
	if(!pe->map)
		return NULL;

	pe->map->owner = pe;
	pe->map->w = w;
	pe->map->h = h;
	pe->map->hit = (unsigned char *)calloc(w, h);
	if(!pe->map->hit)
	{
		pig_map_close(pe->map);
		return NULL;
	}
	pe->map->map = (unsigned char *)calloc(w, h);
	if(!pe->map->map)
	{
		pig_map_close(pe->map);
		return NULL;
	}
	return pe->map;
}


void pig_map_close(PIG_map *pm)
{
	PIG_engine *pe = pm->owner;
	if(pm->tiles)
		SDL_FreeSurface(pm->tiles);
	free(pm->hit);
	free(pm->map);
	free(pe->map);
	pe->map = NULL;
}


int pig_map_tiles(PIG_map *pm, const char *filename, int tw, int th)
{
	SDL_Surface *tmp;
	pm->tw = tw;
	pm->th = th;
	tmp = IMG_Load(filename);
	if(!tmp)
	{
		fprintf(stderr, "Could not load '%s'!\n", filename);
		return -1;
	}
	pm->tiles = SDL_DisplayFormat(tmp);
	if(!pm->tiles)
	{
		fprintf(stderr, "Could not convert '%s'!\n", filename);
		return -1;
	}
	SDL_FreeSurface(tmp);
	return 0;
}


void pig_map_collisions(PIG_map *pm, unsigned first, unsigned count, PIG_sides sides)
{
	int i;
	if(first > 255)
		return;
	if(first + count > 255)
		count = 255 - first;
	for(i = first; i < first + count; ++i)
		pm->hitinfo[i] = sides;
}


/*
 * Load a map from a string (one byte/tile). 'trans'
 * is a string used for translating 'data' into integer
 * tile indices. Each position in 'trans' corresponds
 * to one tile in the tile palette.
 */
int pig_map_from_string(PIG_map *pm, const char *trans, const char *data)
{
	int x, y, z;

	/* Load the map */
	z = 0;
	for(y = 0; y < pm->h; ++y)
		for(x = 0; x < pm->w; ++x)
		{
			const char *f;
			int c = data[z];
			if(!c)
			{
				fprintf(stderr, "Map string too short!\n");
				return -1;
			}
			f = strchr(trans, c);
			if(!f)
			{
				fprintf(stderr, "Character '%c' not in"
						" the translation string!\n",
						c);
				return -1;
			}
			pm->map[z] = f - trans;
			++z;
		}
	/* Generate collision map */
	for(y = 0; y < pm->h; ++y)
		for(x = 0; x < pm->w; ++x)
			pm->hit[y * pm->w + x] =
					pm->hitinfo[pm->map[y * pm->w + x]];
	return 0;
}


/*----------------------------------------------------------
	Object
----------------------------------------------------------*/


static PIG_object *get_object(PIG_engine *pe)
{
	PIG_object *po;
	if(pe->object_pool)
	{
		po = pe->object_pool;
		pe->object_pool = po->next;
		memset(po, 0, sizeof(PIG_object));
	}
	else
	{
		po = (PIG_object *)calloc(1, sizeof(PIG_object));
		if(!po)
			return NULL;
	}
	po->id = ++pe->object_id_counter;
	return po;
}


static void free_object(PIG_object *po)
{
	po->prev = NULL;
	po->next = po->owner->object_pool;
	po->owner->object_pool = po;
	po->id = 0;
}


PIG_object *pig_object_open(PIG_engine *pe, int x, int y, int last)
{
	PIG_object *po = get_object(pe);
	if(!po)
		return NULL;

	po->owner = pe;
	po->tilemask = PIG_ALL;
	po->hitmask = 0;
	po->hitgroup = 0;

	if(last && pe->objects)
	{
		PIG_object *lo = pe->objects;
		while(lo->next)
			lo = lo->next;
		po->prev = lo;
		po->next = NULL;
		lo->next = po;
	}
	else
	{
		po->prev = NULL;
		po->next = pe->objects;
		if(po->next)
			po->next->prev = po;
		pe->objects = po;
	}

	po->x = x;
	po->y = y;
	po->ip.ox = x;
	po->ip.oy = y;
	return po;
}


static void close_object(PIG_object *po)
{
	if(po == po->owner->objects)
		po->owner->objects = po->next;
	else if(po->prev)
		po->prev->next = po->next;
	if(po->next)
		po->next->prev = po->prev;
	free_object(po);
}


void pig_object_close(PIG_object *po)
{
	if(!po->id)
		fprintf(stderr, "Object %p closed more than once!\n", po);
	po->id = 0;	/* Mark for eventual removal and destruction */
}


void pig_object_close_all(PIG_engine *pe)
{
	while(pe->objects)
		close_object(pe->objects);
}


PIG_object *pig_object_find(PIG_object *start, int id)
{
	PIG_object *pob, *pof;
	if(start)
		pob = pof = start;
	else
	{
		pof = start->owner->objects;
		while(pof)
		{
			if(pof->id == id)
				return pof;
			pof = pof->next;
		}
		return NULL;
	}
	while(1)
	{
		if(pob)
		{
			if(pob->id == id)
				return pob;
			pob = pob->prev;
		}
		if(pof)
		{
			if(pof->id == id)
				return pof;
			pof = pof->next;
		}
		else
			if(!pob)
				return NULL;
	}
}
