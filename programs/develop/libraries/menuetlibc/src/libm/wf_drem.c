/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
/*
 * dremf() wrapper for remainderf().
 * 
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include "math.h"
#include "math_private.h"

float
dremf(x, y)
	float x, y;
{
	return remainderf(x, y);
}
