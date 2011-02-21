/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
/*
 * cabsf() wrapper for hypotf().
 * 
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include "math.h"
#include "math_private.h"

struct complex {
	float x;
	float y;
};

float
cabsf(z)
	struct complex z;
{
	return hypotf(z.x, z.y);
}
