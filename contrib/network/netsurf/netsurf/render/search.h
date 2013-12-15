/*
 * Copyright 2009 Mark Benjamin <netsurf-browser.org.MarkBenjamin@dfgh.net>
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

#ifndef _NETSURF_RENDER_SEARCH_H_
#define _NETSURF_RENDER_SEARCH_H_

#include <ctype.h>
#include <string.h>

#include "desktop/search.h"

struct search_context;

/**
 * Called when a search context is destroyed
 * \param context  search context being invalidated
 * \param p        pointer for client data
 */
typedef void (*search_invalidate_callback)(struct search_context *context,
		void *p);

struct search_callbacks {
	struct gui_search_callbacks *gui;
	void *gui_p; /* private gui owned data */
	search_invalidate_callback invalidate;
	void *p; /* private client data */
};


struct search_context * search_create_context(struct hlcache_handle *h, 
		struct search_callbacks callbacks);
void search_destroy_context(struct search_context *context);
void search_step(struct search_context *context, search_flags_t flags,
		const char * string);
void search_show_all(bool all, struct search_context *context);


bool search_term_highlighted(struct content *c,
		unsigned start_offset, unsigned end_offset,
		unsigned *start_idx, unsigned *end_idx,
		struct search_context *context);

#endif
