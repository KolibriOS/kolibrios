/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@devolution.com
*/

/* Functions for dynamically reading and writing endian-specific values */

#include "SDL_endian.h"

Uint16 SDL_ReadLE16 (SDL_RWops *src)
{
	Uint16 value;

	SDL_RWread(src, &value, (sizeof value), 1);
	return(SDL_SwapLE16(value));
}

Uint16 SDL_ReadBE16 (SDL_RWops *src)
{
	Uint16 value;

	SDL_RWread(src, &value, (sizeof value), 1);
	return(SDL_SwapBE16(value));
}
Uint32 SDL_ReadLE32 (SDL_RWops *src)
{
	Uint32 value;

	SDL_RWread(src, &value, (sizeof value), 1);
	return(SDL_SwapLE32(value));
}
Uint32 SDL_ReadBE32 (SDL_RWops *src)
{
	Uint32 value;

	SDL_RWread(src, &value, (sizeof value), 1);
	return(SDL_SwapBE32(value));
}
Uint64 SDL_ReadLE64 (SDL_RWops *src)
{
	Uint64 value;

	SDL_RWread(src, &value, (sizeof value), 1);
	return(SDL_SwapLE64(value));
}
Uint64 SDL_ReadBE64 (SDL_RWops *src)
{
	Uint64 value;

	SDL_RWread(src, &value, (sizeof value), 1);
	return(SDL_SwapBE64(value));
}

int SDL_WriteLE16 (SDL_RWops *dst, Uint16 value)
{
	value = SDL_SwapLE16(value);
	return(SDL_RWwrite(dst, &value, (sizeof value), 1));
}

int SDL_WriteBE16 (SDL_RWops *dst, Uint16 value)
{
	value = SDL_SwapBE16(value);
	return(SDL_RWwrite(dst, &value, (sizeof value), 1));
}

int SDL_WriteLE32 (SDL_RWops *dst, Uint32 value)
{
	value = SDL_SwapLE32(value);
	return(SDL_RWwrite(dst, &value, (sizeof value), 1));
}

int SDL_WriteBE32 (SDL_RWops *dst, Uint32 value)
{
	value = SDL_SwapBE32(value);
	return(SDL_RWwrite(dst, &value, (sizeof value), 1));
}

int SDL_WriteLE64 (SDL_RWops *dst, Uint64 value)
{
	value = SDL_SwapLE64(value);
	return(SDL_RWwrite(dst, &value, (sizeof value), 1));
}

int SDL_WriteBE64 (SDL_RWops *dst, Uint64 value)
{
	value = SDL_SwapBE64(value);
	return(SDL_RWwrite(dst, &value, (sizeof value), 1));
}
