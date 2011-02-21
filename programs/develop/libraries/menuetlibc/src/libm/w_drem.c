/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
/*
 * drem() wrapper for remainder().
 * 
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include <math.h>

double
drem(x, y)
	double x, y;
{
	return remainder(x, y);
}
