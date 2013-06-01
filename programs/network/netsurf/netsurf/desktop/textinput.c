/*
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2004 Andrew Timmins <atimmins@blueyonder.co.uk>
 * Copyright 2004 John Tytgat <joty@netsurf-browser.org>
 * Copyright 2005 Adrian Lees <adrianl@users.sourceforge.net>
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
 * Textual input handling (implementation)
 */

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <dom/dom.h>

#include "desktop/browser_private.h"
#include "desktop/gui.h"
#include "desktop/mouse.h"
#include "desktop/scrollbar.h"
#include "desktop/selection.h"
#include "desktop/textinput.h"
#include "render/box.h"
#include "render/font.h"
#include "render/form.h"
#include "render/html_internal.h"
#include "render/layout.h"
#include "utils/log.h"
#include "utils/talloc.h"
#include "utils/utf8.h"
#include "utils/utils.h"

/* Define to enable textinput debug */
#undef TEXTINPUT_DEBUG


/**
 * Position the caret and assign a callback for key presses.
 *
 * \param bw  The browser window in which to place the caret
 * \param x   X coordinate of the caret
 * \param y   Y coordinate
 * \param height    Height of caret
 * \param caret_cb  Callback function for keypresses
 * \param paste_cb  Callback function for pasting text
 * \param move_cb   Callback function for caret movement
 * \param p1  Callback private data pointer, passed to callback function
 * \param p2  Callback private data pointer, passed to callback function
 */
void browser_window_place_caret(struct browser_window *bw,
		int x, int y, int height,
		browser_caret_callback caret_cb,
		browser_paste_callback paste_cb,
		browser_move_callback move_cb,
		void *p1, void *p2)
{
	struct browser_window *root_bw;
	int pos_x = 0;
	int pos_y = 0;

	/* Find top level browser window */
	root_bw = browser_window_get_root(bw);
	browser_window_get_position(bw, true, &pos_x, &pos_y);

	x = x * bw->scale + pos_x;
	y = y * bw->scale + pos_y;

	gui_window_place_caret(root_bw->window, x, y, height * bw->scale);
	bw->caret_callback = caret_cb;
	bw->paste_callback = paste_cb;
	bw->move_callback = move_cb;
	bw->caret_p1 = p1;
	bw->caret_p2 = p2;

	/* Set focus browser window */
	root_bw->focus = bw;
}


/**
 * Removes the caret and callback for key process.
 *
 * \param bw  The browser window from which to remove caret
 */
void browser_window_remove_caret(struct browser_window *bw)
{
	struct browser_window *root_bw;

	root_bw = browser_window_get_root(bw);

	if (root_bw && root_bw->window)
		gui_window_remove_caret(root_bw->window);

	bw->caret_callback = NULL;
	bw->paste_callback = NULL;
	bw->move_callback = NULL;
	bw->caret_p1 = NULL;
	bw->caret_p2 = NULL;
}


/**
 * Handle key presses in a browser window.
 *
 * \param bw   The root browser window
 * \param key  The UCS4 character codepoint
 * \return true if key handled, false otherwise
 */
bool browser_window_key_press(struct browser_window *bw, uint32_t key)
{
	struct browser_window *focus = bw->focus;

	assert(bw->window != NULL);

	/* safe keys that can be handled whether input claimed or not */
	switch (key) {
		case KEY_COPY_SELECTION:
			selection_copy_to_clipboard(bw->cur_sel);
			return true;

		case KEY_CLEAR_SELECTION:
			selection_clear(bw->cur_sel, true);
			return true;

		case KEY_ESCAPE:
			if (bw->cur_sel && selection_defined(bw->cur_sel)) {
				selection_clear(bw->cur_sel, true);
				return true;
			}
			/* if there's no selection,
			 * leave Escape for the caller */
			return false;
	}

	if (focus->caret_callback) {
		/* Pass keypress onto anything that has claimed input focus */
		return focus->caret_callback(focus, key,
				focus->caret_p1, focus->caret_p2);
	}

	/* keys we can't handle here if cursor is in form */
	switch (key) {
		case KEY_SELECT_ALL:
			selection_select_all(bw->cur_sel);
			return true;
	}

	return false;
}


/**
 * Paste a block of text into a browser window at the caret position.
 *
 * \param  bw        browser window
 * \param  utf8      pointer to block of text
 * \param  utf8_len  length (bytes) of text block
 * \param  last      true iff this is the last chunk (update screen too)
 * \return true iff successful
 *
 * TODO: Remove this function.
 */

bool browser_window_paste_text(struct browser_window *bw, const char *utf8,
		unsigned utf8_len, bool last)
{
	if (!bw->focus || !bw->focus->paste_callback)
		return false;

	return bw->focus->paste_callback(bw->focus, utf8, utf8_len, last,
			bw->focus->caret_p1, bw->focus->caret_p2);
}

