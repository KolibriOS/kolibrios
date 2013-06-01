/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
 *
 * Plot code common to all bpp just with differing types
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#ifndef PLOT_TYPE
#error PLOT_TYPE must be set to uint8_t, uint16_t, or uint32_t
#endif
#ifndef PLOT_LINELEN
#error PLOT_LINELEN must be a macro to increment a line length
#endif

#include "palette.h"

#define SIGN(x)  ((x<0) ?  -1  :  ((x>0) ? 1 : 0))

static bool
line(nsfb_t *nsfb, int linec, nsfb_bbox_t *line, nsfb_plot_pen_t *pen)
{
        int w;
        PLOT_TYPE ent;
        PLOT_TYPE *pvideo;
        int x, y, i;
        int dx, dy, sdy;
        int dxabs, dyabs;

        ent = colour_to_pixel(nsfb, pen->stroke_colour);

        for (;linec > 0; linec--) {

                if (line->y0 == line->y1) {
                        /* horizontal line special cased */

                        if (!nsfb_plot_clip_ctx(nsfb, line)) {
                                /* line outside clipping */
                                line++;
                                continue;
                        }

                        pvideo = get_xy_loc(nsfb, line->x0, line->y0);

                        w = line->x1 - line->x0;
                        while (w-- > 0)
                                *(pvideo + w) = ent;

                } else {
                        /* standard bresenham line */

                        if (!nsfb_plot_clip_line_ctx(nsfb, line)) {
                                /* line outside clipping */
                                line++;
                                continue;
                        }

                        /* the horizontal distance of the line */
                        dx = line->x1 - line->x0;
                        dxabs = abs (dx);

                        /* the vertical distance of the line */
                        dy = line->y1 - line->y0;
                        dyabs = abs (dy);

                        sdy = dx ? SIGN(dy) * SIGN(dx) : SIGN(dy);

                        if (dx >= 0)
                                pvideo = get_xy_loc(nsfb, line->x0, line->y0);
                        else
                                pvideo = get_xy_loc(nsfb, line->x1, line->y1);

                        x = dyabs >> 1;
                        y = dxabs >> 1;

                        if (dxabs >= dyabs) {
                                /* the line is more horizontal than vertical */
                                for (i = 0; i < dxabs; i++) {
                                        *pvideo = ent;

                                        pvideo++;
                                        y += dyabs;
                                        if (y >= dxabs) {
                                                y -= dxabs;
                                                pvideo += sdy * PLOT_LINELEN(nsfb->linelen);
                                        }
                                }
                        } else {
                                /* the line is more vertical than horizontal */
                                for (i = 0; i < dyabs; i++) {
                                        *pvideo = ent;
                                        pvideo += sdy * PLOT_LINELEN(nsfb->linelen);

                                        x += dxabs;
                                        if (x >= dyabs) {
                                                x -= dyabs;
                                                pvideo++;
                                        }
                                }
                        }

                }
                line++;
        }
        return true;
}


static bool point(nsfb_t *nsfb, int x, int y, nsfb_colour_t c)
{
        PLOT_TYPE *pvideo;

        /* check point lies within clipping region */
        if ((x < nsfb->clip.x0) ||
            (x >= nsfb->clip.x1) ||
            (y < nsfb->clip.y0) ||
            (y >= nsfb->clip.y1))
                return true;

        pvideo = get_xy_loc(nsfb, x, y);

        if ((c & 0xFF000000) != 0) {
                if ((c & 0xFF000000) != 0xFF000000) {
                        c = nsfb_plot_ablend(c, pixel_to_colour(nsfb, *pvideo));
                }

                *pvideo = colour_to_pixel(nsfb, c);
        }
        return true;
}

