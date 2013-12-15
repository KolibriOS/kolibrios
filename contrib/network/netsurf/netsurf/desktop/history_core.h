/*
 * Copyright 2006 James Bursa <bursa@users.sourceforge.net>
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
 * Browser history tree (interface).
 */

#ifndef _NETSURF_DESKTOP_HISTORY_H_
#define _NETSURF_DESKTOP_HISTORY_H_

#include <stdbool.h>

struct hlcache_handle;
struct history;
struct browser_window;
struct history_entry;
struct redraw_context;

struct history *history_create(void);
struct history *history_clone(struct history *history);
void history_add(struct history *history, struct hlcache_handle *content,
		const char *frag_id);
void history_update(struct history *history, struct hlcache_handle *content);
void history_destroy(struct history *history);
void history_back(struct browser_window *bw, struct history *history);
void history_forward(struct browser_window *bw, struct history *history);
bool history_back_available(struct history *history);
bool history_forward_available(struct history *history);
void history_size(struct history *history, int *width, int *height);
bool history_redraw(struct history *history, const struct redraw_context *ctx);
bool history_redraw_rectangle(struct history *history,
		int x0, int y0, int x1, int y1, int x, int y,
		const struct redraw_context *ctx);
bool history_click(struct browser_window *bw, struct history *history,
		int x, int y, bool new_window);
const char *history_position_url(struct history *history, int x, int y);

/**
 * Callback function type for history enumeration
 *
 * \param	history			history being enumerated
 * \param	x0, y0, x1, y1	Coordinates of entry in history tree view
 * \param	entry			Current history entry
 * \return	true to continue enumeration, false to cancel enumeration
 */
typedef bool (*history_enumerate_cb)(const struct history *history, int x0, int y0, 
	 int x1, int y1, 
	 const struct history_entry *entry, void *user_data);

/**
 * Enumerate all entries in the history.
 * Do not change the history while it is being enumerated.
 *
 * \param	history		history to enumerate
 * \param	cb			callback function
 * \param	user_data	context pointer passed to cb
 */
void history_enumerate(const struct history *history, history_enumerate_cb cb, void *user_data);

/**
 * Enumerate all entries that will be reached by the 'forward' button
 *
 * \param	history		The history object to enumerate in
 * \param	cb			The callback function
 * \param	user_data	Data passed to the callback
 */
void history_enumerate_forward( struct history *history, 
		history_enumerate_cb cb, void *user_data );

/**
 * Enumerate all entries that will be reached by the 'back' button
 *
 * \param	history		The history object to enumerate in
 * \param	cb			The callback function
 * \param	user_data	Data passed to the callback
 */
void history_enumerate_back( struct history *history, 
		history_enumerate_cb cb, void *user_data );

/**
 * Returns the URL to a history entry
 *
 * \param	entry		the history entry to retrieve the URL from
 * \return	the URL
 */
const char *history_entry_get_url(const struct history_entry *entry);

/**
 * Returns the URL to a history entry
 *
 * \param	entry		the history entry to retrieve the fragment id from
 * \return	the fragment id
 */
const char *history_entry_get_fragment_id(const struct history_entry *entry);

/**
 * Returns the title of a history entry
 *
 * \param	entry		the history entry to retrieve the title from
 * \return	the title
 */
const char *history_entry_get_title(const struct history_entry *entry);

/**
 * Open a history entry in the specified browser window
 *
 * \param  bw          browser window
 * \param  history     history containing entry
 * \param  entry       entry to open
 * \param  new_window  open entry in new window
 */
void history_go(struct browser_window *bw, struct history *history,
				struct history_entry *entry, bool new_window);

#endif
