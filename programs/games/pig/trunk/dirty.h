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

#ifndef	PIG_DIRTY_H
#define	PIG_DIRTY_H

/* A table of dirtyrects for one display page */
typedef struct PIG_dirtytable
{
	int		size;	/* Table size */
	SDL_Rect	*rects;	/* Table of rects */
	int		count;	/* # of rects currently used */
	int		best;	/* Merge testing starts here! */
} PIG_dirtytable;


PIG_dirtytable *pig_dirty_open(int size);
void pig_dirty_close(PIG_dirtytable *pdt);

/* Add rectangle 'dr' to table 'pdt' */
void pig_dirty_add(PIG_dirtytable *pdt, SDL_Rect *dr);

/* Merge table 'from' into 'pdt' */
void pig_dirty_merge(PIG_dirtytable *pdt, PIG_dirtytable *from);

/* Extend 'to' to a new rect that includes both 'from' and 'to' */
void pig_mergerect(SDL_Rect *from, SDL_Rect *to);

/* Clip 'to' into a rect that is the intersection of 'from' and 'to' */
void pig_intersectrect(SDL_Rect *from, SDL_Rect *to);

#endif /* PIG_DIRTY_H */