static bool
glyph1(nsfb_t *nsfb,
       nsfb_bbox_t *loc,
       const uint8_t *pixel,
       int pitch,
       nsfb_colour_t c)
{
        PLOT_TYPE *pvideo;
        PLOT_TYPE const *pvideo_limit;
        PLOT_TYPE fgcol;
        int xloop;
        int xoff, yoff; /* x and y offset into image */
        int x = loc->x0;
        int y = loc->y0;
        int width;
        int height;
        const size_t line_len = PLOT_LINELEN(nsfb->linelen);
        const int first_col = 1 << (loc->x1 - loc->x0 - 1);
        const uint8_t *row;

        if (!nsfb_plot_clip_ctx(nsfb, loc))
                return true;

        height = loc->y1 - y;
        width = loc->x1 - x;

        xoff = loc->x0 - x;
        yoff = loc->y0 - y;

        fgcol = colour_to_pixel(nsfb, c);

        pitch >>= 3; /* bits to bytes */

        pvideo = get_xy_loc(nsfb, x, loc->y0);
        pvideo_limit = pvideo + line_len * (height - yoff);
        row = pixel + yoff * pitch;

        for (; pvideo < pvideo_limit; pvideo += line_len) {
                for (xloop = xoff; xloop < width; xloop++) {

                        if ((*row & (first_col >> xloop)) != 0) {
                                *(pvideo + xloop) = fgcol;
                        }
                }
                row += pitch;
        }

        return true;
}

static bool
glyph8(nsfb_t *nsfb,
       nsfb_bbox_t *loc,
       const uint8_t *pixel,
       int pitch,
       nsfb_colour_t c)
{
        PLOT_TYPE *pvideo;
        nsfb_colour_t fgcol;
        nsfb_colour_t abpixel; /* alphablended pixel */
        int xloop, yloop;
        int xoff, yoff; /* x and y offset into image */
        int x = loc->x0;
        int y = loc->y0;
        int width;
        int height;

        if (!nsfb_plot_clip_ctx(nsfb, loc))
                return true;

        height = (loc->y1 - loc->y0);
        width = (loc->x1 - loc->x0);

        xoff = loc->x0 - x;
        yoff = loc->y0 - y;

        pvideo = get_xy_loc(nsfb, loc->x0, loc->y0);

        fgcol = c & 0xFFFFFF;

        for (yloop = 0; yloop < height; yloop++) {
                for (xloop = 0; xloop < width; xloop++) {
                        abpixel = (pixel[((yoff + yloop) * pitch) + xloop + xoff] << 24) | fgcol;
                        if ((abpixel & 0xFF000000) != 0) {
                                /* pixel is not transparent */
                                if ((abpixel & 0xFF000000) != 0xFF000000) {
                                        abpixel = nsfb_plot_ablend(abpixel,
                                                                   pixel_to_colour(nsfb, *(pvideo + xloop)));
                                }

                                *(pvideo + xloop) = colour_to_pixel(nsfb, abpixel);
                        }
                }
                pvideo += PLOT_LINELEN(nsfb->linelen);
        }

        return true;
}

