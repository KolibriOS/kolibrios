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

#ifndef SIZEBUF_H
#define SIZEBUF_H

#include "opentyr.h"

typedef struct sizebuf_s
{
	Uint8 *data;
	unsigned int bufferLen;
	unsigned int bufferPos;
	bool error;
} sizebuf_t;

void SZ_Init    ( sizebuf_t *, Uint8 *, unsigned int ); /* C style constructor */
bool SZ_Error   ( sizebuf_t * );
void SZ_Memset  ( sizebuf_t *, int, size_t ); /* memset with a sizebuf */
void SZ_Memcpy  ( sizebuf_t *, const Uint8 *, size_t ); /* memcpy with a normal buffer */
void SZ_Memcpy2 ( sizebuf_t *, sizebuf_t *, size_t );   /* memcpy with a sizebuf */
void SZ_Seek    ( sizebuf_t *, long, int ); /* fseek with a sizebuf. */

const Uint8 * SZ_GetCurBufferPtr ( sizebuf_t * ); /* Mimic private member, const return */

void MSG_WriteByte  ( sizebuf_t *, unsigned int );
void MSG_WriteWord  ( sizebuf_t *, unsigned int );
void MSG_WriteDWord ( sizebuf_t *, unsigned int );

unsigned int MSG_ReadByte  ( sizebuf_t * );
unsigned int MSG_ReadWord  ( sizebuf_t * );
unsigned int MSG_ReadDWord ( sizebuf_t * );

#endif
