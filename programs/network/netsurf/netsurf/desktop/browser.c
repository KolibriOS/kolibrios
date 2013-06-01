/*
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2006 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2004 Andrew Timmins <atimmins@blueyonder.co.uk>
 * Copyright 2004 John Tytgat <joty@netsurf-browser.org>
 * Copyright 2006 Richard Wilson <info@tinct.net>
 * Copyright 2008 Michael Drake <tlsa@netsurf-browser.org>
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
 * Browser window creation and manipulation (implementation).
 */

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#include "curl/curl.h"
#include "utils/config.h"
#include "content/content.h"
#include "content/fetch.h"
#include "content/hlcache.h"
#include "content/urldb.h"
#include "desktop/401login.h"
#include "desktop/browser_private.h"
#include "desktop/download.h"
#include "desktop/frames.h"
#include "desktop/history_core.h"
#include "desktop/hotlist.h"
#include "desktop/gui.h"
#include "desktop/knockout.h"
#include "desktop/options.h"
#include "desktop/scrollbar.h"
#include "desktop/selection.h"
#include "desktop/textinput.h"
#include "desktop/plotters.h"

#include "javascript/js.h"

#include "render/form.h"
#include "render/textplain.h"
#include "render/html.h"
#include "render/box.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/nsurl.h"
#include "utils/schedule.h"
#include "utils/url.h"
#include "utils/utils.h"
#include "utils/utf8.h"

/** one or more windows require a reformat */
bool browser_reformat_pending;

/** maximum frame depth */
#define FRAME_DEPTH 8

static nserror browser_window_callback(hlcache_handle *c,
		const hlcache_event *event, void *pw);
static void browser_window_refresh(void *p);
static bool browser_window_check_throbber(struct browser_window *bw);
static void browser_window_convert_to_download(struct browser_window *bw, 
		llcache_handle *stream);
static void browser_window_start_throbber(struct browser_window *bw);
static void browser_window_stop_throbber(struct browser_window *bw);
static void browser_window_destroy_children(struct browser_window *bw);
static void browser_window_destroy_internal(struct browser_window *bw);
static void browser_window_set_scale_internal(struct browser_window *bw,
		float scale);
static void browser_window_find_target_internal(struct browser_window *bw,
		const char *target, int depth, struct browser_window *page,
		int *rdepth, struct browser_window **bw_target);
static void browser_window_mouse_drag_end(struct browser_window *bw,
		browser_mouse_state mouse, int x, int y);


/**
 * Get position of scrollbar widget within browser window.
 *
 * \param  bw		The browser window
 * \param  horizontal	Whether to get position of horizontal scrollbar
 * \param  x		Updated to x-coord of top left of scrollbar widget
 * \param  y		Updated to y-coord of top left of scrollbar widget
 */

static inline void browser_window_get_scrollbar_pos(struct browser_window *bw,
		bool horizontal, int *x, int *y)
{
	if (horizontal) {
		*x = 0;
		*y = bw->height - SCROLLBAR_WIDTH;
	} else {
		*x = bw->width - SCROLLBAR_WIDTH;
		*y = 0;
	}
}


/**
 * Get browser window scrollbar widget length
 *
 * \param  bw		The browser window
 * \param  horizontal	Whether to get length of horizontal scrollbar
 * \return the scrollbar's length
 */

static inline int browser_window_get_scrollbar_len(struct browser_window *bw,
		bool horizontal)
{
	if (horizontal)
		return bw->width - (bw->scroll_y != NULL ? SCROLLBAR_WIDTH : 0);
	else
		return bw->height;
}

/* exported interface, documented in browser.h */
bool browser_window_redraw(struct browser_window *bw, int x, int y,
		const struct rect *clip, const struct redraw_context *ctx)
{
	struct redraw_context new_ctx = *ctx;
	int width = 0;
	int height = 0;
	bool plot_ok = true;
	content_type content_type;
	struct content_redraw_data data;
	struct rect content_clip;

	if (bw == NULL) {
		LOG(("NULL browser window"));
		return false;
	}

	if (bw->current_content == NULL && bw->children == NULL) {
		/* Browser window has no content, render blank fill */
		ctx->plot->clip(clip);
		return ctx->plot->rectangle(clip->x0, clip->y0,
				clip->x1, clip->y1,
				plot_style_fill_white);
	}

	/* Browser window has content OR children (frames) */

	if ((bw->window != NULL) && 
	    (ctx->plot->option_knockout)) {
		/* Root browser window: start knockout */
		knockout_plot_start(ctx, &new_ctx);
	}

	new_ctx.plot->clip(clip);

	/* Handle redraw of any browser window children */
	if (bw->children) {
		struct browser_window *child;
		int cur_child;
		int children = bw->rows * bw->cols;

		if (bw->window != NULL)
			/* Root browser window; start with blank fill */
			plot_ok &= new_ctx.plot->rectangle(clip->x0, clip->y0,
					clip->x1, clip->y1,
					plot_style_fill_white);

		/* Loop through all children of bw */
		for (cur_child = 0; cur_child < children; cur_child++) {
			/* Set current child */
			child = &bw->children[cur_child];

			/* Get frame edge box in global coordinates */
			content_clip.x0 = (x + child->x) * child->scale;
			content_clip.y0 = (y + child->y) * child->scale;
			content_clip.x1 = content_clip.x0 +
					child->width * child->scale;
			content_clip.y1 = content_clip.y0 +
					child->height * child->scale;

			/* Intersect it with clip rectangle */
			if (content_clip.x0 < clip->x0)
				content_clip.x0 = clip->x0;
			if (content_clip.y0 < clip->y0)
				content_clip.y0 = clip->y0;
			if (clip->x1 < content_clip.x1)
				content_clip.x1 = clip->x1;
			if (clip->y1 < content_clip.y1)
				content_clip.y1 = clip->y1;

			/* Skip this frame if it lies outside clip rectangle */
			if (content_clip.x0 >= content_clip.x1 ||
					content_clip.y0 >= content_clip.y1)
				continue;

			/* Redraw frame */
			plot_ok &= browser_window_redraw(child,
					x + child->x, y + child->y,
					&content_clip, &new_ctx);
		}

		/* Nothing else to redraw for browser windows with children;
		 * cleanup and return */
		if (bw->window != NULL && ctx->plot->option_knockout) {
			/* Root browser window: knockout end */
			knockout_plot_end();
		}

		return plot_ok;
	}

	/* Handle browser windows with content to redraw */

	content_type = content_get_type(bw->current_content);
	if (content_type != CONTENT_HTML && content_type != CONTENT_TEXTPLAIN) {
		/* Set render area according to scale */
		width = content_get_width(bw->current_content) * bw->scale;
		height = content_get_height(bw->current_content) * bw->scale;

		/* Non-HTML may not fill viewport to extents, so plot white
		 * background fill */
		plot_ok &= new_ctx.plot->rectangle(clip->x0, clip->y0,
				clip->x1, clip->y1, plot_style_fill_white);
	}

	/* Set up content redraw data */
	data.x = x - scrollbar_get_offset(bw->scroll_x);
	data.y = y - scrollbar_get_offset(bw->scroll_y);
	data.width = width;
	data.height = height;

	data.background_colour = 0xFFFFFF;
	data.scale = bw->scale;
	data.repeat_x = false;
	data.repeat_y = false;

	content_clip = *clip;

	if (!bw->window) {
		int x0 = x * bw->scale;
		int y0 = y * bw->scale;
		int x1 = (x + bw->width - ((bw->scroll_y != NULL) ?
				SCROLLBAR_WIDTH : 0)) * bw->scale;
		int y1 = (y + bw->height - ((bw->scroll_x != NULL) ?
				SCROLLBAR_WIDTH : 0)) * bw->scale;

		if (content_clip.x0 < x0) content_clip.x0 = x0;
		if (content_clip.y0 < y0) content_clip.y0 = y0;
		if (x1 < content_clip.x1) content_clip.x1 = x1;
		if (y1 < content_clip.y1) content_clip.y1 = y1;
	}
 
	/* Render the content */
	plot_ok &= content_redraw(bw->current_content, &data,
			&content_clip, &new_ctx);

	/* Back to full clip rect */
	new_ctx.plot->clip(clip);

	if (!bw->window) {
		/* Render scrollbars */
		int off_x, off_y;
		if (bw->scroll_x != NULL) {
			browser_window_get_scrollbar_pos(bw, true,
					&off_x, &off_y);
			plot_ok &= scrollbar_redraw(bw->scroll_x,
					x + off_x, y + off_y, clip,
					bw->scale, &new_ctx);
		}
		if (bw->scroll_y != NULL) {
			browser_window_get_scrollbar_pos(bw, false,
					&off_x, &off_y);
			plot_ok &= scrollbar_redraw(bw->scroll_y,
					x + off_x, y + off_y, clip,
					bw->scale, &new_ctx);
		}
	}
	
	if (bw->window != NULL && ctx->plot->option_knockout) {
		/* Root browser window: end knockout */
		knockout_plot_end();
	}

	return plot_ok;
}

/* exported interface, documented in browser.h */
bool browser_window_redraw_ready(struct browser_window *bw)
{
	if (bw == NULL) {
		LOG(("NULL browser window"));
		return false;
	} else if (bw->current_content != NULL) {
		/* Can't render locked contents */
		return !content_is_locked(bw->current_content);
	}

	return true;
}

/* exported interface, documented in browser.h */
void browser_window_update_extent(struct browser_window *bw)
{
	if (bw->window != NULL)
		/* Front end window */
		gui_window_update_extent(bw->window);
	else
		/* Core-managed browser window */
		browser_window_handle_scrollbars(bw);
}

/* exported interface, documented in browser.h */
void browser_window_get_position(struct browser_window *bw, bool root,
		int *pos_x, int *pos_y)
{
	*pos_x = 0;
	*pos_y = 0;

	assert(bw != NULL);

	while (bw) {
		switch (bw->browser_window_type) {

		case BROWSER_WINDOW_FRAMESET:
			*pos_x += bw->x * bw->scale;
			*pos_y += bw->y * bw->scale;
			break;

		case BROWSER_WINDOW_NORMAL:
			/* There is no offset to the root browser window */
			break;

		case BROWSER_WINDOW_FRAME:
			/* Iframe and Frame handling is identical;
			 * fall though */
		case BROWSER_WINDOW_IFRAME:
			*pos_x += (bw->x - scrollbar_get_offset(bw->scroll_x)) *
					bw->scale;
			*pos_y += (bw->y - scrollbar_get_offset(bw->scroll_y)) *
					bw->scale;
			break;
		}

		bw = bw->parent;

		if (!root) {
			/* return if we just wanted the position in the parent
			 * browser window. */
			return;
		}
	}
}