static bool bitmap_scaled(nsfb_t *nsfb, const nsfb_bbox_t *loc,
		const nsfb_colour_t *pixel, int bmp_width, int bmp_height,
		int bmp_stride, bool alpha)
{
	PLOT_TYPE *pvideo;
	PLOT_TYPE *pvideo_limit;
	nsfb_colour_t abpixel; /* alphablended pixel */
	int xloop;
	int xoff, yoff, xoffs; /* x and y offsets into image */
	int x = loc->x0;
	int y = loc->y0;
	int width = loc->x1 - loc->x0; /* size to scale to */
	int height = loc->y1 - loc->y0; /* size to scale to */
	int rheight, rwidth; /* post-clipping render area dimensions */
	int dx, dy; /* scale factor (integer part) */
	int dxr, dyr; /* scale factor (remainder) */
	int rx, ry, rxs; /* remainder trackers */
	nsfb_bbox_t clipped; /* clipped display */

	/* The part of the scaled image actually displayed is cropped to the
	 * current context. */
	clipped.x0 = x;
	clipped.y0 = y;
	clipped.x1 = x + width;
	clipped.y1 = y + height;

	if (!nsfb_plot_clip_ctx(nsfb, &clipped))
		return true;

	/* get height of rendering region, after clipping */
	if (height > (clipped.y1 - clipped.y0))
		rheight = (clipped.y1 - clipped.y0);
	else
		rheight = height;

	/* get width of rendering region, after clipping */
	if (width > (clipped.x1 - clipped.x0))
		rwidth = (clipped.x1 - clipped.x0);
	else
		rwidth = width;

	if (nsfb->palette != NULL) {
		nsfb_palette_dither_init(nsfb->palette, rwidth);
	}

	/* get veritcal (y) and horizontal (x) scale factors; both integer
	 * part and remainder */
	dx = bmp_width / width;
	dy = (bmp_height / height) * bmp_stride;
	dxr = bmp_width % width;
	dyr = bmp_height % height;

	/* get start offsets to part of image being scaled, after clipping and
	 * set remainder trackers to correct starting value */
	if (clipped.x0 - x != 0) {
		xoffs = ((clipped.x0 - x) * bmp_width) / width;
		rxs = ((clipped.x0 - x) * bmp_width) % width;
	} else {
		xoffs = 0;
		rxs = 0;
	}
	if (clipped.y0 - y != 0) {
		yoff = (((clipped.y0 - y) * bmp_height) / height) * bmp_stride;
		ry = ((clipped.y0 - y) * bmp_height) % height;
	} else {
		yoff = 0;
		ry = 0;
	}

	/* plot the image */
	pvideo = get_xy_loc(nsfb, clipped.x0, clipped.y0);
	pvideo_limit = pvideo + PLOT_LINELEN(nsfb->linelen) * rheight;
	if (alpha) {
		for (; pvideo < pvideo_limit;
				pvideo += PLOT_LINELEN(nsfb->linelen)) {
			/* looping through render area vertically */
			xoff = xoffs;
			rx = rxs;
			for (xloop = 0; xloop < rwidth; xloop++) {
				/* looping through render area horizontally */
				/* get value of source pixel in question */
				abpixel = pixel[yoff + xoff];
				if ((abpixel & 0xFF000000) != 0) {
					/* pixel is not transparent; have to
					 * plot something */
					if ((abpixel & 0xFF000000) !=
							0xFF000000) {
						/* pixel is not opaque; need to
						 * blend */
						abpixel = nsfb_plot_ablend(
								abpixel,
								pixel_to_colour(
								nsfb, 
								*(pvideo +
								xloop)));
					}
					/* plot pixel */
					*(pvideo + xloop) = colour_to_pixel(
							nsfb, abpixel);
				}
				/* handle horizontal interpolation */
				xoff += dx;
				rx += dxr;
				if (rx >= width) {
					xoff++;
					rx -= width;
				}
			}
			/* handle vertical interpolation */
			yoff += dy;
			ry += dyr;
			if (ry >= height) {
				yoff += bmp_stride;
				ry -= height;
			}
		}
	} else {
		for (; pvideo < pvideo_limit;
				pvideo += PLOT_LINELEN(nsfb->linelen)) {
			/* looping through render area vertically */
			xoff = xoffs;
			rx = rxs;
			for (xloop = 0; xloop < rwidth; xloop++) {
				/* looping through render area horizontally */
				/* get value of source pixel in question */
				abpixel = pixel[yoff + xoff];
				/* plot pixel */
				*(pvideo + xloop) = colour_to_pixel(
						nsfb, abpixel);

				/* handle horizontal interpolation */
				xoff += dx;
				rx += dxr;
				if (rx >= width) {
					xoff++;
					rx -= width;
				}
			}
			/* handle vertical interpolation */
			yoff += dy;
			ry += dyr;
			if (ry >= height) {
				yoff += bmp_stride;
				ry -= height;
			}
		}
	}

	if (nsfb->palette != NULL) {
		nsfb_palette_dither_fini(nsfb->palette);
	}

	return true;
}

