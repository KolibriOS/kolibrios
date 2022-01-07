/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/** @file SDL_stdinc.h
 *  This is a general header that includes C language support
 */

#ifndef _SDL_stdinc_h
#define _SDL_stdinc_h

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>

/** The number of elements in an array */
#define SDL_arraysize(array)	(sizeof(array)/sizeof(array[0]))

/* Use proper C++ casts when compiled as C++ to be compatible with the option
 -Wold-style-cast of GCC (and -Werror=old-style-cast in GCC 4.2 and above. */
#ifdef __cplusplus
#define SDL_reinterpret_cast(type, expression) reinterpret_cast<type>(expression)
#define SDL_static_cast(type, expression) static_cast<type>(expression)
#else
#define SDL_reinterpret_cast(type, expression) ((type)(expression))
#define SDL_static_cast(type, expression) ((type)(expression))
#endif

typedef enum {
	DUMMY_ENUM_VALUE
} SDL_DUMMY_ENUM;

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#define SDL_malloc  malloc
#define SDL_calloc	calloc
#define SDL_realloc	realloc
#define SDL_free	free
#define SDL_stack_alloc(type, count)	(type*)SDL_malloc(sizeof(type)*(count))
#define SDL_stack_free(data)	SDL_free(data)
#define SDL_qsort	qsort
#define SDL_abs		abs
#define SDL_min(x, y)	(((x) < (y)) ? (x) : (y))
#define SDL_max(x, y)	(((x) > (y)) ? (x) : (y))
#define SDL_isdigit(X)  isdigit(X)
#define SDL_isspace(X)  isspace(X)
#define SDL_toupper(X)  toupper(X)
#define SDL_tolower(X)  tolower(X)
#define SDL_memset      memset
#define SDL_memmove     memmove
#define SDL_memcmp      memcmp
#define SDL_strlen      strlen
#define SDL_strlcpy     strlcpy
#define SDL_strlcat    strlcat
#define SDL_strdup     strdup
#define SDL_strrev      _strrev
#define SDL_strupr      _strupr
#define SDL_strlwr      _strlwr
#define SDL_strchr      strchr
#define SDL_strrchr     strrchr
#define SDL_strstr      strstr
#define SDL_itoa        itoa
#define SDL_ltoa        _ltoa
#define SDL_uitoa       _uitoa
#define SDL_ultoa       _ultoa
#define SDL_strtol      strtol
#define SDL_strtoul      strtoul
#define SDL_strtod      strtod
#define SDL_atoi        atoi
#define SDL_atof        atof
#define SDL_strcmp      strcmp
#define SDL_strncmp     strncmp
#define SDL_sscanf      sscanf
#define SDL_snprintf    snprintf
#define SDL_vsnprintf   vsnprintf

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_stdinc_h */