/* exported interface, documented in browser.h */
void browser_window_set_position(struct browser_window *bw, int x, int y)
{
	assert(bw != NULL);

	if (bw->window == NULL) {
		/* Core managed browser window */
		bw->x = x;
		bw->y = y;
	} else {
		LOG(("Asked to set position of front end window."));
		assert(0);
	}
}

/* exported interface, documented in browser.h */
void browser_window_set_drag_type(struct browser_window *bw,
		browser_drag_type type, const struct rect *rect)
{
	struct browser_window *top_bw = browser_window_get_root(bw);
	gui_drag_type gtype;

	bw->drag_type = type;

	if (type == DRAGGING_NONE) {
		top_bw->drag_window = NULL;
	} else {
		top_bw->drag_window = bw;

		switch (type) {
		case DRAGGING_SCR_X:
		case DRAGGING_SCR_Y:
		case DRAGGING_CONTENT_SCROLLBAR:
			gtype = GDRAGGING_SCROLLBAR;
			break;
		default:
			gtype = GDRAGGING_OTHER;
			break;
		}

		gui_window_drag_start(top_bw->window, gtype, rect);
	}
}

/* exported interface, documented in browser.h */
browser_drag_type browser_window_get_drag_type(struct browser_window *bw)
{
	return bw->drag_type;
}

/* exported interface, documented in browser.h */
struct browser_window * browser_window_get_root(struct browser_window *bw)
{
	while (bw && bw->parent) {
		bw = bw->parent;
	}
	return bw;
}

/* exported interface, documented in browser.h */
bool browser_window_has_selection(struct browser_window *bw)
{
	assert(bw->window);

	if (bw->cur_sel != NULL && selection_defined(bw->cur_sel)) {
		return true;
	} else {
		return false;
	}
}

/* exported interface, documented in browser.h */
void browser_window_set_selection(struct browser_window *bw,
		struct selection *s)
{
	assert(bw->window);

	if (bw->cur_sel != s && bw->cur_sel != NULL) {
		/* Clear any existing selection */
		selection_clear(bw->cur_sel, true);
	}

	/* Replace current selection pointer */
	if (s == NULL && bw->current_content != NULL) {
		bw->cur_sel = content_get_selection(bw->current_content);
	} else {
		bw->cur_sel = s;
	}
}

/* exported interface, documented in browser.h */
struct selection *browser_window_get_selection(struct browser_window *bw)
{
	assert(bw->window);

	return bw->cur_sel;
}

/* exported interface, documented in browser.h */
void browser_window_scroll_visible(struct browser_window *bw,
		const struct rect *rect)
{
	assert(bw != NULL);

	if (bw->window != NULL) {
		/* Front end window */
		gui_window_scroll_visible(bw->window,
				rect->x0, rect->y0, rect->x1, rect->y1);
	} else {
		/* Core managed browser window */
		if (bw->scroll_x != NULL)
			scrollbar_set(bw->scroll_x, rect->x0, false);
		if (bw->scroll_y != NULL)
			scrollbar_set(bw->scroll_y, rect->y0, false);
	}
}

/* exported interface, documented in browser.h */
void browser_window_set_scroll(struct browser_window *bw, int x, int y)
{
	if (bw->window != NULL) {
		gui_window_set_scroll(bw->window, x, y);
	} else {
		if (bw->scroll_x != NULL)
			scrollbar_set(bw->scroll_x, x, false);
		if (bw->scroll_y != NULL)
			scrollbar_set(bw->scroll_y, y, false);
	}
}

/**
 * Internal helper for browser_window_get_contextual_content
 */
static void browser_window__get_contextual_content(struct browser_window *bw,
		int x, int y, struct contextual_content *data)
{
	/* Handle (i)frame scroll offset (core-managed browser windows only) */
	x += scrollbar_get_offset(bw->scroll_x);
	y += scrollbar_get_offset(bw->scroll_y);

	if (bw->children) {
		/* Browser window has children, so pass request on to
		 * appropriate child */
		struct browser_window *bwc;
		int cur_child;
		int children = bw->rows * bw->cols;

		/* Loop through all children of bw */
		for (cur_child = 0; cur_child < children; cur_child++) {
			/* Set current child */
			bwc = &bw->children[cur_child];

			/* Skip this frame if (x, y) coord lies outside */
			if (x < bwc->x || bwc->x + bwc->width < x ||
					y < bwc->y || bwc->y + bwc->height < y)
				continue;

			/* Pass request into this child */
			browser_window__get_contextual_content(bwc,
					(x - bwc->x), (y - bwc->y), data);
			return;
		}

		/* Coordinate not contained by any frame */
		return;
	}

	if (bw->current_content == NULL)
		/* No content; nothing to set */
		return;

	/* Pass request to content */
	content_get_contextual_content(bw->current_content, x, y, data);
	data->main = bw->current_content;
}

/* exported interface, documented in browser.h */
void browser_window_get_contextual_content(struct browser_window *bw,
		int x, int y, struct contextual_content *data)
{
	data->link_url = NULL;
	data->object = NULL;
	data->main = NULL;
	data->form_features = CTX_FORM_NONE;

	browser_window__get_contextual_content(bw, x, y, data);
}

/* exported interface, documented in browser.h */
bool browser_window_scroll_at_point(struct browser_window *bw,
		int x, int y, int scrx, int scry)
{
	bool handled_scroll = false;
	assert(bw != NULL);

	/* Handle (i)frame scroll offset (core-managed browser windows only) */
	x += scrollbar_get_offset(bw->scroll_x);
	y += scrollbar_get_offset(bw->scroll_y);

	if (bw->children) {
		/* Browser window has children, so pass request on to
		 * appropriate child */
		struct browser_window *bwc;
		int cur_child;
		int children = bw->rows * bw->cols;

		/* Loop through all children of bw */
		for (cur_child = 0; cur_child < children; cur_child++) {
			/* Set current child */
			bwc = &bw->children[cur_child];

			/* Skip this frame if (x, y) coord lies outside */
			if (x < bwc->x || bwc->x + bwc->width < x ||
					y < bwc->y || bwc->y + bwc->height < y)
				continue;

			/* Pass request into this child */
			return browser_window_scroll_at_point(bwc,
					(x - bwc->x), (y - bwc->y),
					scrx, scry);
		}
	}

	/* Try to scroll any current content */
	if (bw->current_content != NULL && content_scroll_at_point(
			bw->current_content, x, y, scrx, scry) == true)
		/* Scroll handled by current content */
		return true;

	/* Try to scroll this window, if scroll not already handled */
	if (handled_scroll == false) {
		if (bw->scroll_y && scrollbar_scroll(bw->scroll_y, scry))
			handled_scroll = true;

		if (bw->scroll_x && scrollbar_scroll(bw->scroll_x, scrx))
			handled_scroll = true;
	}

	return handled_scroll;
}

/* exported interface, documented in browser.h */
bool browser_window_drop_file_at_point(struct browser_window *bw,
		int x, int y, char *file)
{
	assert(bw != NULL);

	/* Handle (i)frame scroll offset (core-managed browser windows only) */
	x += scrollbar_get_offset(bw->scroll_x);
	y += scrollbar_get_offset(bw->scroll_y);

	if (bw->children) {
		/* Browser window has children, so pass request on to
		 * appropriate child */
		struct browser_window *bwc;
		int cur_child;
		int children = bw->rows * bw->cols;

		/* Loop through all children of bw */
		for (cur_child = 0; cur_child < children; cur_child++) {
			/* Set current child */
			bwc = &bw->children[cur_child];

			/* Skip this frame if (x, y) coord lies outside */
			if (x < bwc->x || bwc->x + bwc->width < x ||
					y < bwc->y || bwc->y + bwc->height < y)
				continue;

			/* Pass request into this child */
			return browser_window_drop_file_at_point(bwc,
					(x - bwc->x), (y - bwc->y),
					file);
		}
	}

	/* Pass file drop on to any content */
	if (bw->current_content != NULL)
		return content_drop_file_at_point(bw->current_content,
				x, y, file);

	return false;
}

/* exported interface, documented in browser.h */
void browser_window_debug_dump(struct browser_window *bw, FILE *f)
{
	if (bw->current_content != NULL)
		content_debug_dump(bw->current_content, f);
}


/**
 * Create and open a new root browser window with the given page.
 *
 * \param  url	    URL to start fetching in the new window (copied)
 * \param  clone    The browser window to clone
 * \param  referer  The referring uri (copied), or 0 if none
 * \param history_add add to history
 * \param new_tab create a new tab
 * \return new browser window or NULL on error
 */

struct browser_window *browser_window_create(const char *url,
		struct browser_window *clone,
		const char *referer, bool history_add, bool new_tab)
{
	struct browser_window *bw;
	struct browser_window *top;

	LOG(("start bwc"));
	
	assert(clone || history_add);

	if ((bw = calloc(1, sizeof *bw)) == NULL) {
		warn_user("NoMemory", 0);
		return NULL;
	}

	LOG(("new js..."));
	

	bw->jsctx = js_newcontext();

	LOG(("start bw init common"));
	
	/* Initialise common parts */
	browser_window_initialise_common(bw, clone);

	LOG(("set props"));
	
	/* window characteristics */
	bw->browser_window_type = BROWSER_WINDOW_NORMAL;
	bw->scrolling = SCROLLING_YES;
	bw->border = true;
	bw->no_resize = true;
	LOG(("wallclock!!!"));
	
	bw->last_action = wallclock();
	bw->focus = bw;

	/* gui window */
	/* from the front end's pov, it clones the top level browser window,
	 * so find that. */
	
	LOG(("get root"));
	
	
	top = browser_window_get_root(clone);

	LOG(("start bw gui create"));
	

	bw->window = gui_create_browser_window(bw, top, new_tab);

	if (bw->window == NULL) {
		LOG(("destroy window"));
	
		browser_window_destroy(bw);
		return NULL;
	}

	LOG(("url?"));
	
	if (url) {
		LOG(("bw go go go"));
	
		browser_window_go(bw, url, referer, history_add);
	}

	LOG(("success bwc"));
	
	return bw;
}


