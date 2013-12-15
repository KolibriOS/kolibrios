/*
 * Copyright 2010 Vincent Sanders <vince@simtec.co.uk>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <libnsfb.h>
#include <libnsfb_plot.h>
#include <libnsfb_event.h>

#include "desktop/browser_private.h"
#include "desktop/gui.h"
#include "desktop/plotters.h"
#include "desktop/netsurf.h"
#include "desktop/options.h"
#include "utils/log.h"
#include "utils/url.h"
#include "utils/messages.h"
#include "utils/utils.h"
#include "desktop/textinput.h"
#include "render/form.h"

#include "framebuffer/gui.h"
#include "framebuffer/fbtk.h"
#include "framebuffer/framebuffer.h"
#include "framebuffer/schedule.h"
#include "framebuffer/findfile.h"
#include "framebuffer/image_data.h"
#include "framebuffer/font.h"

#include "content/urldb.h"
#include "desktop/history_core.h"
#include "content/fetch.h"

static int
localhistory_redraw(fbtk_widget_t *widget, fbtk_callback_info *cbi)
{
	struct gui_localhistory *glh = cbi->context;
	nsfb_bbox_t rbox;

	struct redraw_context ctx = {
		.interactive = true,
		.background_images = true,
		.plot = &fb_plotters
	};

	rbox.x0 = fbtk_get_absx(widget);
	rbox.y0 = fbtk_get_absy(widget);

	rbox.x1 = rbox.x0 + fbtk_get_width(widget);
	rbox.y1 = rbox.y0 + fbtk_get_height(widget);

	nsfb_claim(fbtk_get_nsfb(widget), &rbox);

	nsfb_plot_rectangle_fill(fbtk_get_nsfb(widget), &rbox, 0xffffffff);

	history_redraw_rectangle(glh->bw->history,
				 glh->scrollx,
				 glh->scrolly,
				 fbtk_get_width(widget) + glh->scrollx,
				 fbtk_get_height(widget) + glh->scrolly,
				 0, 0, &ctx);

	nsfb_update(fbtk_get_nsfb(widget), &rbox);

	return 0;
}

static int
localhistory_click(fbtk_widget_t *widget, fbtk_callback_info *cbi)
{
	struct gui_localhistory *glh = cbi->context;

	if (cbi->event->type != NSFB_EVENT_KEY_UP)
		return 0;

	history_click(glh->bw, glh->bw->history, cbi->x, cbi->y, false);

	fbtk_set_mapping(glh->window, false);

	return 1;
}

struct gui_localhistory *
fb_create_localhistory(struct browser_window *bw,
		       fbtk_widget_t *parent,
		       int furniture_width)
{
	
	LOG(("init local hist.."));
	
	struct gui_localhistory *glh;
LOG(("LH calloc."));
	
	glh = calloc(1, sizeof(struct gui_localhistory));


	if (glh == NULL)
		return NULL;

	glh->bw = bw;

LOG(("conatiner.."));
	
	/* container window */
	LOG(("fbtk window"));
	
	glh->window = fbtk_create_window(parent, 0, 0, 0, 0, 0);

LOG(("hist user"));
	
	glh->history = fbtk_create_user(glh->window, 0, 0, -furniture_width, -furniture_width, glh);
LOG(("hist handlers..."));
	

	fbtk_set_handler(glh->history, FBTK_CBT_REDRAW, localhistory_redraw, glh);
	fbtk_set_handler(glh->history, FBTK_CBT_CLICK, localhistory_click, glh);
	/*
	  fbtk_set_handler(gw->localhistory, FBTK_CBT_INPUT, fb_browser_window_input, gw);
	  fbtk_set_handler(gw->localhistory, FBTK_CBT_POINTERMOVE, fb_browser_window_move, bw);
	*/

	/* create horizontal scrollbar */
	
	LOG(("init hor scroll"));
	
	glh->hscroll = fbtk_create_hscroll(glh->window,
					   0,
					   fbtk_get_height(glh->window) - furniture_width,
					   fbtk_get_width(glh->window) - furniture_width,
					   furniture_width,
					   FB_SCROLL_COLOUR,
					   FB_FRAME_COLOUR,
					   NULL,
					   NULL);

	LOG(("init ver scroll"));
	glh->vscroll = fbtk_create_vscroll(glh->window,
					   fbtk_get_width(glh->window) - furniture_width,
					   0,
					   furniture_width,
					   fbtk_get_height(glh->window) - furniture_width,
					   FB_SCROLL_COLOUR,
					   FB_FRAME_COLOUR,
					   NULL,
					   NULL);

	LOG(("init fill"));
	fbtk_create_fill(glh->window,
			 fbtk_get_width(glh->window) - furniture_width,
			 fbtk_get_height(glh->window) - furniture_width,
			 furniture_width,
			 furniture_width,
			 FB_FRAME_COLOUR);

	LOG(("fine!"));
	return glh;
}

void
fb_localhistory_map(struct gui_localhistory * glh)
{
	fbtk_set_zorder(glh->window, INT_MIN);
	fbtk_set_mapping(glh->window, true);
}
