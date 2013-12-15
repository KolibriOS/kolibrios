/*
 * Copyright 2006 John-Mark Bell <jmb@netsurf-browser.org>
 * Copyright 2009 Paul Blokus <paul_pl@users.sourceforge.net> 
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
 * Single/Multi-line UTF-8 text area (interface)
 */

#ifndef _NETSURF_DESKTOP_TEXTAREA_H_
#define _NETSURF_DESKTOP_TEXTAREA_H_

#include <stdint.h>
#include <stdbool.h>
#include "desktop/browser.h"
#include "desktop/plot_style.h"


struct textarea;

/* Text area flags */
typedef enum {
	TEXTAREA_DEFAULT		= (1 << 0),	/**< Standard input */
	TEXTAREA_MULTILINE		= (1 << 1),	/**< Multiline area */
	TEXTAREA_READONLY		= (1 << 2),	/**< Non-editable */
	TEXTAREA_INTERNAL_CARET		= (1 << 3),	/**< Render own caret */
	TEXTAREA_PASSWORD		= (1 << 4)	/**< Obscured display */
} textarea_flags;

typedef enum {
	TEXTAREA_DRAG_NONE,
	TEXTAREA_DRAG_SCROLLBAR,
	TEXTAREA_DRAG_SELECTION
} textarea_drag_type;

typedef enum {
	TEXTAREA_MSG_DRAG_REPORT,	/**< Textarea drag start/end report */
	TEXTAREA_MSG_REDRAW_REQUEST,	/**< Textarea redraw request */
	TEXTAREA_MSG_MOVED_CARET	/**< Textarea caret moved */
} textarea_msg_type;

struct textarea_msg {
	struct textarea *ta;

	textarea_msg_type type;
	union {
		textarea_drag_type drag;
		struct rect redraw;
		struct {
			bool hidden;
			int x;
			int y;
			int height;
		} caret;
	} data;
};

typedef struct textarea_setup {
	textarea_flags flags;	/**< Setup flags */

	int width;		/**< Textarea width */
	int height;		/**< Textarea height */

	int pad_top;		/**< Textarea top padding */
	int pad_right;		/**< Textarea right padding */
	int pad_bottom;		/**< Textarea bottom padding */
	int pad_left;		/**< Textarea left padding */

	int border_width;	/**< Textarea border width */
	colour border_col;	/**< Textarea border colour */

	colour selected_text;	/**< Textarea selected text colour */
	colour selected_bg;	/**< Textarea selection background colour */
	plot_font_style_t text;	/**< Textarea background colour and font */

} textarea_setup;

/**
 * Client callback for the textarea
 *
 * \param data		user data passed at textarea creation
 * \param textarea_msg	textarea message data
 */
typedef void(*textarea_client_callback)(void *data, struct textarea_msg *msg);

/**
 * Create a text area
 *
 * \param setup	textarea settings and style
 * \param redraw_callback will be called when textarea wants to redraw
 * \param data	user specified data which will be passed to callbacks
 * \return Opaque handle for textarea or 0 on error
 */
struct textarea *textarea_create(const textarea_setup *setup,
		textarea_client_callback callback, void *data);

/**
 * Destroy a text area
 *
 * \param ta Text area to destroy
 */
void textarea_destroy(struct textarea *ta);

/**
 * Set the text in a text area, discarding any current text
 *
 * \param ta Text area
 * \param text UTF-8 text to set text area's contents to
 * \return true on success, false on memory exhaustion
 */
bool textarea_set_text(struct textarea *ta, const char *text);

/**
 * Extract the text from a text area
 *
 * \param ta Text area
 * \param buf Pointer to buffer to receive data, or NULL
 *            to read length required
 * \param len Length (bytes) of buffer pointed to by buf, or 0 to read length
 * \return Length (bytes) written/required or -1 on error
 */
int textarea_get_text(struct textarea *ta, char *buf, unsigned int len);

/**
 * Set the caret's position
 *
 * \param ta 		Text area
 * \param caret 	0-based character index to place caret at, -1 removes
 * 			the caret
 * \return true on success false otherwise
 */
bool textarea_set_caret(struct textarea *ta, int caret);

/**
 * Get the caret's position
 *
 * \param ta Text area
 * \return 0-based character index of caret location, or -1 on error
 */
int textarea_get_caret(struct textarea *ta);

/**
 * Handle redraw requests for text areas
 *
 * \param ta	textarea to render
 * \param x	x coordinate of textarea top
 * \param y	y coordinate of textarea left
 * \param bg	background colour under textarea
 * \param clip	clip rectangle
 * \param ctx	current redraw context
 */
void textarea_redraw(struct textarea *ta, int x, int y, colour bg,
		const struct rect *clip, const struct redraw_context *ctx);

/**
 * Key press handling for text areas.
 *
 * \param ta	The text area which got the keypress
 * \param key	The ucs4 character codepoint
 * \return     	true if the keypress is dealt with, false otherwise.
 */
bool textarea_keypress(struct textarea *ta, uint32_t key);

/**
 * Handles all kinds of mouse action
 *
 * \param ta	Text area
 * \param mouse	the mouse state at action moment
 * \param x	X coordinate
 * \param y	Y coordinate
 * \return true if action was handled false otherwise
 */
bool textarea_mouse_action(struct textarea *ta, browser_mouse_state mouse,
		int x, int y);

/**
 * Gets the dimensions of a textarea
 *
 * \param width		if not NULL, gets updated to the width of the textarea
 * \param height	if not NULL, gets updated to the height of the textarea
 */
void textarea_get_dimensions(struct textarea *ta, int *width, int *height);

/**
 * Set the dimensions of a textarea, causing a reflow and
 * emitting a redraw request.
 *
 * \param width 	the new width of the textarea
 * \param height	the new height of the textarea
 */
void textarea_set_dimensions(struct textarea *ta, int width, int height);
#endif

