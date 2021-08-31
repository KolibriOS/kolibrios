/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
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

/*
 * This file is largely based on (and named after) a set of common reading/
 * writing functions used in Quake engines.  Its purpose is to allow extraction
 * of bytes, words, and dwords in a safe, endian adjused environment and should
 * probably be used in any situation where checking for buffer overflows
 * manually makes the code a godawful mess.
 *
 * Currently this is only used by the animation decoding.
 *
 * This file is written with the intention of being easily converted into a
 * class capable of throwing exceptions if data is out of range.
 *
 * If an operation fails, subsequent operations will also fail.  The sizebuf
 * is assumed to be in an invalid state.  This COULD be changed pretty easily
 * and in normal Quake IIRC it is.  But our MO is to bail on failure, not
 * figure out what went wrong (making throws perfect).
 */
#include "sizebuf.h"

#include "SDL_endian.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Construct buffer with the passed array and size */
void SZ_Init(sizebuf_t * sz, Uint8 * buf, unsigned int size)
{
	sz->data = buf;
	sz->bufferLen = size;
	sz->bufferPos = 0;
	sz->error = false;
}
/* Check error flags */
bool SZ_Error(sizebuf_t * sz)
{
	return(sz->error);
}
/* mimic memset */
void SZ_Memset(sizebuf_t * sz, int value, size_t count)
{
	/* Do bounds checking before writing */
	if (sz->error || sz->bufferPos + count > sz->bufferLen)
	{
		sz->error = true;
		return;
	}

	/* Memset and increment pointer */
	memset(sz->data + sz->bufferPos, value, count);
	sz->bufferPos += count;
}
/* Mimic memcpy.  Two versions, one for buffers, one for sizebuf objects.
 * Overload in C++. */
void SZ_Memcpy(sizebuf_t * sz, const Uint8 * buf, size_t count)
{
	/* State checking */
	if (sz->error || sz->bufferPos + count > sz->bufferLen)
	{
		sz->error = true;
		return;
	}

	/* Memcpy & increment */
	memcpy(sz->data + sz->bufferPos, buf, count);
	sz->bufferPos += count;
}
void SZ_Memcpy2(sizebuf_t * sz, sizebuf_t * bf, size_t count)
{
	/* State checking */
	if (sz->error || sz->bufferPos + count > sz->bufferLen)
	{
		sz->error = true;
		return;
	}
	if (bf->error || bf->bufferPos + count > bf->bufferLen)
	{
		bf->error = true;
		return;
	}

	/* Memcpy & increment */
	memcpy(sz->data + sz->bufferPos, bf->data + bf->bufferPos, count);
	sz->bufferPos += count;
	bf->bufferPos += count;
}
/* Reposition buffer pointer */
void SZ_Seek(sizebuf_t * sz, long count, int mode)
{
	/* Okay, it's reasonable to reset the error bool on seeking... */

	switch(mode)
	{
		case SEEK_SET:
			sz->bufferPos = count;
			break;
		case SEEK_CUR:
			sz->bufferPos += count;
			break;
		case SEEK_END:
			sz->bufferPos = sz->bufferLen - count;
			break;
		default:
			assert(false);
	}

	/* Check errors */
	if (sz->bufferPos > sz->bufferLen)
	{
		sz->error = true;
	} else {
		sz->error = false;
	}
}
const Uint8 * SZ_GetCurBufferPtr (sizebuf_t * sz)
{
	return(sz->data);
}

/* The code below makes use of pointer casts, similar to what is in efread.
 * It's not the ONLY way to write ints to a stream, but it's probably the
 * cleanest of the lot.  Better to have it here than littered all over the code.
 */
void MSG_WriteByte(sizebuf_t * sz, unsigned int value)
{
	if (sz->error || sz->bufferPos + 1 > sz->bufferLen)
	{
		sz->error = true;
		return;
	}

	sz->data[sz->bufferPos] = value;
	sz->bufferPos++;
}
void MSG_WriteWord(sizebuf_t * sz, unsigned int value)
{
	if (sz->error || sz->bufferPos + 2 > sz->bufferLen)
	{
		sz->error = true;
		return;
	}

	*((Uint16 *)(sz->data + sz->bufferPos)) = SDL_SwapLE16( ((Uint16)value) );
	sz->bufferPos += 2;
}
void MSG_WriteDWord(sizebuf_t * sz, unsigned int value)
{
	if (sz->error || sz->bufferPos + 4 > sz->bufferLen)
	{
		sz->error = true;
		return;
	}

	*((Uint32 *)(sz->data + sz->bufferPos)) = SDL_SwapLE32( ((Uint32)value) );
	sz->bufferPos += 4;
}

unsigned int MSG_ReadByte(sizebuf_t * sz)
{
	unsigned int ret;


	if (sz->error || sz->bufferPos + 1 > sz->bufferLen)
	{
		sz->error = true;
		return(0);
	}

	ret = sz->data[sz->bufferPos];
	sz->bufferPos += 1;

	return(ret);
}
unsigned int MSG_ReadWord(sizebuf_t * sz)
{
	unsigned int ret;


	if (sz->error || sz->bufferPos + 2 > sz->bufferLen)
	{
		sz->error = true;
		return(0);
	}

	ret = SDL_SwapLE16(*((Uint16 *)(sz->data + sz->bufferPos)));
	sz->bufferPos += 2;

	return(ret);
}
unsigned int MSG_ReadDWord(sizebuf_t * sz)
{
	unsigned int ret;


	if (sz->error || sz->bufferPos + 4 > sz->bufferLen)
	{
		sz->error = true;
		return(0);
	}

	ret = SDL_SwapLE32(*((Uint32 *)(sz->data + sz->bufferPos)));
	sz->bufferPos += 4;

	return(ret);
}
