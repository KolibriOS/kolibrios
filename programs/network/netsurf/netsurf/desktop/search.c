/*
 * Copyright 2004 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2005 Adrian Lees <adrianl@users.sourceforge.net>
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
 
 /** \file
 * Free text search (core)
 */
#include "utils/config.h"

#include <ctype.h>
#include <string.h>
#include <dom/dom.h>
#include "content/content.h"
#include "content/hlcache.h"
#include "desktop/browser_private.h"
#include "desktop/gui.h"
#include "desktop/options.h"
#include "desktop/search.h"
#include "desktop/selection.h"
#include "render/box.h"
#include "render/html.h"
#include "render/search.h"
#include "render/textplain.h"
#include "utils/config.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/url.h"
#include "utils/utils.h"


/* callback informing us that a search context is nolonger valid */
static void browser_window_search_invalidate(struct search_context *context,
		void *p)
{
	struct browser_window *bw = p;
	assert(bw != NULL);

	if (bw->cur_search != NULL && bw->cur_search == context) {
		/* The browser's current search is the one being invalidated */
		bw->cur_search = NULL;
	}
}


bool browser_window_search_create_context(struct browser_window *bw, 
		struct gui_search_callbacks *gui_callbacks, void *gui_p)
{
	struct search_callbacks callbacks;
	assert(bw != NULL);
	assert(gui_callbacks != NULL);

	if (bw->cur_search != NULL)
		search_destroy_context(bw->cur_search);
	bw->cur_search = NULL;

	if (!bw->current_content)
		return false;

	callbacks.gui = gui_callbacks;
	callbacks.gui_p = gui_p;
	callbacks.invalidate = browser_window_search_invalidate;
	callbacks.p = bw;
	bw->cur_search = search_create_context(bw->current_content, callbacks);

	if (bw->cur_search == NULL)
		return false;

	return true;
}


void browser_window_search_destroy_context(struct browser_window *bw)
{
	assert(bw != NULL);

	if (bw->cur_search != NULL)
		search_destroy_context(bw->cur_search);
	bw->cur_search = NULL;
}


/**
 * to simplify calls to search_step(); checks that the browser_window is
 * non-NULL, creates a new search_context in case of a new search
 * \param bw the browser_window the search refers to
 * \param callbacks the callbacks to modify appearance according to results
 * \param gui_p a pointer returned to the callbacks
 * \return true for success
 */
bool browser_window_search_verify_new(struct browser_window *bw,
		struct gui_search_callbacks *gui_callbacks, void *gui_p)
{
	if (bw == NULL)
		return false;
	if (bw->cur_search == NULL)
		return browser_window_search_create_context(bw,
				gui_callbacks, gui_p);

	return true;
}


void browser_window_search_step(struct browser_window *bw,
		search_flags_t flags, const char *string)
{
	assert(bw != NULL);

	if (bw->cur_search != NULL)
		search_step(bw->cur_search, flags, string);
}


void browser_window_search_show_all(bool all, struct browser_window *bw)
{
	assert(bw != NULL);

	if (bw->cur_search != NULL)
		search_show_all(all, bw->cur_search);
}

