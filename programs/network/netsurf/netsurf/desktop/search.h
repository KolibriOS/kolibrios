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

#ifndef _NETSURF_DESKTOP_SEARCH_H_
#define _NETSURF_DESKTOP_SEARCH_H_

#include <ctype.h>
#include <string.h>

typedef enum {
	SEARCH_FLAG_CASE_SENSITIVE = (1 << 0),
	SEARCH_FLAG_FORWARDS = (1 << 1),
	SEARCH_FLAG_SHOWALL = (1 << 2)
} search_flags_t;

/**
 * Change the displayed search status.
 * \param found  search pattern matched in text
 * \param p gui private data pointer provided with search callbacks
 */
typedef void (*gui_search_status)(bool found, void *p);

/**
 * display hourglass while searching
 * \param active start/stop indicator
 * \param p gui private data pointer provided with search callbacks
 */
typedef void (*gui_search_hourglass)(bool active, void *p);

/**
 * add search string to recent searches list
 * front has full liberty how to implement the bare notification;
 * core gives no guarantee of the integrity of the const char *
 * \param string search pattern
 * \param p gui private data pointer provided with search callbacks
 */
typedef void (*gui_search_add_recent)(const char *string, void *p);

/**
 * activate search forwards button in gui
 * \param active activate/inactivate
 * \param p gui private data pointer provided with search callbacks
 */
typedef void (*gui_search_forward_state)(bool active, void *p);

/**
 * activate search back button in gui
 * \param active activate/inactivate
 * \param p gui private data pointer provided with search callbacks
 */
typedef void (*gui_search_back_state)(bool active, void *p);

struct gui_search_callbacks {
	gui_search_forward_state 	forward_state;
	gui_search_back_state		back_state;
	gui_search_status		status;
	gui_search_hourglass		hourglass;
	gui_search_add_recent		add_recent;
};


bool browser_window_search_create_context(struct browser_window *bw, 
		struct gui_search_callbacks *gui_callbacks, void *gui_p);
void browser_window_search_destroy_context(struct browser_window *bw);
bool browser_window_search_verify_new(struct browser_window *bw,
		struct gui_search_callbacks *gui_callbacks, void *gui_p);
void browser_window_search_step(struct browser_window *bw,
		search_flags_t flags, const char *string);
void browser_window_search_show_all(bool all, struct browser_window *bw);

#endif