/**
 * Initialise common parts of a browser window
 *
 * \param bw     The window to initialise
 * \param clone  The window to clone, or NULL if none
 */
void browser_window_initialise_common(struct browser_window *bw,
		struct browser_window *clone)
{
	LOG(("bwc init common IN"));
	assert(bw);

	LOG(("check clone..."));

	if (!clone){
		bw->history = history_create();
		LOG(("hist create"));
		}
	else {
		bw->history = history_clone(clone->history);
	LOG(("hist clone"));}


	LOG(("winchar"));
	/* window characteristics */
	bw->cur_sel = NULL;
	bw->cur_search = NULL;
	bw->refresh_interval = -1;

	bw->reformat_pending = false;
	bw->drag_type = DRAGGING_NONE;
	LOG(("nsoption int scale"));
	bw->scale = (float) nsoption_int(scale) / 100.0;


	bw->scroll_x = NULL;
	bw->scroll_y = NULL;

	bw->focus = NULL;

	/* initialise status text cache */
	bw->status_text = NULL;
	bw->status_text_len = 0;
	bw->status_match = 0;
	bw->status_miss = 0;
LOG(("fine bwc init OK"));

}


/**
 * Start fetching a page in a browser window.
 *
 * \param  bw	    browser window
 * \param  url	    URL to start fetching (copied)
 * \param  referer  the referring uri (copied), or 0 if none
 * \param history_add Add to history
 *
 * Any existing fetches in the window are aborted.
 */

void browser_window_go(struct browser_window *bw, const char *url,
		const char *referer, bool history_add)
{
	/* All fetches passing through here are verifiable
	 * (i.e are the result of user action) */
	browser_window_go_post(bw, url, 0, 0, history_add, referer,
			false, true, NULL);
}


/**
 * Start a download of the given URL from a browser window.
 *
 * \param  bw	    browser window
 * \param  url	    URL to start downloading (copied)
 * \param  referer  the referring uri (copied), or 0 if none
 */

void browser_window_download(struct browser_window *bw, const char *url,
		const char *referer)
{
	browser_window_go_post(bw, url, 0, 0, false, referer,
			true, true, NULL);
}


/**
 * Start fetching a page in a browser window.
 *
 * \param  bw	    browser window
 * \param  url	    URL to start fetching (copied)
 * \param  referer  the referring uri (copied), or 0 if none
 * \param history_add add to history
 * \param parent parent handle
 *
 * Any existing fetches in the window are aborted.
 */

void browser_window_go_unverifiable(struct browser_window *bw,
		const char *url, const char *referer, bool history_add,
		hlcache_handle *parent)
{
	/* All fetches passing through here are unverifiable
	 * (i.e are not the result of user action) */
	browser_window_go_post(bw, url, 0, 0, history_add, referer,
			false, false, parent);
}

/**
 * Start fetching a page in a browser window, POSTing form data.
 *
 * \param  bw		   browser window
 * \param  url		   URL to start fetching (copied)
 * \param  post_urlenc	   url encoded post data, or 0 if none
 * \param  post_multipart  multipart post data, or 0 if none
 * \param  add_to_history  add to window history
 * \param  referer	   the referring uri (copied), or 0 if none
 * \param  download	   download, rather than render the uri
 * \param  verifiable	   this transaction is verifiable
 * \param  parent	   Parent content, or NULL
 *
 * Any existing fetches in the window are aborted.
 *
 * If post_urlenc and post_multipart are 0 the url is fetched using GET.
 *
 * The page is not added to the window history if add_to_history is false.
 * This should be used when returning to a page in the window history.
 */

void browser_window_go_post(struct browser_window *bw, const char *url,
		char *post_urlenc,
		struct fetch_multipart_data *post_multipart,
		bool add_to_history, const char *referer, bool download,
		bool verifiable, hlcache_handle *parent)
{
	
	LOG(("GO POST"));
	hlcache_handle *c;
	int depth = 0;
	struct browser_window *cur;
	uint32_t fetch_flags = 0;
	bool fetch_is_post = (post_urlenc != NULL || post_multipart != NULL);
	llcache_post_data post;
	hlcache_child_context child;
	nserror error;

	nsurl *nsref = NULL;
	nsurl *nsurl;

	LOG(("bw %p, url %s", bw, url));
	assert(bw);
	assert(url);

	/* don't allow massively nested framesets */
	for (cur = bw; cur->parent; cur = cur->parent)
		depth++;
	if (depth > FRAME_DEPTH) {
		LOG(("frame depth too high."));
		return;
	}

	/* Set up retrieval parameters */
	if (verifiable)
		fetch_flags |= LLCACHE_RETRIEVE_VERIFIABLE;

	if (post_multipart != NULL) {
		post.type = LLCACHE_POST_MULTIPART;
		post.data.multipart = post_multipart;
	} else if (post_urlenc != NULL) {
		post.type = LLCACHE_POST_URL_ENCODED;
		post.data.urlenc = post_urlenc;
	}

	if (parent != NULL && content_get_type(parent) == CONTENT_HTML) {
		child.charset = html_get_encoding(parent);
		child.quirks = content_get_quirks(parent);
	}

	error = nsurl_create(url, &nsurl);
	if (error != NSERROR_OK) {
		return;
	}

	if (referer != NULL) {
		error = nsurl_create(referer, &nsref);
		if (error != NSERROR_OK) {
			nsurl_unref(nsurl);
			return;
		}
	}

	/* Get download out of the way */
	if (download) {
		llcache_handle *l;
		struct browser_window *root;

		root = browser_window_get_root(bw);
		assert(root != NULL);

		fetch_flags |= LLCACHE_RETRIEVE_FORCE_FETCH;
		fetch_flags |= LLCACHE_RETRIEVE_STREAM_DATA;

		error = llcache_handle_retrieve(nsurl, fetch_flags, nsref, 
				fetch_is_post ? &post : NULL,
				NULL, NULL, &l);
		if (error == NSERROR_NO_FETCH_HANDLER) {
			gui_launch_url(nsurl_access(nsurl));
		} else if (error != NSERROR_OK) {
			LOG(("Failed to fetch download: %d", error));
		} else {
			error = download_context_create(l, root->window);
			if (error != NSERROR_OK) {
				LOG(("Failed creating download context: %d", 
						error));
				llcache_handle_abort(l);
				llcache_handle_release(l);
			}
		}

		nsurl_unref(nsurl);
		if (nsref != NULL)
			nsurl_unref(nsref);

		return;
	}

	if (bw->frag_id != NULL)
		lwc_string_unref(bw->frag_id);
	bw->frag_id = NULL;

	if (nsurl_has_component(nsurl, NSURL_FRAGMENT)) {
		bool same_url = false;

		bw->frag_id = nsurl_get_component(nsurl, NSURL_FRAGMENT);

		/* Compare new URL with existing one (ignoring fragments) */
		if (bw->current_content != NULL && 
				hlcache_handle_get_url(bw->current_content) != NULL) {
			same_url = nsurl_compare(nsurl,
					hlcache_handle_get_url(bw->current_content),
					NSURL_COMPLETE);
		}

		/* if we're simply moving to another ID on the same page,
		 * don't bother to fetch, just update the window.
		 */
		if (same_url && fetch_is_post == false && 
				nsurl_has_component(nsurl, NSURL_QUERY) ==
						false) {
			nsurl_unref(nsurl);
			if (nsref != NULL)
				nsurl_unref(nsref);
			if (add_to_history)
				history_add(bw->history, bw->current_content,
						bw->frag_id == NULL ? NULL :
						lwc_string_data(bw->frag_id));
			browser_window_update(bw, false);
			if (bw->current_content != NULL) {
				browser_window_refresh_url_bar(bw,
					hlcache_handle_get_url(bw->current_content),
					bw->frag_id);
			}
			return;
		}
	}

	browser_window_stop(bw);
	browser_window_remove_caret(bw);
	browser_window_destroy_children(bw);

	LOG(("Loading '%s'", nsurl_access(nsurl)));

	LOG(("Set status loading"));
	browser_window_set_status(bw, messages_get("Loading"));
	bw->history_add = add_to_history;

	/* Verifiable fetches may trigger a download */
	if (verifiable)
		fetch_flags |= HLCACHE_RETRIEVE_MAY_DOWNLOAD;


	LOG(("hlcache retr"));
	error = hlcache_handle_retrieve(nsurl,
			fetch_flags | HLCACHE_RETRIEVE_SNIFF_TYPE,
			nsref,
			fetch_is_post ? &post : NULL,
			browser_window_callback, bw,
			parent != NULL ? &child : NULL,
			CONTENT_ANY, &c);

	switch (error) {
	case NSERROR_NO_FETCH_HANDLER: /* no handler for this type */
		LOG(("no fetcher"));
		gui_launch_url(nsurl_access(nsurl));
		nsurl_unref(nsurl);
		if (nsref != NULL)
			nsurl_unref(nsref);
		break;

	case NSERROR_OK:
		LOG(("generic error"));
		bw->loading_content = c;
		browser_window_start_throbber(bw);
		browser_window_refresh_url_bar(bw, nsurl, NULL);

		nsurl_unref(nsurl);
		if (nsref != NULL)
			nsurl_unref(nsref);
		break;

	default: /* assume out of memory */
		LOG(("SUKAAA don't know why"));
		/* TODO: fix all fetcher errors being reported as OOM? */
		nsurl_unref(nsurl);
		if (nsref != NULL)
			nsurl_unref(nsref);
		browser_window_set_status(bw, messages_get("NoMemory"));
		warn_user("NoMemory", 0);

	}

	LOG(("record time"));
	/* Record time */
	bw->last_action = wallclock();
	
	LOG(("That's fine"));
}



/**
 * Callback for fetchcache() for browser window favicon fetches.
 */

