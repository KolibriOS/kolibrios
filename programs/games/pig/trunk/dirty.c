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
#include "engine.h"

/* Approximate worth of one dirtyrect in pixels. */
#define	PIG_WORST_MERGE		300

/*
 * If the merged result gets at most this many percent
 * bigger than the larger of the two input rects,
 * accept it as Perfect.
 */
#define	PIG_INSTANT_MERGE	10


PIG_dirtytable *pig_dirty_open(int size)
{
	PIG_dirtytable *pdt = (PIG_dirtytable *)malloc(sizeof(PIG_dirtytable));
	if(!pdt)
		return NULL;

	pdt->size = size;
	pdt->rects = (SDL_Rect *)calloc(size, sizeof(SDL_Rect));
	if(!pdt->rects)
	{
		free(pdt);
		return NULL;
	}

	pdt->count = 0;
	pdt->best = 0;
	return pdt;
}


void pig_dirty_close(PIG_dirtytable *pdt)
{
	free(pdt->rects);
	free(pdt);
}


void pig_mergerect(SDL_Rect *from, SDL_Rect *to)
{
	int x1 = from->x;
	int y1 = from->y;
	int x2 = from->x + from->w;
	int y2 = from->y + from->h;
	if(to->x < x1)
		x1 = to->x;
	if(to->y < y1)
		y1 = to->y;
	if(to->x + to->w > x2)
		x2 = to->x + to->w;
	if(to->y + to->h > y2)
		y2 = to->y + to->h;
	to->x = x1;
	to->y = y1;
	to->w = x2 - x1;
	to->h = y2 - y1;
}


void pig_intersectrect(SDL_Rect *from, SDL_Rect *to)
{
	int Amin, Amax, Bmin, Bmax;
	Amin = to->x;
	Amax = Amin + to->w;
	Bmin = from->x;
	Bmax = Bmin + from->w;
	if(Bmin > Amin)
		Amin = Bmin;
	to->x = Amin;
	if(Bmax < Amax)
		Amax = Bmax;
	to->w = Amax - Amin > 0 ? Amax - Amin : 0;

	Amin = to->y;
	Amax = Amin + to->h;
	Bmin = from->y;
	Bmax = Bmin + from->h;
	if(Bmin > Amin)
		Amin = Bmin;
	to->y = Amin;
	if(Bmax < Amax)
		Amax = Bmax;
	to->h = Amax - Amin > 0 ? Amax - Amin : 0;
}


void pig_dirty_add(PIG_dirtytable *pdt, SDL_Rect *dr)
{
	int i, j, best_i, best_loss;
	/*
	 * Look for merger candidates.
	 *
	 * We start right before the best match we
	 * had the last time around. This can give
	 * us large numbers of direct or quick hits
	 * when dealing with old/new rects for moving
	 * objects and the like.
	 */
	best_i = -1;
	best_loss = 100000000;
	if(pdt->count)
		i = (pdt->best + pdt->count - 1) % pdt->count;
	for(j = 0; j < pdt->count; ++j)
	{
		int a1, a2, am, ratio, loss;
		SDL_Rect testr;

		a1 = dr->w * dr->h;

		testr = pdt->rects[i];
		a2 = testr.w * testr.h;

		pig_mergerect(dr, &testr);
		am = testr.w * testr.h;

		/* Perfect or Instant Pick? */
		ratio = 100 * am / (a1 > a2 ? a1 : a2);
		if(ratio < PIG_INSTANT_MERGE)
		{
			/* Ok, this is good enough! Stop searching. */
			pig_mergerect(dr, &pdt->rects[i]);
			pdt->best = i;
			return;
		}

		loss = am - a1 - a2;
		if(loss < best_loss)
		{
			best_i = i;
			best_loss = loss;
			pdt->best = i;
		}

		++i;
		i %= pdt->count;
	}
	/* ...and if the best result is good enough, merge! */
	if((best_i >= 0) && (best_loss < PIG_WORST_MERGE))
	{
		pig_mergerect(dr, &pdt->rects[best_i]);
		return;
	}

	/* Try to add to table... */
	if(pdt->count < pdt->size)
	{
		pdt->rects[pdt->count++] = *dr;
		return;
	}

	/* Emergency: Table full! Grab best candidate... */
	pig_mergerect(dr, &pdt->rects[best_i]);
}


void pig_dirty_merge(PIG_dirtytable *pdt, PIG_dirtytable *from)
{
	int i;
	for(i = 0; i < from->count; ++i)
		pig_dirty_add(pdt, from->rects + i);
}
