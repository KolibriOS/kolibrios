/*
 * Copyright 2010 Vincent Sanders <vince@simtec.co.uk>
 *
 * Framebuffer windowing toolkit bitmaped image widget
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

#include <stdbool.h>
#include <stdlib.h>

#include <libnsfb.h>
#include <libnsfb_plot.h>

#include "desktop/browser.h"

#include "framebuffer/gui.h"
#include "framebuffer/fbtk.h"
#include "framebuffer/image_data.h"

#include "utils/log.h"
#include <menuet/os.h>

#include "widget.h"

static int
fb_redraw_bitmap(fbtk_widget_t *widget, fbtk_callback_info *cbi)
{
	LOG(("REDRAW BITMAP"));
	//__menuet__debug_out("REDRAW BITMAP");
	nsfb_bbox_t bbox;
	nsfb_bbox_t rect;
	nsfb_t *nsfb;

	LOG(("REDRAW BITMAP 1 "));
	//__menuet__debug_out("REDRAW BITMAP 1");


	nsfb = fbtk_get_nsfb(widget);

	LOG(("REDRAW BITMAP 2"));
	//__menuet__debug_out("REDRAW BITMAP 2");


	fbtk_get_bbox(widget, &bbox);

	rect = bbox;

	LOG(("REDRAW BITMAP 3 "));
	//__menuet__debug_out("REDRAW BITMAP 3");


	nsfb_claim(nsfb, &bbox);

	LOG(("REDRAW BITMAP 4"));
	//__menuet__debug_out("REDRAW BITMAP 4");

	/* clear background */
	if ((widget->bg & 0xFF000000) != 0) {
		/* transparent polygon filling isnt working so fake it */
		
			LOG(("REDRAW BITMAP 5"));
	//__menuet__debug_out("REDRAW BITMAP 5");

		nsfb_plot_rectangle_fill(nsfb, &bbox, widget->bg);
	}

	LOG(("REDRAW BITMAP 6"));
	//__menuet__debug_out("REDRAW BITMAP 6\n");

	/* plot the image */
	
	LOG(("STUB: DON'T REAL DRAW"));
	//__menuet__debug_out("STUB: DON'T REAL DRAW\n");
	
	
	LOG(("pixdata is %x", (nsfb_colour_t *)widget->u.bitmap.bitmap->pixdata));
	LOG(("pixdata is w:%d h:%d",widget->u.bitmap.bitmap->width,
			 widget->u.bitmap.bitmap->height));
	
	//hmm
	
	//int zap;
	//if (widget->u.bitmap.bitmap->width % 4 != 0) {
	//	zap = widget->u.bitmap.bitmap->width + 2; }
	
	//nsfb_plot_rectangle_fill(nsfb, &rect, 0xFFFFFF);
	
	
	nsfb_plot_bitmap(nsfb,
			 &rect,
			 (nsfb_colour_t *)widget->u.bitmap.bitmap->pixdata,
			 //0, 0, 0, 
			 widget->u.bitmap.bitmap->width,
			 widget->u.bitmap.bitmap->height,
			 widget->u.bitmap.bitmap->width, 
			 !widget->u.bitmap.bitmap->opaque);
	
	
		
	
	
	LOG(("REDRAW BITMAP 7"));
	//__menuet__debug_out("REDRAW BITMAP 7\n");

	nsfb_update(nsfb, &bbox);

	LOG(("REDRAW BITMAP OK\n"));
	//__menuet__debug_out("REDRAW BITMAP OK\n");

	return 0;
}

/* exported function documented in fbtk.h */
void
fbtk_set_bitmap(fbtk_widget_t *widget, struct fbtk_bitmap *image)
{
	LOG(("SET BITMAP"));
	//__menuet__debug_out("set BITMAP");
	if ((widget == NULL) || (widget->type != FB_WIDGET_TYPE_BITMAP))
		return;

	widget->u.bitmap.bitmap = image;

	fbtk_request_redraw(widget);
}

/* exported function documented in fbtk.h */
fbtk_widget_t *
fbtk_create_bitmap(fbtk_widget_t *parent,
		   int x,
		   int y,
		   int width,
		   int height,
		   colour c,
		   struct fbtk_bitmap *image)
{
	LOG(("CREATE BITMAP"));
	//__menuet__debug_out("cr BITMAP");
	fbtk_widget_t *neww;

	neww = fbtk_widget_new(parent, FB_WIDGET_TYPE_BITMAP, x, y, width, height);

	neww->bg = c;
	neww->mapped = true;
	neww->u.bitmap.bitmap = image;

	fbtk_set_handler(neww, FBTK_CBT_REDRAW, fb_redraw_bitmap, NULL);

	return neww;
}

/* exported function documented in fbtk.h */
fbtk_widget_t *
fbtk_create_button(fbtk_widget_t *parent,
		   int x,
		   int y,
		   int width,
		   int height,
		   colour c,
		   struct fbtk_bitmap *image,
		   fbtk_callback click,
		   void *pw)
{
	fbtk_widget_t *neww;

	LOG(("CREATE BUTTON BITMAP"));
	//__menuet__debug_out("cr bb BITMAP");
	neww = fbtk_widget_new(parent, FB_WIDGET_TYPE_BITMAP, x, y, width, height);

	neww->bg = c;
	neww->mapped = true;
	neww->u.bitmap.bitmap = image;

	fbtk_set_handler(neww, FBTK_CBT_REDRAW, fb_redraw_bitmap, NULL);
	fbtk_set_handler(neww, FBTK_CBT_CLICK, click, pw);
	fbtk_set_handler(neww, FBTK_CBT_POINTERENTER, fbtk_set_ptr, &hand_image);

	return neww;
}

/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
