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
 * Interface to platform-specific gui functions.
 */

#ifndef _NETSURF_DESKTOP_GUI_H_
#define _NETSURF_DESKTOP_GUI_H_

typedef enum {
	GUI_SAVE_SOURCE,
	GUI_SAVE_DRAW,
	GUI_SAVE_PDF,
	GUI_SAVE_TEXT,
	GUI_SAVE_COMPLETE,
	GUI_SAVE_OBJECT_ORIG,
	GUI_SAVE_OBJECT_NATIVE,
	GUI_SAVE_LINK_URI,
	GUI_SAVE_LINK_URL,
	GUI_SAVE_LINK_TEXT,
	GUI_SAVE_HOTLIST_EXPORT_HTML,
	GUI_SAVE_HISTORY_EXPORT_HTML,
	GUI_SAVE_TEXT_SELECTION,
	GUI_SAVE_CLIPBOARD_CONTENTS
} gui_save_type;

typedef enum {
	GDRAGGING_NONE,
	GDRAGGING_SCROLLBAR,
	GDRAGGING_OTHER
} gui_drag_type;

struct gui_window;
struct gui_download_window;
struct browser_window;
struct selection;
struct form_control;

#include <stdbool.h>

#include <libwapcaplet/libwapcaplet.h>
#include <libcss/libcss.h>

#include "utils/config.h"
#include "content/hlcache.h"
#include "desktop/download.h"
#include "desktop/mouse.h"
#include "desktop/search.h"
#include "utils/errors.h"

/** \todo remove these when each frontend calls nslog_init */
#include <stdio.h>
bool nslog_ensure(FILE *fptr);

void gui_poll(bool active);
void gui_quit(void);

struct gui_window *gui_create_browser_window(struct browser_window *bw,
		struct browser_window *clone, bool new_tab);
void gui_window_destroy(struct gui_window *g);
void gui_window_set_title(struct gui_window *g, const char *title);
void gui_window_redraw_window(struct gui_window *g);
void gui_window_update_box(struct gui_window *g,
		const struct rect *rect);
bool gui_window_get_scroll(struct gui_window *g, int *sx, int *sy);
void gui_window_set_scroll(struct gui_window *g, int sx, int sy);
void gui_window_scroll_visible(struct gui_window *g, int x0, int y0,
		int x1, int y1);
void gui_window_get_dimensions(struct gui_window *g, int *width, int *height,
		bool scaled);
void gui_window_update_extent(struct gui_window *g);
void gui_window_set_status(struct gui_window *g, const char *text);
void gui_window_set_pointer(struct gui_window *g, gui_pointer_shape shape);
void gui_window_hide_pointer(struct gui_window *g);
void gui_window_set_url(struct gui_window *g, const char *url);
void gui_window_start_throbber(struct gui_window *g);
void gui_window_stop_throbber(struct gui_window *g);
void gui_window_set_icon(struct gui_window *g, hlcache_handle *icon);
void gui_window_set_search_ico(hlcache_handle *ico);
void gui_window_place_caret(struct gui_window *g, int x, int y, int height);
void gui_window_remove_caret(struct gui_window *g);
void gui_window_new_content(struct gui_window *g);
bool gui_window_scroll_start(struct gui_window *g);

bool gui_window_drag_start(struct gui_window *g, gui_drag_type type,
		const struct rect *rect);

void gui_window_save_link(struct gui_window *g, const char *url, 
		const char *title);

struct gui_download_window *gui_download_window_create(download_context *ctx,
		struct gui_window *parent);
nserror gui_download_window_data(struct gui_download_window *dw, 
		const char *data, unsigned int size);
void gui_download_window_error(struct gui_download_window *dw,
		const char *error_msg);
void gui_download_window_done(struct gui_download_window *dw);

void gui_drag_save_object(gui_save_type type, hlcache_handle *c,
		struct gui_window *g);
void gui_drag_save_selection(struct selection *s, struct gui_window *g);
void gui_start_selection(struct gui_window *g);
void gui_clear_selection(struct gui_window *g);



/**
 * Core asks front end for clipboard contents.
 *
 * \param  buffer  UTF-8 text, allocated by front end, ownership yeilded to core
 * \param  length  Byte length of UTF-8 text in buffer
 */
void gui_get_clipboard(char **buffer, size_t *length);

typedef struct nsnsclipboard_styles {
	size_t start;			/**< Start of run */

	plot_font_style_t style;	/**< Style to give text run */
} nsclipboard_styles;
/**
 * Core tells front end to put given text in clipboard
 *
 * \param  buffer    UTF-8 text, owned by core
 * \param  length    Byte length of UTF-8 text in buffer
 * \param  styles    Array of styles given to text runs, owned by core, or NULL
 * \param  n_styles  Number of text run styles in array
 */
void gui_set_clipboard(const char *buffer, size_t length,
		nsclipboard_styles styles[], int n_styles);



void gui_create_form_select_menu(struct browser_window *bw,
		struct form_control *control);

void gui_launch_url(const char *url);

struct ssl_cert_info;

void gui_cert_verify(nsurl *url, const struct ssl_cert_info *certs, 
		unsigned long num, nserror (*cb)(bool proceed, void *pw),
		void *cbpw);

/**
 * Callback to translate resource to full url.
 *
 * Transforms a resource: path into a full URL. The returned URL
 * is used as the target for a redirect. The caller takes ownership of
 * the returned nsurl including unrefing it when finished with it.
 *
 * \param path The path of the resource to locate.
 * \return A string containing the full URL of the target object or
 *         NULL if no suitable resource can be found.
 */
nsurl* gui_get_resource_url(const char *path);

/** css callback to obtain named system colours from a frontend. */
css_error gui_system_colour(void *pw, lwc_string *name, css_color *color);

/** Obtain a named system colour from a frontend. */
colour gui_system_colour_char(const char *name);

bool gui_system_colour_init(void);
void gui_system_colour_finalize(void);

#endif
