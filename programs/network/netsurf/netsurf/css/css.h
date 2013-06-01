/*
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
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

#ifndef netsurf_css_css_h_
#define netsurf_css_css_h_

#include <stdint.h>

#include <libcss/libcss.h>

#include "utils/errors.h"

struct content;
struct content_css_data;
struct hlcache_handle;
struct http_parameter;
struct nscss_import;

/**
 * Type of callback called when a CSS object has finished
 *
 * \param css  CSS object that has completed
 * \param pw   Client-specific data
 */
typedef void (*nscss_done_callback)(struct content_css_data *css, void *pw);

/**
 * CSS content data
 */
struct content_css_data
{
	css_stylesheet *sheet;		/**< Stylesheet object */
	char *charset;			/**< Character set of stylesheet */
	struct nscss_import *imports;	/**< Array of imported sheets */
	uint32_t import_count;		/**< Number of sheets imported */
	uint32_t next_to_register;	/**< Index of next import to register */
	nscss_done_callback done;	/**< Completion callback */
	void *pw;			/**< Client data */
};

/**
 * Imported stylesheet record
 */
struct nscss_import {
	struct hlcache_handle *c;	/**< Content containing sheet */
	uint64_t media;		/**< Media types that sheet applies to */
};

nserror nscss_init(void);

nserror nscss_create_css_data(struct content_css_data *c,
		const char *url, const char *charset, bool quirks,
		nscss_done_callback done, void *pw);
css_error nscss_process_css_data(struct content_css_data *c, const char *data, 
		unsigned int size);
css_error nscss_convert_css_data(struct content_css_data *c);
void nscss_destroy_css_data(struct content_css_data *c);

css_stylesheet *nscss_get_stylesheet(struct hlcache_handle *h);
struct nscss_import *nscss_get_imports(struct hlcache_handle *h, uint32_t *n);

#endif

