/*
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 * Option reading and saving (interface).
 *
 * Non-platform specific options can be added by editing this file and
 * netsurf/desktop/options.c
 *
 * Platform specific options should be added in the platform options.h.
 *
 * The following types of options are supported:
 *  - bool (OPTION_BOOL)
 *  - int (OPTION_INTEGER)
 *  - char* (OPTION_STRING) (must be allocated on heap, may be 0, free before
 *                           assigning a new value)
 */

#ifndef _NETSURF_DESKTOP_OPTIONS_H_
#define _NETSURF_DESKTOP_OPTIONS_H_

#define _NETSURF_DESKTOP_OPTIONS_INCLUDING_

#include <stdbool.h>
#include <stdio.h>

#include "desktop/plot_style.h"
#include "desktop/options_main.h"

#if defined(riscos)
#include "riscos/options.h"
#elif defined(nsgtk)
#include "gtk/options.h"
#elif defined(nsbeos)
#include "beos/options.h"
#elif defined(nsamiga)
#include "amiga/options.h"
#elif defined(nsframebuffer)
#include "framebuffer/options.h"
#elif defined(nsatari)
#include "atari/options.h"
#elif defined(nsmonkey)
#include "monkey/options.h"
#else
#define NSOPTION_EXTRA_DEFINE
#define NSOPTION_EXTRA_DEFAULTS
#define NSOPTION_EXTRA_TABLE
#endif

/* allow the colour defaults to be overidden by the frontends */
#ifndef NSOPTION_SYS_COLOUR_DEFAULTS
#define NSOPTION_SYS_COLOUR_DEFAULTS NSOPTION_MAIN_SYS_COLOUR_DEFAULTS
#endif

#undef _NETSURF_DESKTOP_OPTIONS_INCLUDING_


enum { OPTION_HTTP_PROXY_AUTH_NONE = 0,
       OPTION_HTTP_PROXY_AUTH_BASIC = 1,
       OPTION_HTTP_PROXY_AUTH_NTLM = 2 };

#define DEFAULT_MARGIN_TOP_MM 10
#define DEFAULT_MARGIN_BOTTOM_MM 10
#define DEFAULT_MARGIN_LEFT_MM 10
#define DEFAULT_MARGIN_RIGHT_MM 10
#define DEFAULT_EXPORT_SCALE 0.7
#ifdef riscos
#define DEFAULT_REFLOW_PERIOD 100 /* time in cs */
#else
#define DEFAULT_REFLOW_PERIOD 25 /* time in cs */
#endif

struct ns_options {
	NSOPTION_MAIN_DEFINE;
	NSOPTION_EXTRA_DEFINE;
};

/* global option struct */
extern struct ns_options nsoptions;

/* acessors */
#define nsoption_bool(OPTION) (nsoptions.OPTION)
#define nsoption_int(OPTION) (nsoptions.OPTION)
#define nsoption_charp(OPTION) (nsoptions.OPTION)
#define nsoption_colour(OPTION) (nsoptions.OPTION)

#define nsoption_set_bool(OPTION, VALUE) nsoptions.OPTION = VALUE
#define nsoption_set_int(OPTION, VALUE) nsoptions.OPTION = VALUE
#define nsoption_set_colour(OPTION, VALUE) nsoptions.OPTION = VALUE
#define nsoption_set_charp(OPTION, VALUE) do {	\
	if (nsoptions.OPTION != NULL) {		\
		free(nsoptions.OPTION);		\
	}					\
	nsoptions.OPTION = VALUE;               \
        if ((nsoptions.OPTION != NULL) &&	\
	    (*nsoptions.OPTION == 0)) {		\
            free(nsoptions.OPTION);             \
            nsoptions.OPTION = NULL;            \
        }                                       \
    } while (0)

#define nsoption_setnull_charp(OPTION, VALUE)           \
    do {                                                \
        if (nsoptions.OPTION == NULL) {                 \
            nsoptions.OPTION = VALUE;                   \
            if (*nsoptions.OPTION == 0) {               \
                free(nsoptions.OPTION);                 \
                nsoptions.OPTION = NULL;                \
            }                                           \
        } else {                                        \
            free(VALUE);                                \
        }                                               \
    } while (0)


/**
 * Read options from a file.
 *
 * \param  path  name of file to read options from
 *
 * Option variables corresponding to lines in the file are updated. Missing
 * options are unchanged. If the file fails to open, options are unchanged.
 */
void nsoption_read(const char *path);

/**
 * Save options to a file.
 *
 * \param  path  name of file to write options to
 *
 * Errors are ignored.
 */
void nsoption_write(const char *path);

/**
 * Dump user options to stream
 *
 * \param outf output stream to dump options to.
 */
void nsoption_dump(FILE *outf);

/**
 * Fill a buffer with an option using a format.
 *
 * The format string is copied into the output buffer with the
 * following replaced:
 * %k - The options key
 * %t - The options type
 * %V - value - HTML type formatting
 * %v - value - plain formatting
 *
 * \param string  The buffer in which to place the results.
 * \param size    The size of the string buffer.
 * \param option  The opaque option number.
 * \param fmt     The format string.
 * \return The number of bytes written to \a string or -1 on error
 */
int nsoption_snoptionf(char *string, size_t size, unsigned int option,
		const char *fmt);

/**
 * Process commandline and set options approriately.
 */
void nsoption_commandline(int *pargc, char **argv);

/**
 * Set default values for unset front-end specific options
 */
void gui_options_init_defaults(void);

#endif