static nserror browser_window_favicon_callback(hlcache_handle *c,
		const hlcache_event *event, void *pw)
{
	struct browser_window *bw = pw;

	switch (event->type) {
	case CONTENT_MSG_DONE:
		if (bw->current_favicon != NULL) {
			content_status status = 
					content_get_status(bw->current_favicon);

			if ((status == CONTENT_STATUS_READY) ||
					(status == CONTENT_STATUS_DONE))
				content_close(bw->current_favicon);

			hlcache_handle_release(bw->current_favicon);
		}

		bw->current_favicon = c;
		bw->loading_favicon = NULL;

		/* content_get_bitmap on the hlcache_handle should give 
		 *   us the favicon bitmap at this point
		 */
		gui_window_set_icon(bw->window, c);
		break;

	case CONTENT_MSG_ERROR:

		/* clean up after ourselves */
		if (c == bw->loading_favicon)
			bw->loading_favicon = NULL;
		else if (c == bw->current_favicon) {
			bw->current_favicon = NULL;
		}

		hlcache_handle_release(c);

		if (bw->failed_favicon == false) {
			nsurl *nsref = NULL;
			nsurl *nsurl;
			nserror error;

			bw->failed_favicon = true;

			error = nsurl_create("resource:favicon.ico", &nsurl);
			if (error != NSERROR_OK) {
				LOG(("Unable to create default location url"));
			} else {

				hlcache_handle_retrieve(nsurl,
						HLCACHE_RETRIEVE_SNIFF_TYPE, 
						nsref, NULL,
						browser_window_favicon_callback,
						bw, NULL, CONTENT_IMAGE, 
						&bw->loading_favicon);

				nsurl_unref(nsurl);
			}

		}
		break;

	default:
		break;
	}
	return NSERROR_OK;
}

static void browser_window_update_favicon(hlcache_handle *c,
		struct browser_window *bw, struct content_rfc5988_link *link)
{
	lwc_string *icon_str;
	nsurl *nsref = NULL;
	nsurl *nsurl;
	nserror error;

	assert(c != NULL);
	assert(bw !=NULL);

	if (bw->window == NULL)
		/* Not top-level browser window; not interested */
		return;
	
	/* already fetching the favicon - use that */
	if (bw->loading_favicon != NULL) 
		return;

	bw->failed_favicon = false;

	if (link == NULL) {
		/* look for favicon metadata link */
		if (lwc_intern_string("icon", SLEN("icon"), 
				&icon_str) == lwc_error_ok) {
			link = content_find_rfc5988_link(c, icon_str);
			lwc_string_unref(icon_str);
		}
	}

	if (link == NULL) {
		if (lwc_intern_string("shortcut icon", SLEN("shortcut icon"), 
				&icon_str) == lwc_error_ok) {
			link = content_find_rfc5988_link(c, icon_str);
			lwc_string_unref(icon_str);
		}
	}

	if (link == NULL) {
		lwc_string *scheme;
		bool speculative_default = false;

		nsurl = hlcache_handle_get_url(c);

		scheme = nsurl_get_component(nsurl, NSURL_SCHEME);

		/* If the document was fetched over http(s), then speculate 
		 * that there's a favicon living at /favicon.ico */
		if ((lwc_string_length(scheme) == SLEN("HTTP") &&
				strcasecmp(lwc_string_data(scheme), 
						"http") == 0) ||
		    (lwc_string_length(scheme) == SLEN("HTTPS") &&
				strcasecmp(lwc_string_data(scheme), 
						"https") == 0)) {
			speculative_default = true;
		}

		lwc_string_unref(scheme);

		if (speculative_default) {
			/* no favicon via link, try for the default location */
			error = nsurl_join(nsurl, "/favicon.ico", &nsurl);
		} else {
			bw->failed_favicon = true;
			error = nsurl_create("resource:favicon.ico", &nsurl);
		}
		if (error != NSERROR_OK) {
			LOG(("Unable to create default location url"));
			return;
		}
	} else {
		nsurl = nsurl_ref(link->href);
	}

	if (link == NULL) {
		LOG(("fetching general favicon from '%s'", 
		     nsurl_access(nsurl)));
	} else {
		LOG(("fetching favicon rel:%s '%s'",
				lwc_string_data(link->rel),
				nsurl_access(nsurl)));
	}

	hlcache_handle_retrieve(nsurl, HLCACHE_RETRIEVE_SNIFF_TYPE, 
			nsref, NULL, browser_window_favicon_callback, 
			bw, NULL, CONTENT_IMAGE, &bw->loading_favicon);

	nsurl_unref(nsurl);
}

/** window callback errorcode handling */
static void 
browser_window_callback_errorcode(hlcache_handle *c,
				  struct browser_window *bw, 
				  nserror code)
{
	const char* message;

	message = messages_get_errorcode(code);

	browser_window_set_status(bw, message);

	/* Only warn the user about errors in top-level windows */
	if (bw->browser_window_type == BROWSER_WINDOW_NORMAL) {
		warn_user(message, 0);
	}

	if (c == bw->loading_content) {
		bw->loading_content = NULL;
	} else if (c == bw->current_content) {
		bw->current_content = NULL;
		browser_window_remove_caret(bw);
	}

	hlcache_handle_release(c);

	browser_window_stop_throbber(bw);
}

/**
 * Callback for fetchcache() for browser window fetches.
 */

nserror browser_window_callback(hlcache_handle *c,
		const hlcache_event *event, void *pw)
{
	struct browser_window *bw = pw;

	switch (event->type) {
	case CONTENT_MSG_DOWNLOAD:
		assert(bw->loading_content == c);

		browser_window_convert_to_download(bw, event->data.download);

		if (bw->current_content != NULL) {
			browser_window_refresh_url_bar(bw,
					hlcache_handle_get_url(bw->current_content),
					bw->frag_id);
		}
		break;

	case CONTENT_MSG_LOADING:
		assert(bw->loading_content == c);

#ifdef WITH_THEME_INSTALL
		if (content_get_type(c) == CONTENT_THEME) {
			theme_install_start(c);
			bw->loading_content = NULL;
			browser_window_stop_throbber(bw);
		} else
#endif
		{
			bw->refresh_interval = -1;
			browser_window_set_status(bw, 
					content_get_status_message(c));
		}
		break;

	case CONTENT_MSG_READY:
	{
		int width, height;

		assert(bw->loading_content == c);

		if (bw->current_content != NULL) {
			content_status status = 
					content_get_status(bw->current_content);

			if (status == CONTENT_STATUS_READY ||
					status == CONTENT_STATUS_DONE)
				content_close(bw->current_content);

			hlcache_handle_release(bw->current_content);
		}

		bw->current_content = c;
		bw->loading_content = NULL;

		/* Format the new content to the correct dimensions */
		browser_window_get_dimensions(bw, &width, &height, true);
		content_reformat(c, false, width, height);

		browser_window_remove_caret(bw);

		if (bw->window)
			gui_window_new_content(bw->window);

		browser_window_refresh_url_bar(bw,
				hlcache_handle_get_url(bw->current_content),
				bw->frag_id);

		/* new content; set scroll_to_top */
		browser_window_update(bw, true);
		content_open(c, bw, 0, 0);
		browser_window_set_status(bw, content_get_status_message(c));

		/* history */
		if (bw->history_add && bw->history) {
			nsurl *url = hlcache_handle_get_url(c);

			history_add(bw->history, c, bw->frag_id == NULL ? NULL :
					lwc_string_data(bw->frag_id));
			if (urldb_add_url(url)) {
				urldb_set_url_title(url, content_get_title(c));
				urldb_update_url_visit_data(url);
				urldb_set_url_content_type(url, 
						content_get_type(c));
				/* This is safe as we've just added the URL */
				global_history_add(urldb_get_url(url));
			}
		}

		if (bw->window != NULL) {
			browser_window_set_selection(bw,
					content_get_selection(c));
		}

		/* frames */
		if (content_get_type(c) == CONTENT_HTML && 
				html_get_frameset(c) != NULL)
			browser_window_create_frameset(bw, 
					html_get_frameset(c));
		if (content_get_type(c) == CONTENT_HTML && 
				html_get_iframe(c) != NULL)
			browser_window_create_iframes(bw, html_get_iframe(c));
	}
		break;

	case CONTENT_MSG_DONE:
		assert(bw->current_content == c);

		if (bw->window == NULL) {
			/* Updated browser window's scrollbars.
			 * TODO: do this before CONTENT_MSG_DONE */
			browser_window_reformat(bw, true,
					bw->width, bw->height);
			browser_window_handle_scrollbars(bw);
		}

		browser_window_update(bw, false);
		browser_window_set_status(bw, content_get_status_message(c));
		browser_window_stop_throbber(bw);
		browser_window_update_favicon(c, bw, NULL);

		history_update(bw->history, c);
		hotlist_visited(c);

		if (bw->refresh_interval != -1)
			schedule(bw->refresh_interval,
					browser_window_refresh, bw);
		break;

	case CONTENT_MSG_ERRORCODE:
		browser_window_callback_errorcode(c, bw, event->data.errorcode);
		break;

	case CONTENT_MSG_ERROR:
		browser_window_set_status(bw, event->data.error);

		/* Only warn the user about errors in top-level windows */
		if (bw->browser_window_type == BROWSER_WINDOW_NORMAL)
			warn_user(event->data.error, 0);

		if (c == bw->loading_content)
			bw->loading_content = NULL;
		else if (c == bw->current_content) {
			bw->current_content = NULL;
			browser_window_remove_caret(bw);
		}

		hlcache_handle_release(c);

		browser_window_stop_throbber(bw);
		break;

	case CONTENT_MSG_STATUS:
		if (event->data.explicit_status_text == NULL) {
			/* Object content's status text updated */
			const char *status = NULL;
			if (bw->loading_content != NULL)
				/* Give preference to any loading content */
				status = content_get_status_message(
						bw->loading_content);

			if (status == NULL)
				status = content_get_status_message(c);

			if (status != NULL)
				browser_window_set_status(bw, status);
		} else {
			/* Object content wants to set explicit message */
			browser_window_set_status(bw,
					event->data.explicit_status_text);
		}
		break;

	case CONTENT_MSG_REFORMAT:
		if (c == bw->current_content &&
			content_get_type(c) == CONTENT_HTML) {
			/* reposition frames */
			if (html_get_frameset(c) != NULL)
				browser_window_recalculate_frameset(bw);
			/* reflow iframe positions */
			if (html_get_iframe(c) != NULL)
				browser_window_recalculate_iframes(bw);
		}

		if (bw->move_callback)
			bw->move_callback(bw, bw->caret_p1, bw->caret_p2);

		if (!(event->data.background)) {
			/* Reformatted content should be redrawn */
			browser_window_update(bw, false);
		}
		break;

	case CONTENT_MSG_REDRAW:
	{
		struct rect rect = {
			.x0 = event->data.redraw.x,
			.y0 = event->data.redraw.y,
			.x1 = event->data.redraw.x + event->data.redraw.width,
			.y1 = event->data.redraw.y + event->data.redraw.height
		};

		browser_window_update_box(bw, &rect);
	}
		break;

	case CONTENT_MSG_REFRESH:
		bw->refresh_interval = event->data.delay * 100;
		break;
		
	case CONTENT_MSG_LINK: /* content has an rfc5988 link element */
	{
		lwc_string *icon_str;
		lwc_string *shortcut_icon_str;
		bool icon_match = false;
		bool shortcut_icon_match = false;

		if (lwc_intern_string("icon", SLEN("icon"), 
				&icon_str) == lwc_error_ok) {
			if (lwc_string_caseless_isequal(
					event->data.rfc5988_link->rel, 
					icon_str, &icon_match) != lwc_error_ok) {
				icon_match = false;
			}
			lwc_string_unref(icon_str);
		}

		if (lwc_intern_string("shortcut icon", SLEN("shortcut icon"), 
				&shortcut_icon_str) == lwc_error_ok) {
			if (lwc_string_caseless_isequal(
					event->data.rfc5988_link->rel, 
					shortcut_icon_str, 
					&shortcut_icon_match) != lwc_error_ok) {
				shortcut_icon_match = false;
			}
			lwc_string_unref(shortcut_icon_str);
		}

		if (icon_match || shortcut_icon_match) {
			/* it's a favicon perhaps start a fetch for it */
			browser_window_update_favicon(c, bw,
					event->data.rfc5988_link);
		}
	}
		break;

	case CONTENT_MSG_GETCTX: 
		/* only the content object created by the browser
		 * window requires a new global compartment object 
		 */
		assert(bw->loading_content == c);
		if (js_newcompartment(bw->jsctx,
				      bw,
				      hlcache_handle_get_content(c)) != NULL) {
			*(event->data.jscontext) = bw->jsctx;
		}
		break;

	case CONTENT_MSG_SCROLL:
		/* Content wants to be scrolled */
		if (bw->current_content != c)
			break;

		if (event->data.scroll.area) {
			struct rect rect = {
				.x0 = event->data.scroll.x0,
				.y0 = event->data.scroll.y0,
				.x1 = event->data.scroll.x1,
				.y1 = event->data.scroll.y1
			};
			browser_window_scroll_visible(bw, &rect);
		} else {
			browser_window_set_scroll(bw,
					event->data.scroll.x0,
					event->data.scroll.y0);
		}

		break;

	case CONTENT_MSG_DRAGSAVE:
	{
		/* Content wants drag save of a content */
		struct browser_window *root = browser_window_get_root(bw);
		hlcache_handle *save = event->data.dragsave.content;

		if (save == NULL) {
			save = c;
		}

		switch(event->data.dragsave.type) {
		case CONTENT_SAVE_ORIG:
			gui_drag_save_object(GUI_SAVE_OBJECT_ORIG, save,
					root->window);
			break;
		case CONTENT_SAVE_NATIVE:
			gui_drag_save_object(GUI_SAVE_OBJECT_NATIVE, save,
					root->window);
			break;
		case CONTENT_SAVE_COMPLETE:
			gui_drag_save_object(GUI_SAVE_COMPLETE, save,
					root->window);
			break;
		case CONTENT_SAVE_SOURCE:
			gui_drag_save_object(GUI_SAVE_SOURCE, save,
					root->window);
			break;
		}
	}
		break;

	case CONTENT_MSG_SAVELINK:
	{
		/* Content wants a link to be saved */
		struct browser_window *root = browser_window_get_root(bw);
		gui_window_save_link(root->window,
				event->data.savelink.url,
				event->data.savelink.title);
	}
		break;

	case CONTENT_MSG_POINTER:
		/* Content wants to have specific mouse pointer */
		browser_window_set_pointer(bw, event->data.pointer);
		break;

	default:
		assert(0);
	}

	return NSERROR_OK;
}


