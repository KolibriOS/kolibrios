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
#include "arg_parse.h"

#include "std_support.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void permute( const char *argv[], int *first_nonopt, int *first_opt, int after_opt );

static int parse_short_opt( int argc, const char *const argv[], const Options *options, Option *option );
static int parse_long_opt( int argc, const char *const argv[], const Options *options, Option *option );

Option parse_args( int argc, const char *argv[], const Options *options )
{
	static int argn = 1;
	static bool no_more_options = false;
	
	static int first_nonopt = 1;
	
	Option option = { NOT_OPTION, NULL, 0 };
	option.argn = first_nonopt;
	
	while (argn < argc)
	{
		size_t arg_len = strlen(argv[argn]);
		
		if (!no_more_options &&
		    argv[argn][0] == '-' &&  // first char is '-'
		    arg_len > 1)             // option is not "-"
		{
			option.argn = argn;
			
			if (argv[argn][1] == '-')  // string begins with "--"
			{
				if (arg_len == 2)  // "--" alone indicates end of options
				{
					++argn;
					no_more_options = true;
				}
				else
				{
					argn = parse_long_opt(argc, argv, options, &option);
				}
			}
			else
			{
				argn = parse_short_opt(argc, argv, options, &option);
			}
			
			// shift option in front of non-options
			permute(argv, &first_nonopt, &option.argn, argn);
			
			// don't include "--" in non-options
			if (no_more_options)
				++option.argn;
			break;
		}
		else
		{
			// skip non-options, permute later when option encountered
			++argn;
		}
	}
	
	return option;
}

static void permute( const char *argv[], int *first_nonopt, int *first_opt, int after_opt )
{
	const int nonopts = *first_opt - *first_nonopt;
	
	// slide each of the options in front of the non-options
	for (int i = *first_opt; i < after_opt; ++i)
	{
		for (int j = i; j > *first_nonopt; --j)
		{
			// swap argv[j] and argv[j - 1]
			const char *temp = argv[j];
			argv[j] = argv[j - 1];
			argv[j - 1] = temp;
		}
		
		// position of first non-option shifts right once for each option
		++(*first_nonopt);
	}
	
	// position of first option is initial position of first non-option
	*first_opt -= nonopts;
}

static int parse_short_opt( int argc, const char *const argv[], const Options *options, Option *option )
{
	static size_t offset = 1;  // ignore the "-"
	
	int argn = option->argn;
	
	const char *arg = argv[argn];
	
	const size_t arg_len = strlen(arg);
	
	const bool arg_attached = (offset + 1 < arg_len),  // possible argument attached?
	           last_in_argv = (argn == argc - 1);
	
	option->value = INVALID_OPTION;
	
	for (; !(options->short_opt == 0 &&
	         options->long_opt == NULL); ++options)
	{
		if (options->short_opt != 0 &&
		    options->short_opt == arg[offset])
		{
			option->value = options->value;
			
			if (options->has_arg)
			{
				if (arg_attached)  // arg direclty follows option
				{
					option->arg = arg + offset + 1;
					
					offset = arg_len;
				}
				else if (!last_in_argv)  // arg is next in argv
				{
					option->arg = argv[++argn];
					
					offset = arg_len;
				}
				else
				{
					option->value = OPTION_MISSING_ARG;
					break;
				}
			}
			
			break;
		}
	}
	
	switch (option->value)
	{
	case INVALID_OPTION:
		fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], argv[option->argn][offset]);
		break;
	case OPTION_MISSING_ARG:
		fprintf(stderr, "%s: option requires an argument -- '%c'\n", argv[0], argv[option->argn][offset]);
		break;
	}
	
	if (++offset >= arg_len)
	{
		++argn;
		offset = 1;
	}
	
	return argn;  // which arg in argv that parse_args() should examine when called again
}

static int parse_long_opt( int argc, const char *const argv[], const Options *options, Option *option )
{
	int argn = option->argn;
	
	const char *arg = argv[argn] + 2;  // ignore the "--"
	
	const size_t arg_len = strlen(arg),
	             arg_opt_len = ot_strchrnul(arg, '=') - arg;  // length before "="
	
	const bool arg_attached = (arg_opt_len < arg_len),  // argument attached using "="?
	           last_in_argv = (argn == argc - 1);
	
	option->value = INVALID_OPTION;
	
	for (; !(options->short_opt == 0 &&
	         options->long_opt == NULL); ++options)
	{
		if (options->long_opt != NULL &&
		    strncmp(options->long_opt, arg, arg_opt_len) == 0)  // matches (partially, at least)
		{
			if (option->value != INVALID_OPTION)  // other match already found
			{
				option->value = AMBIGUOUS_OPTION;
				break;
			}
			
			option->value = options->value;
			
			if (options->has_arg)
			{
				if (arg_attached)  // arg is after "="
				{
					option->arg = arg + arg_opt_len + 1;
				}
				else if (!last_in_argv)  // arg is next in argv
				{
					option->arg = argv[++argn];
				}
				else  // arg is missing
				{
					option->value = OPTION_MISSING_ARG;
					// can't break, gotta check for ambiguity
				}
			}
			
			if (arg_opt_len == strlen(options->long_opt)) // exact match
				break;
			// can't break for partial match, gotta check for ambiguity
		}
	}
	
	switch (option->value)
	{
	case INVALID_OPTION:
		fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], argv[option->argn]);
		break;
	case AMBIGUOUS_OPTION:
		fprintf(stderr, "%s: option '%s' is ambiguous\n", argv[0], argv[option->argn]);
		break;
	case OPTION_MISSING_ARG:
		fprintf(stderr, "%s: option '%s' requires an argument\n", argv[0], argv[option->argn]);
		break;
	}
	
	++argn;
	
	return argn;  // which arg in argv that parse_args() should examine when called again
}
