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
#ifndef ARG_PARSE_H
#define ARG_PARSE_H

#include <stdbool.h>

// this is essentially a reimplementation of getopt_long()

typedef struct
{
	int value;
	char short_opt;
	const char *long_opt;
	bool has_arg;
}
Options;

enum
{
	// indicates that argv[argn..argc) are not options
	NOT_OPTION = 0,
	
	/* behavior of parse_args() is undefined after
	   it has returned any of the following values */
	INVALID_OPTION = -1,
	AMBIGUOUS_OPTION = -2,
	OPTION_MISSING_ARG = -3
};

typedef struct
{
	int value;
	const char *arg;
	
	int argn;
}
Option;

Option parse_args( int argc, const char *argv[], const Options *options );

#endif /* ARG_PARSE_H */