/*
 * Get the dimensions of the area a browser window occupies
 *
 * \param  bw      The browser window to get dimensions of
 * \param  width   Updated to the browser window viewport width
 * \param  height  Updated to the browser window viewport height
 * \param  scaled  Whether we want the height with scale applied
 */

void browser_window_get_dimensions(struct browser_window *bw,
		int *width, int *height, bool scaled)
{
	assert(bw);

	if (bw->window == NULL) {
		/* Core managed browser window */
		*width = bw->width;
		*height = bw->height;
	} else {
		/* Front end window */
		gui_window_get_dimensions(bw->window, width, height, scaled);
	}
}


/*
 * Set the dimensions of the area a browser window occupies
 *
 * \param  bw      The browser window to set dimensions of
 * \param  width   Width in pixels
 * \param  height  Height in pixels
 */

void browser_window_set_dimensions(struct browser_window *bw,
		int width, int height)
{
	assert(bw);

	if (bw->window == NULL) {
		/* Core managed browser window */
		bw->width = width;
		bw->height = height;
	} else {
		LOG(("Asked to set dimensions of front end window."));
		assert(0);
	}
}


/**
 * Transfer the loading_content to a new download window.
 */

void browser_window_convert_to_download(struct browser_window *bw,
		llcache_handle *stream)
{
	struct browser_window *root = browser_window_get_root(bw);
	nserror error;

	assert(root != NULL);

	error = download_context_create(stream, root->window);
	if (error != NSERROR_OK) {
		llcache_handle_abort(stream);
		llcache_handle_release(stream);
	}

	/* remove content from browser window */
	hlcache_handle_release(bw->loading_content);
	bw->loading_content = NULL;

	browser_window_stop_throbber(bw);
}


/**
 * Handle meta http-equiv refresh time elapsing by loading a new page.
 *
 * \param  p  browser window to refresh with new page
 */

void browser_window_refresh(void *p)
{
	struct browser_window *bw = p;
	bool history_add = true;
	const char *url;
	const char *refresh;

	assert(bw->current_content != NULL &&
		(content_get_status(bw->current_content) == 
				CONTENT_STATUS_READY ||
		content_get_status(bw->current_content) == 
				CONTENT_STATUS_DONE));

	/* Ignore if the refresh URL has gone
	 * (may happen if a fetch error occurred) */
	refresh = nsurl_access(content_get_refresh_url(bw->current_content));
	if (refresh == NULL)
		return;

	/* mark this content as invalid so it gets flushed from the cache */
	content_invalidate_reuse_data(bw->current_content);

	url = nsurl_access(hlcache_handle_get_url(bw->current_content));
	if (url != NULL && strcmp(url, refresh) == 0)
		history_add = false;

	/* Treat an (almost) immediate refresh in a top-level browser window as
	 * if it were an HTTP redirect, and thus make the resulting fetch
	 * verifiable.
	 *
	 * See fetchcache.c for why redirected fetches should be verifiable at
	 * all.
	 */
	if (bw->refresh_interval <= 100 && bw->parent == NULL) {
		browser_window_go(bw, refresh, url, history_add);
	} else {
		browser_window_go_unverifiable(bw, refresh, url, history_add, 
				bw->current_content);
	}
}


/**
 * Start the busy indicator.
 *
 * \param  bw  browser window
 */

void browser_window_start_throbber(struct browser_window *bw)
{
	bw->throbbing = true;

	while (bw->parent)
		bw = bw->parent;

	gui_window_start_throbber(bw->window);
}


/**
 * Stop the busy indicator.
 *
 * \param  bw  browser window
 */

void browser_window_stop_throbber(struct browser_window *bw)
{
	bw->throbbing = false;

	while (bw->parent)
		bw = bw->parent;

	if (!browser_window_check_throbber(bw))
		gui_window_stop_throbber(bw->window);
}

bool browser_window_check_throbber(struct browser_window *bw)
{
	int children, index;

	if (bw->throbbing)
		return true;

	if (bw->children) {
		children = bw->rows * bw->cols;
		for (index = 0; index < children; index++) {
			if (browser_window_check_throbber(&bw->children[index]))
				return true;
		}
	}
	if (bw->iframes) {
		for (index = 0; index < bw->iframe_count; index++) {
			if (browser_window_check_throbber(&bw->iframes[index]))
				return true;
		}
	}
	return false;
}

/**
 * Redraw browser window, set extent to content, and update title.
 *
 * \param  bw		  browser_window
 * \param  scroll_to_top  move view to top of page
 */

void browser_window_update(struct browser_window *bw, bool scroll_to_top)
{
	int x, y;

	if (bw->current_content == NULL)
		return;

	switch (bw->browser_window_type) {

	case BROWSER_WINDOW_NORMAL:
		/* Root browser window, constituting a front end window/tab */
		gui_window_set_title(bw->window, 
				content_get_title(bw->current_content));

		browser_window_update_extent(bw);

		if (scroll_to_top)
			browser_window_set_scroll(bw, 0, 0);

		/* if frag_id exists, then try to scroll to it */
		/** @todo don't do this if the user has scrolled */
		if (bw->frag_id && html_get_id_offset(bw->current_content,
				bw->frag_id, &x, &y)) {
			browser_window_set_scroll(bw, x, y);
		}

		gui_window_redraw_window(bw->window);

		break;

	case BROWSER_WINDOW_IFRAME:
		/* Internal iframe browser window */
		assert(bw->parent != NULL);
		assert(bw->parent->current_content != NULL);

		browser_window_update_extent(bw);

		if (scroll_to_top)
			browser_window_set_scroll(bw, 0, 0);

		/* if frag_id exists, then try to scroll to it */
		/** @todo don't do this if the user has scrolled */
		if (bw->frag_id && html_get_id_offset(bw->current_content,
				bw->frag_id, &x, &y)) {
			browser_window_set_scroll(bw, x, y);
		}

		html_redraw_a_box(bw->parent->current_content, bw->box);
		break;

	case BROWSER_WINDOW_FRAME:
	{
		struct rect rect;
		browser_window_update_extent(bw);

		if (scroll_to_top)
			browser_window_set_scroll(bw, 0, 0);

		/* if frag_id exists, then try to scroll to it */
		/** @todo don't do this if the user has scrolled */
		if (bw->frag_id && html_get_id_offset(bw->current_content,
				bw->frag_id, &x, &y)) {
			browser_window_set_scroll(bw, x, y);
		}

		rect.x0 = scrollbar_get_offset(bw->scroll_x);
		rect.y0 = scrollbar_get_offset(bw->scroll_y);
		rect.x1 = rect.x0 + bw->width;
		rect.y1 = rect.y0 + bw->height;

		browser_window_update_box(bw, &rect);
	}
		break;

	default:
	case BROWSER_WINDOW_FRAMESET:
		/* Nothing to do */
		break;
	}
}