static bool
bitmap(nsfb_t *nsfb,
       const nsfb_bbox_t *loc,
       const nsfb_colour_t *pixel,
       int bmp_width,
       int bmp_height,
       int bmp_stride,
       bool alpha)
{
        PLOT_TYPE *pvideo;
        nsfb_colour_t abpixel; /* alphablended pixel */
        int xloop, yloop;
        int xoff, yoff; /* x and y offset into image */
        int x = loc->x0;
        int y = loc->y0;
        int width = loc->x1 - loc->x0;
        int height = loc->y1 - loc->y0;
        nsfb_bbox_t clipped; /* clipped display */

        if (width == 0 || height == 0)
                return true;

        /* Scaled bitmaps are handled by a separate function */
        if (width != bmp_width || height != bmp_height)
                return bitmap_scaled(nsfb, loc, pixel, bmp_width, bmp_height,
                                bmp_stride, alpha);

        /* The part of the image actually displayed is cropped to the
         * current context. */
        clipped.x0 = x;
        clipped.y0 = y;
        clipped.x1 = x + width;
        clipped.y1 = y + height;

        if (!nsfb_plot_clip_ctx(nsfb, &clipped))
                return true;

        if (height > (clipped.y1 - clipped.y0))
                height = (clipped.y1 - clipped.y0);

        if (width > (clipped.x1 - clipped.x0))
                width = (clipped.x1 - clipped.x0);

        if (nsfb->palette != NULL) {
                nsfb_palette_dither_init(nsfb->palette, width);
        }

        xoff = clipped.x0 - x;
        yoff = (clipped.y0 - y) * bmp_stride;
        height = height * bmp_stride + yoff;

        /* plot the image */
        pvideo = get_xy_loc(nsfb, clipped.x0, clipped.y0);

        if (alpha) {
                for (yloop = yoff; yloop < height; yloop += bmp_stride) {
                        for (xloop = 0; xloop < width; xloop++) {
                                abpixel = pixel[yloop + xloop + xoff];
                                if ((abpixel & 0xFF000000) != 0) {
                                        /* pixel is not transparent; have to
                                         * plot something */
                                        if ((abpixel & 0xFF000000) !=
                                                       0xFF000000) {
                                                /* pixel is not opaque; need to
                                                 * blend */
                                                abpixel = nsfb_plot_ablend(
                                                                abpixel,
                                                                pixel_to_colour(
                                                                nsfb,
                                                                *(pvideo +
                                                                xloop)));
                                        }

                                        *(pvideo + xloop) = colour_to_pixel(
                                                        nsfb, abpixel);
                                }
                        }
                        pvideo += PLOT_LINELEN(nsfb->linelen);
                }
        } else {
                for (yloop = yoff; yloop < height; yloop += bmp_stride) {
                        for (xloop = 0; xloop < width; xloop++) {
                                abpixel = pixel[yloop + xloop + xoff];
                                *(pvideo + xloop) = colour_to_pixel(
                                                nsfb, abpixel);
                        }
                        pvideo += PLOT_LINELEN(nsfb->linelen);
                }
        }

        if (nsfb->palette != NULL) {
                nsfb_palette_dither_fini(nsfb->palette);
        }

        return true;
}

static bool readrect(nsfb_t *nsfb, nsfb_bbox_t *rect, nsfb_colour_t *buffer)
{
        PLOT_TYPE *pvideo;
        int xloop, yloop;
        int width;

        if (!nsfb_plot_clip_ctx(nsfb, rect)) {
                return true;
        }

        width = rect->x1 - rect->x0;

        pvideo = get_xy_loc(nsfb, rect->x0, rect->y0);

        for (yloop = rect->y0; yloop < rect->y1; yloop += 1) {
                for (xloop = 0; xloop < width; xloop++) {
                        *buffer = pixel_to_colour(nsfb, *(pvideo + xloop));
                        buffer++;
                }
                pvideo += PLOT_LINELEN(nsfb->linelen);
        }
        return true;
}


/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
