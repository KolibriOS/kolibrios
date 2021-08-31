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
#ifndef FILE_H
#define FILE_H

#include "SDL_endian.h"

#include <stdbool.h>
#include <stdio.h>

extern const char *custom_data_dir;

const char *data_dir( void );

FILE *dir_fopen( const char *dir, const char *file, const char *mode );
FILE *dir_fopen_warn( const char *dir, const char *file, const char *mode );
FILE *dir_fopen_die( const char *dir, const char *file, const char *mode );

bool dir_file_exists( const char *dir, const char *file );

long ftell_eof( FILE *f );

// endian-swapping fread/fwrite that die if the expected amount cannot be read/written
size_t efread( void *buffer, size_t size, size_t num, FILE *stream );
size_t efwrite( const void *buffer, size_t size, size_t num, FILE *stream );

#endif // FILE_H