void browser_window_update_box(struct browser_window *bw, struct rect *rect)
{
	int pos_x;
	int pos_y;
	struct browser_window *top;

	assert(bw);

	if (bw->window != NULL) {
		/* Front end window */
		gui_window_update_box(bw->window, rect);
	} else {
		/* Core managed browser window */
		browser_window_get_position(bw, true, &pos_x, &pos_y);

		top = browser_window_get_root(bw);

		rect->x0 += pos_x / bw->scale;
		rect->y0 += pos_y / bw->scale;
		rect->x1 += pos_x / bw->scale;
		rect->y1 += pos_y / bw->scale;

		gui_window_update_box(top->window, rect);
	}
}


/**
 * Stop all fetching activity in a browser window.
 *
 * \param  bw  browser window
 */

void browser_window_stop(struct browser_window *bw)
{
	int children, index;

	if (bw->loading_content != NULL) {
		hlcache_handle_abort(bw->loading_content);
		hlcache_handle_release(bw->loading_content);
		bw->loading_content = NULL;
	}

	if (bw->current_content != NULL && content_get_status(
			bw->current_content) != CONTENT_STATUS_DONE) {
		nserror error;
		assert(content_get_status(bw->current_content) == 
				CONTENT_STATUS_READY);
		error = hlcache_handle_abort(bw->current_content);
		assert(error == NSERROR_OK);
	}

	schedule_remove(browser_window_refresh, bw);

	if (bw->children) {
		children = bw->rows * bw->cols;
		for (index = 0; index < children; index++)
			browser_window_stop(&bw->children[index]);
	}
	if (bw->iframes) {
		children = bw->iframe_count;
		for (index = 0; index < children; index++)
			browser_window_stop(&bw->iframes[index]);
	}

	if (bw->current_content != NULL) {
		browser_window_refresh_url_bar(bw, 
				hlcache_handle_get_url(bw->current_content),
				bw->frag_id);
	}

	browser_window_stop_throbber(bw);
}


/**
 * Reload the page in a browser window.
 *
 * \param  bw  browser window
 * \param  all whether to reload all objects associated with the page
 */

void browser_window_reload(struct browser_window *bw, bool all)
{
	hlcache_handle *c;
	unsigned int i;

	if (bw->current_content == NULL || bw->loading_content != NULL)
		return;

	if (all && content_get_type(bw->current_content) == CONTENT_HTML) {
		struct html_stylesheet *sheets;
		struct content_html_object *object;
		unsigned int count;

		c = bw->current_content;

		/* invalidate objects */
		object = html_get_objects(c, &count);

		for (; object != NULL; object = object->next) {
			if (object->content != NULL)
				content_invalidate_reuse_data(object->content);
		}

		/* invalidate stylesheets */
		sheets = html_get_stylesheets(c, &count);

		for (i = STYLESHEET_START; i != count; i++) {
			if (sheets[i].type == HTML_STYLESHEET_EXTERNAL &&
					sheets[i].data.external != NULL) {
				content_invalidate_reuse_data(
						sheets[i].data.external);
			}
		}
	}

	content_invalidate_reuse_data(bw->current_content);

	browser_window_go(bw, nsurl_access(
			hlcache_handle_get_url(bw->current_content)), 0, false);
}


/**
 * Change the status bar of a browser window.
 *
 * \param  bw	 browser window
 * \param  text  new status text (copied)
 */

void browser_window_set_status(struct browser_window *bw, const char *text)
{
	int text_len;
	/* find topmost window */
	while (bw->parent)
		bw = bw->parent;

	if ((bw->status_text != NULL) && 
	    (strcmp(text, bw->status_text) == 0)) {
		/* status text is unchanged */
		bw->status_match++;
		return;
	}

	/* status text is changed */
 
	text_len = strlen(text);

	if ((bw->status_text == NULL) || (bw->status_text_len < text_len)) {
		/* no current string allocation or it is not long enough */
		free(bw->status_text);
		bw->status_text = strdup(text);
		bw->status_text_len = text_len;
	} else {
		/* current allocation has enough space */
		memcpy(bw->status_text, text, text_len + 1);
	}

	bw->status_miss++;
	gui_window_set_status(bw->window, bw->status_text);
}


/**
 * Change the shape of the mouse pointer
 *
 * \param bw Browser window to set shape in
 * \param shape The pointer shape to use
 */

void browser_window_set_pointer(struct browser_window *bw,
		browser_pointer_shape shape)
{
	struct browser_window *root = browser_window_get_root(bw);
	gui_pointer_shape gui_shape;
	bool loading;

	assert(root);
	assert(root->window);

	loading = (bw->loading_content != NULL || (bw->current_content &&
			content_get_status(bw->current_content) ==
			CONTENT_STATUS_READY));

	if (wallclock() - bw->last_action < 100 && loading) {
		/* If loading and less than 1 second since last link followed,
		 * force progress indicator pointer */
		gui_shape = GUI_POINTER_PROGRESS;

	} else if (shape == BROWSER_POINTER_AUTO) {
		/* Up to browser window to decide */
		if (loading)
			gui_shape = GUI_POINTER_PROGRESS;
		else
			gui_shape = GUI_POINTER_DEFAULT;

	} else {
		/* Use what we were told */
		gui_shape = (gui_pointer_shape)shape;
	}

	gui_window_set_pointer(root->window, gui_shape);
}


/**
 * Close and destroy a browser window.
 *
 * \param  bw  browser window
 */

void browser_window_destroy(struct browser_window *bw)
{
	/* can't destoy child windows on their own */
	assert(!bw->parent);

	/* destroy */
	browser_window_destroy_internal(bw);
	free(bw);
}


/**
 * Close and destroy all child browser window.
 *
 * \param  bw  browser window
 */

void browser_window_destroy_children(struct browser_window *bw)
{
	int i;

	if (bw->children) {
		for (i = 0; i < (bw->rows * bw->cols); i++)
			browser_window_destroy_internal(&bw->children[i]);
		free(bw->children);
		bw->children = NULL;
		bw->rows = 0;
		bw->cols = 0;
	}
	if (bw->iframes) {
		for (i = 0; i < bw->iframe_count; i++)
			browser_window_destroy_internal(&bw->iframes[i]);
		free(bw->iframes);
		bw->iframes = NULL;
		bw->iframe_count = 0;
	}
}


/**
 * Release all memory associated with a browser window.
 *
 * \param  bw  browser window
 */

void browser_window_destroy_internal(struct browser_window *bw)
{
	assert(bw);

	LOG(("Destroying window"));

	if (bw->children != NULL || bw->iframes != NULL)
		browser_window_destroy_children(bw);

	schedule_remove(browser_window_refresh, bw);

	/* If this brower window is not the root window, and has focus, unset
	 * the root browser window's focus pointer. */
	if (!bw->window) {
		struct browser_window *top = browser_window_get_root(bw);

		if (top->focus == bw)
			top->focus = top;

		if (bw->current_content != NULL &&
				top->cur_sel == content_get_selection(
						bw->current_content)) {
			browser_window_set_selection(top, NULL);
		}
	}

	/* Destroying a search context causes it to redraw any deselected,
	 * content areas, so do this first */
	browser_window_search_destroy_context(bw);

	/* Destruction order is important: we must ensure that the frontend 
	 * destroys any window(s) associated with this browser window before 
	 * we attempt any destructive cleanup. 
	 */

	if (bw->window) {
		/* Only the root window has a GUI window */
		gui_window_destroy(bw->window);
	}

	if (bw->loading_content != NULL) {
		hlcache_handle_abort(bw->loading_content);
		hlcache_handle_release(bw->loading_content);
		bw->loading_content = NULL;
	}

	if (bw->current_content != NULL) {
		content_status status = content_get_status(bw->current_content);
		if (status == CONTENT_STATUS_READY || 
				status == CONTENT_STATUS_DONE)
			content_close(bw->current_content);

		hlcache_handle_release(bw->current_content);
		bw->current_content = NULL;
	}

	if (bw->loading_favicon != NULL) {
		hlcache_handle_abort(bw->loading_favicon);
		hlcache_handle_release(bw->loading_favicon);
		bw->loading_favicon = NULL;
	}

	if (bw->current_favicon != NULL) {
		content_status status = content_get_status(bw->current_favicon);

		if (status == CONTENT_STATUS_READY ||
		    status == CONTENT_STATUS_DONE)
			content_close(bw->current_favicon);

		hlcache_handle_release(bw->current_favicon);
		bw->current_favicon = NULL;
	}

	if (bw->box != NULL) {
		bw->box->iframe = NULL;
		bw->box = NULL;
	}

	if (bw->jsctx != NULL) {
		js_destroycontext(bw->jsctx);
	}

	/* These simply free memory, so are safe here */

	if (bw->frag_id != NULL)
		lwc_string_unref(bw->frag_id);

	history_destroy(bw->history);

	free(bw->name);
	free(bw->status_text);
	bw->status_text = NULL;
	LOG(("Status text cache match:miss %d:%d",
			bw->status_match, bw->status_miss));
}


/**
 * Reformat a browser window contents to a new width or height.
 *
 * \param  bw      the browser window to reformat
 * \param  width   new width
 * \param  height  new height
 */

void browser_window_reformat(struct browser_window *bw, bool background,
		int width, int height)
{
	hlcache_handle *c = bw->current_content;

	if (c == NULL)
		return;

	if (bw->browser_window_type != BROWSER_WINDOW_IFRAME) {
		/* Iframe dimensions are already scaled in parent's layout */
		width  /= bw->scale;
		height /= bw->scale;
	}

	if (bw->window == NULL) {
		/* Core managed browser window; subtract scrollbar width */
		width  -= bw->scroll_y ? SCROLLBAR_WIDTH : 0;
		height -= bw->scroll_x ? SCROLLBAR_WIDTH : 0;

		width  = width  > 0 ? width  : 0;
		height = height > 0 ? height : 0;
	}

	content_reformat(c, background, width, height);
}


/**
 * Sets the scale of a browser window
 *
 * \param bw	The browser window to scale
 * \param scale	The new scale
 * \param all	Scale all windows in the tree (ie work up aswell as down)
 */

