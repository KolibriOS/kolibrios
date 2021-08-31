/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2010  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef VIDEO_SCALE_H
#define VIDEO_SCALE_H

#include "opentyr.h"

#include "SDL.h"

typedef void (*ScalerFunction)( SDL_Surface *dst, SDL_Surface *src );

struct Scalers
{
	int width, height;
	ScalerFunction scaler8, scaler16, scaler32;
	const char *name;
};

extern uint scaler;
extern const struct Scalers scalers[];
extern const uint scalers_count;

void set_scaler_by_name( const char *name );

#endif /* VIDEO_SCALE_H */