void browser_window_set_scale(struct browser_window *bw, float scale, bool all)
{
	while (bw->parent && all)
		bw = bw->parent;

	browser_window_set_scale_internal(bw, scale);

	if (bw->parent)
		bw = bw->parent;

	browser_window_recalculate_frameset(bw);
}

void browser_window_set_scale_internal(struct browser_window *bw, float scale)
{
	int i;
	hlcache_handle *c;

	if (fabs(bw->scale-scale) < 0.0001)
		return;

	bw->scale = scale;
	c = bw->current_content;

	if (c != NULL) {
		if (content_can_reformat(c) == false) {
			browser_window_update(bw, false);
		} else {
			bw->reformat_pending = true;
			browser_reformat_pending = true;
		}
	}

	for (i = 0; i < (bw->cols * bw->rows); i++)
		browser_window_set_scale_internal(&bw->children[i], scale);
	for (i = 0; i < bw->iframe_count; i++)
		browser_window_set_scale_internal(&bw->iframes[i], scale);
}


/**
 * Gets the scale of a browser window
 *
 * \param bw	The browser window to scale
 * \return 
 */

float browser_window_get_scale(struct browser_window *bw)
{
	return bw->scale;
}


/**
 * Update URL bar for a given browser window to given URL
 *
 * \param bw	Browser window to update URL bar for.
 * \param url	URL for content displayed by bw, excluding any fragment.
 * \param frag	Additional fragment. May be NULL if none.
 */

void browser_window_refresh_url_bar(struct browser_window *bw, nsurl *url,
		lwc_string *frag)
{
	assert(bw);
	assert(url);

	if (bw->parent != NULL) {
		/* Not root window; don't set a URL in GUI URL bar */
		return;
	}

	if (frag == NULL) {
		/* With no fragment, we may as well pass url straight through
		 * saving a malloc, copy, free cycle.
		 */
		gui_window_set_url(bw->window, nsurl_access(url));
	} else {
		nsurl *display_url;
		nserror error;

		error = nsurl_refragment(url, frag, &display_url);
		if (error != NSERROR_OK) {
			warn_user("NoMemory", 0);
			return;
		}

		gui_window_set_url(bw->window, nsurl_access(display_url));
		nsurl_unref(display_url);
	}
}

/**
 * Locate a browser window in the specified stack according.
 *
 * \param bw  the browser_window to search all relatives of
 * \param target  the target to locate
 * \param mouse The current mouse state
 * \return The browser window the mouse is in
 */

struct browser_window *browser_window_find_target(struct browser_window *bw,
		const char *target, browser_mouse_state mouse)
{
	struct browser_window *bw_target;
	struct browser_window *top;
	hlcache_handle *c;
	int rdepth;

	/* use the base target if we don't have one */
	c = bw->current_content;
	if (target == NULL && c != NULL && content_get_type(c) == CONTENT_HTML)
		target = html_get_base_target(c);
	if (target == NULL)
		target = TARGET_SELF;

	/* allow the simple case of target="_blank" to be ignored if requested
	 */
	if ((!(mouse & BROWSER_MOUSE_CLICK_2)) &&
			(!((mouse & BROWSER_MOUSE_CLICK_2) &&
			(mouse & BROWSER_MOUSE_MOD_2))) &&
	    (!nsoption_bool(target_blank))) {
		/* not a mouse button 2 click
		 * not a mouse button 1 click with ctrl pressed
		 * configured to ignore target="_blank" */
		if ((target == TARGET_BLANK) || (!strcasecmp(target, "_blank")))
			return bw;
	}

	/* handle reserved keywords */
	if (((nsoption_bool(button_2_tab)) && 
	     (mouse & BROWSER_MOUSE_CLICK_2))||
	    ((!nsoption_bool(button_2_tab)) &&
	     ((mouse & BROWSER_MOUSE_CLICK_1) &&
	      (mouse & BROWSER_MOUSE_MOD_2))) ||
	    ((nsoption_bool(button_2_tab)) && 
	     ((target == TARGET_BLANK) ||
	      (!strcasecmp(target, "_blank"))))) {
		/* open in new tab if:
		 * - button_2 opens in new tab and button_2 was pressed
		 * OR
		 * - button_2 doesn't open in new tabs and button_1 was
		 *   pressed with ctrl held
		 * OR
		 * - button_2 opens in new tab and the link target is "_blank"
		 */
		bw_target = browser_window_create(NULL, bw, NULL, false, true);
		if (!bw_target)
			return bw;
		return bw_target;
	} else if (((!nsoption_bool(button_2_tab)) &&
		    (mouse & BROWSER_MOUSE_CLICK_2)) ||
		   ((nsoption_bool(button_2_tab)) &&
		    ((mouse & BROWSER_MOUSE_CLICK_1) &&
		     (mouse & BROWSER_MOUSE_MOD_2))) ||
		   ((!nsoption_bool(button_2_tab)) && 
		    ((target == TARGET_BLANK) ||
		     (!strcasecmp(target, "_blank"))))) {
		/* open in new window if:
		 * - button_2 doesn't open in new tabs and button_2 was pressed
		 * OR
		 * - button_2 opens in new tab and button_1 was pressed with
		 *   ctrl held
		 * OR
		 * - button_2 doesn't open in new tabs and the link target is
		 *   "_blank"
		 */
		bw_target = browser_window_create(NULL, bw, NULL, false, false);
		if (!bw_target)
			return bw;
		return bw_target;
	} else if ((target == TARGET_SELF) || (!strcasecmp(target, "_self"))) {
		return bw;
	} else if ((target == TARGET_PARENT) ||
			(!strcasecmp(target, "_parent"))) {
		if (bw->parent)
			return bw->parent;
		return bw;
	} else if ((target == TARGET_TOP) || (!strcasecmp(target, "_top"))) {
		while (bw->parent)
			bw = bw->parent;
		return bw;
	}

	/* find frame according to B.8, ie using the following priorities:
	 *
	 *  1) current frame
	 *  2) closest to front
	 */
	rdepth = -1;
	bw_target = NULL;
	for (top = bw; top->parent; top = top->parent);
	browser_window_find_target_internal(top, target, 0, bw, &rdepth,
			&bw_target);
	if (bw_target)
		return bw_target;

	/* we require a new window using the target name */
	if (!nsoption_bool(target_blank))
		return bw;

	bw_target = browser_window_create(NULL, bw, NULL, false, false);
	if (!bw_target)
		return bw;

	/* frame names should begin with an alphabetic character (a-z,A-Z),
	 * however in practice you get things such as '_new' and '2left'. The
	 * only real effect this has is when giving out names as it can be
	 * assumed that an author intended '_new' to create a new nameless
	 * window (ie '_blank') whereas in the case of '2left' the intention
	 * was for a new named window. As such we merely special case windows
	 * that begin with an underscore. */
	if (target[0] != '_') {
		bw_target->name = strdup(target);
		if (!bw_target->name)
			warn_user("NoMemory", 0);
	}
	return bw_target;
}

void browser_window_find_target_internal(struct browser_window *bw,
		const char *target, int depth, struct browser_window *page,
		int *rdepth, struct browser_window **bw_target)
{
	int i;

	if ((bw->name) && (!strcasecmp(bw->name, target))) {
		if ((bw == page) || (depth > *rdepth)) {
			*rdepth = depth;
			*bw_target = bw;
		}
	}

	if ((!bw->children) && (!bw->iframes))
		return;

	depth++;

	if (bw->children != NULL) {
		for (i = 0; i < (bw->cols * bw->rows); i++) {
			if ((bw->children[i].name) &&
					(!strcasecmp(bw->children[i].name, 
					target))) {
				if ((page == &bw->children[i]) || 
						(depth > *rdepth)) {
					*rdepth = depth;
					*bw_target = &bw->children[i];
				}
			}
			if (bw->children[i].children)
				browser_window_find_target_internal(
						&bw->children[i],
						target, depth, page, 
						rdepth, bw_target);
		}
	}

	if (bw->iframes != NULL) {
		for (i = 0; i < bw->iframe_count; i++)
			browser_window_find_target_internal(&bw->iframes[i], 
					target, depth, page, rdepth, bw_target);
	}
}


/**
 * Handle non-click mouse action in a browser window. (drag ends, movements)
 *
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 */

void browser_window_mouse_track(struct browser_window *bw,
		browser_mouse_state mouse, int x, int y)
{
	hlcache_handle *c = bw->current_content;
	const char *status = NULL;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;

	if (bw->window != NULL && bw->drag_window && bw != bw->drag_window) {
		/* This is the root browser window and there's an active drag
		 * in a sub window.
		 * Pass the mouse action straight on to that bw. */
		struct browser_window *drag_bw = bw->drag_window;
		int off_x = 0;
		int off_y = 0;

		browser_window_get_position(drag_bw, true, &off_x, &off_y);

		if (drag_bw->browser_window_type == BROWSER_WINDOW_FRAME) {
			browser_window_mouse_track(drag_bw, mouse,
					x - off_x, y - off_y);

		} else if (drag_bw->browser_window_type ==
				BROWSER_WINDOW_IFRAME) {
			browser_window_mouse_track(drag_bw, mouse,
					x - off_x / bw->scale,
					y - off_y / bw->scale);
		}
		return;
	}

	if (bw->children) {
		/* Browser window has children (frames) */
		struct browser_window *child;
		int cur_child;
		int children = bw->rows * bw->cols;

		for (cur_child = 0; cur_child < children; cur_child++) {

			child = &bw->children[cur_child];

			if (x < child->x || y < child->y ||
					child->x + child->width < x ||
					child->y + child->height < y) {
				/* Click not in this child */
				continue;
			}

			/* It's this child that contains the mouse; pass
			 * mouse action on to child */
			browser_window_mouse_track(child, mouse,
					x - child->x + scrollbar_get_offset(
							child->scroll_x),
					y - child->y + scrollbar_get_offset(
							child->scroll_y));

			/* Mouse action was for this child, we're done */
			return;
		}

		/* Odd if we reached here, but nothing else can use the click
		 * when there are children. */
		return;
	}

	if (c == NULL && bw->drag_type != DRAGGING_FRAME)
		return;

	if (bw->drag_type != DRAGGING_NONE && !mouse) {
		browser_window_mouse_drag_end(bw, mouse, x, y);
	}

	/* Browser window's horizontal scrollbar */
	if (bw->scroll_x != NULL && bw->drag_type != DRAGGING_SCR_Y) {
		int scr_x, scr_y;
		browser_window_get_scrollbar_pos(bw, true, &scr_x, &scr_y);
		scr_x = x - scr_x - scrollbar_get_offset(bw->scroll_x);
		scr_y = y - scr_y - scrollbar_get_offset(bw->scroll_y);

		if ((scr_x > 0 && scr_x < browser_window_get_scrollbar_len(bw,
						true) &&
				scr_y > 0 && scr_y < SCROLLBAR_WIDTH &&
				bw->drag_type == DRAGGING_NONE) ||
				bw->drag_type == DRAGGING_SCR_X) {
			/* Start a scrollbar drag, or continue existing drag */
			status = scrollbar_mouse_action(bw->scroll_x, mouse,
					scr_x, scr_y);
			pointer = BROWSER_POINTER_DEFAULT;

			if (status != NULL)
				browser_window_set_status(bw, status);

			browser_window_set_pointer(bw, pointer);
			return;
		}
	}

	/* Browser window's vertical scrollbar */
	if (bw->scroll_y != NULL) {
		int scr_x, scr_y;
		browser_window_get_scrollbar_pos(bw, false, &scr_x, &scr_y);
		scr_x = x - scr_x - scrollbar_get_offset(bw->scroll_x);
		scr_y = y - scr_y - scrollbar_get_offset(bw->scroll_y);

		if ((scr_y > 0 && scr_y < browser_window_get_scrollbar_len(bw,
						false) &&
				scr_x > 0 && scr_x < SCROLLBAR_WIDTH &&
				bw->drag_type == DRAGGING_NONE) ||
				bw->drag_type == DRAGGING_SCR_Y) {
			/* Start a scrollbar drag, or continue existing drag */
			status = scrollbar_mouse_action(bw->scroll_y, mouse,
					scr_x, scr_y);
			pointer = BROWSER_POINTER_DEFAULT;

			if (status != NULL)
				browser_window_set_status(bw, status);

			browser_window_set_pointer(bw, pointer);
			return;
		}
	}

	if (bw->drag_type == DRAGGING_FRAME) {
		browser_window_resize_frame(bw, bw->x + x, bw->y + y);
	} else if (bw->drag_type == DRAGGING_PAGE_SCROLL) {
		/* mouse movement since drag started */
		int scrollx = bw->drag_start_x - x;
		int scrolly = bw->drag_start_y - y;

		/* new scroll offsets */
		scrollx += bw->drag_start_scroll_x;
		scrolly += bw->drag_start_scroll_y;

		bw->drag_start_scroll_x = scrollx;
		bw->drag_start_scroll_y = scrolly;

		browser_window_set_scroll(bw, scrollx, scrolly);
	} else {
		assert(c != NULL);
		content_mouse_track(c, bw, mouse, x, y);
	}
}


/**
 * Handle mouse clicks in a browser window.
 *
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 */

void browser_window_mouse_click(struct browser_window *bw,
		browser_mouse_state mouse, int x, int y)
{
	hlcache_handle *c = bw->current_content;
	const char *status = NULL;
	browser_pointer_shape pointer = BROWSER_POINTER_DEFAULT;

	if (bw->children) {
		/* Browser window has children (frames) */
		struct browser_window *child;
		int cur_child;
		int children = bw->rows * bw->cols;

		for (cur_child = 0; cur_child < children; cur_child++) {

			child = &bw->children[cur_child];

			if (x < child->x || y < child->y ||
					child->x + child->width < x ||
					child->y + child->height < y) {
				/* Click not in this child */
				continue;
			}

			/* It's this child that contains the click; pass it
			 * on to child. */
			browser_window_mouse_click(child, mouse,
					x - child->x + scrollbar_get_offset(
							child->scroll_x),
					y - child->y + scrollbar_get_offset(
							child->scroll_y));

			/* Mouse action was for this child, we're done */
			return;
		}

		return;
	}

	if (!c)
		return;

	if (bw->scroll_x != NULL) {
		int scr_x, scr_y;
		browser_window_get_scrollbar_pos(bw, true, &scr_x, &scr_y);
		scr_x = x - scr_x - scrollbar_get_offset(bw->scroll_x);
		scr_y = y - scr_y - scrollbar_get_offset(bw->scroll_y);

		if (scr_x > 0 && scr_x < browser_window_get_scrollbar_len(bw,
						true) &&
				scr_y > 0 && scr_y < SCROLLBAR_WIDTH) {
			status = scrollbar_mouse_action(bw->scroll_x, mouse,
					scr_x, scr_y);
			pointer = BROWSER_POINTER_DEFAULT;

			if (status != NULL)
				browser_window_set_status(bw, status);

			browser_window_set_pointer(bw, pointer);
			return;
		}
	}

	if (bw->scroll_y != NULL) {
		int scr_x, scr_y;
		browser_window_get_scrollbar_pos(bw, false, &scr_x, &scr_y);
		scr_x = x - scr_x - scrollbar_get_offset(bw->scroll_x);
		scr_y = y - scr_y - scrollbar_get_offset(bw->scroll_y);

		if (scr_y > 0 && scr_y < browser_window_get_scrollbar_len(bw,
						false) &&
				scr_x > 0 && scr_x < SCROLLBAR_WIDTH) {
			status = scrollbar_mouse_action(bw->scroll_y, mouse,
					scr_x, scr_y);
			pointer = BROWSER_POINTER_DEFAULT;

			if (status != NULL)
				browser_window_set_status(bw, status);

			browser_window_set_pointer(bw, pointer);
			return;
		}
	}

	switch (content_get_type(c)) {
	case CONTENT_HTML:
	case CONTENT_TEXTPLAIN:
		content_mouse_action(c, bw, mouse, x, y);
		break;
	default:
		if (mouse & BROWSER_MOUSE_MOD_2) {
			if (mouse & BROWSER_MOUSE_DRAG_2)
				gui_drag_save_object(GUI_SAVE_OBJECT_NATIVE, c,
						bw->window);
			else if (mouse & BROWSER_MOUSE_DRAG_1)
				gui_drag_save_object(GUI_SAVE_OBJECT_ORIG, c,
						bw->window);
		}
		else if (mouse & (BROWSER_MOUSE_DRAG_1 |
				BROWSER_MOUSE_DRAG_2)) {
			browser_window_page_drag_start(bw, x, y);
			browser_window_set_pointer(bw, BROWSER_POINTER_MOVE);
		}
		break;
	}
}


/**
 * Handles the end of a drag operation in a browser window.
 *
 * \param  bw	  browser window
 * \param  mouse  state of mouse buttons and modifier keys
 * \param  x	  coordinate of mouse
 * \param  y	  coordinate of mouse
 *
 * TODO: Remove this function, once these things are associated with content,
 *       rather than bw.
 */

void browser_window_mouse_drag_end(struct browser_window *bw,
		browser_mouse_state mouse, int x, int y)
{
	int scr_x, scr_y;

	switch (bw->drag_type) {
	case DRAGGING_SELECTION:
	case DRAGGING_OTHER:
	case DRAGGING_CONTENT_SCROLLBAR:
		/* Drag handled by content handler */
		break;

	case DRAGGING_SCR_X:

		browser_window_get_scrollbar_pos(bw, true, &scr_x, &scr_y);

		scr_x = x - scr_x - scrollbar_get_offset(bw->scroll_x);
		scr_y = y - scr_y - scrollbar_get_offset(bw->scroll_y);

		scrollbar_mouse_drag_end(bw->scroll_x, mouse, scr_x, scr_y);

		bw->drag_type = DRAGGING_NONE;
		break;

	case DRAGGING_SCR_Y:

		browser_window_get_scrollbar_pos(bw, false, &scr_x, &scr_y);

		scr_x = x - scr_x - scrollbar_get_offset(bw->scroll_x);
		scr_y = y - scr_y - scrollbar_get_offset(bw->scroll_y);

		scrollbar_mouse_drag_end(bw->scroll_y, mouse, scr_x, scr_y);

		bw->drag_type = DRAGGING_NONE;
		break;

	default:
		browser_window_set_drag_type(bw, DRAGGING_NONE, NULL);
		break;
	}
}


/**
 * Redraw a rectangular region of a browser window
 *
 * \param  bw	  browser window to be redrawn
 * \param  x	  x co-ord of top-left
 * \param  y	  y co-ord of top-left
 * \param  width  width of rectangle
 * \param  height height of rectangle
 */

void browser_window_redraw_rect(struct browser_window *bw, int x, int y,
		int width, int height)
{
	content_request_redraw(bw->current_content, x, y, width, height);
}


/**
 * Start drag scrolling the contents of the browser window
 *
 * \param bw  browser window
 * \param x   x ordinate of initial mouse position
 * \param y   y ordinate
 */

void browser_window_page_drag_start(struct browser_window *bw, int x, int y)
{
	browser_window_set_drag_type(bw, DRAGGING_PAGE_SCROLL, NULL);

	bw->drag_start_x = x;
	bw->drag_start_y = y;

	if (bw->window != NULL) {
		/* Front end window */
		gui_window_get_scroll(bw->window, &bw->drag_start_scroll_x,
				&bw->drag_start_scroll_y);

		gui_window_scroll_start(bw->window);
	} else {
		/* Core managed browser window */
		bw->drag_start_scroll_x = scrollbar_get_offset(bw->scroll_x);
		bw->drag_start_scroll_y = scrollbar_get_offset(bw->scroll_y);
	}
}


/**
 * Check availability of Back action for a given browser window
 *
 * \param bw  browser window
 * \return true if Back action is available
 */

bool browser_window_back_available(struct browser_window *bw)
{
	return (bw && bw->history && history_back_available(bw->history));
}


/**
 * Check availability of Forward action for a given browser window
 *
 * \param bw  browser window
 * \return true if Forward action is available
 */

bool browser_window_forward_available(struct browser_window *bw)
{
	return (bw && bw->history && history_forward_available(bw->history));
}


/**
 * Check availability of Reload action for a given browser window
 *
 * \param bw  browser window
 * \return true if Reload action is available
 */

bool browser_window_reload_available(struct browser_window *bw)
{
	return (bw && bw->current_content && !bw->loading_content);
}


/**
 * Check availability of Stop action for a given browser window
 *
 * \param bw  browser window
 * \return true if Stop action is available
 */

bool browser_window_stop_available(struct browser_window *bw)
{
	return (bw && (bw->loading_content ||
			(bw->current_content &&
			(content_get_status(bw->current_content) != 
			CONTENT_STATUS_DONE))));
}
