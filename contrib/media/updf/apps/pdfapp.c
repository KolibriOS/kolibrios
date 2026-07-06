/*
 * uPDF glue layer for MuPDF 1.19 (KolibriOS port).
 * Renders pages into BGRA pixmaps for the KolibriOS blitter and exposes
 * the operations the GUI (kos_main.c) drives: open/close, render current or
 * neighbour page, zoom/rotate/grayscale/navigate, and text selection.
 */

#include "pdfapp.h"
#include <string.h>
#include <stdlib.h>

/* provided by kos_main.c */
extern void winrepaint(pdfapp_t *app);

#define ZOOMSTEP 1.2f
#define MINDPI   18.0f
#define MAXDPI   1200.0f
#define MAX_QUADS 512
/* KolibriOS has modest RAM: keep the resource store bounded */
#define STORE_LIMIT (32 << 20)

/* ---- helpers --------------------------------------------------------- */

static fz_matrix pdfapp_computectm(pdfapp_t *app)
{
	fz_matrix ctm = fz_scale(app->resolution / 72.0f, app->resolution / 72.0f);
	ctm = fz_pre_rotate(ctm, (float)app->rotate);
	return ctm;
}

/* Invert a device-space rectangle in app->image (BGRA). Self-inverse, so
   calling twice with the same rect restores the pixels. */
static void invert_rect(pdfapp_t *app, int dx0, int dy0, int dx1, int dy1)
{
	fz_pixmap *img = app->image;
	int x0, y0, x1, y1, x, y, k;
	unsigned char *p;

	if (!img)
		return;
	x0 = dx0 - img->x; x1 = dx1 - img->x;
	y0 = dy0 - img->y; y1 = dy1 - img->y;
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 > img->w) x1 = img->w;
	if (y1 > img->h) y1 = img->h;

	for (y = y0; y < y1; y++)
	{
		p = img->samples + (ptrdiff_t)y * img->stride + (ptrdiff_t)x0 * img->n;
		for (x = x0; x < x1; x++)
			for (k = 0; k < img->n; k++, p++)
				*p = 255 - *p;
	}
}

/* Convert a BGRA pixmap to grayscale by luminance, in place. The KolibriOS
   blitter needs 32bpp, so we desaturate rather than render fz_device_gray. */
static void desaturate(fz_pixmap *pix)
{
	unsigned char *row;
	int x, y, l;

	if (!pix || pix->n < 3)
		return;
	for (y = 0; y < pix->h; y++)
	{
		row = pix->samples + (ptrdiff_t)y * pix->stride;
		for (x = 0; x < pix->w; x++, row += pix->n)
		{
			l = (row[2] * 77 + row[1] * 150 + row[0] * 29) >> 8; /* bgr */
			row[0] = row[1] = row[2] = (unsigned char)l;
		}
	}
}

/* Render a page into a fresh BGRA pixmap on a WHITE background.
   fz_new_pixmap_from_page(alpha=1) clears to transparent black; since the
   KolibriOS blitter has no alpha it then shows a black page for any PDF that
   doesn't paint its own background. So we allocate the pixmap ourselves,
   clear it to opaque white, and run the page over it. Alpha stays (n=4)
   because the blitter needs 32bpp. */
static fz_pixmap *render_bgra_white(pdfapp_t *app, fz_page *page, fz_matrix ctm)
{
	fz_irect bbox;
	fz_pixmap *pix;
	fz_device *dev = NULL;

	bbox = fz_round_rect(fz_transform_rect(fz_bound_page(app->ctx, page), ctm));
	pix = fz_new_pixmap_with_bbox(app->ctx, fz_device_bgr(app->ctx), bbox, NULL, 1);

	fz_var(dev);
	fz_try(app->ctx)
	{
		fz_clear_pixmap_with_value(app->ctx, pix, 0xff); /* opaque white paper */
		dev = fz_new_draw_device(app->ctx, fz_identity, pix);
		fz_run_page(app->ctx, page, dev, ctm, NULL);
		fz_close_device(app->ctx, dev);
	}
	fz_always(app->ctx)
		fz_drop_device(app->ctx, dev);
	fz_catch(app->ctx)
	{
		fz_drop_pixmap(app->ctx, pix);
		fz_rethrow(app->ctx);
	}
	return pix;
}

static void pdfapp_loadpage(pdfapp_t *app)
{
	if (app->stext) { fz_drop_stext_page(app->ctx, app->stext); app->stext = NULL; }
	if (app->page)  { fz_drop_page(app->ctx, app->page); app->page = NULL; }

	fz_try(app->ctx)
	{
		app->page = fz_load_page(app->ctx, app->doc, app->pageno - 1);
		app->stext = fz_new_stext_page_from_page(app->ctx, app->page, NULL);
	}
	fz_catch(app->ctx)
	{
		app->page = NULL;
		app->stext = NULL;
	}
}

/* ---- lifecycle ------------------------------------------------------- */

void pdfapp_init(pdfapp_t *app)
{
	memset(app, 0, sizeof(*app));
	app->resolution = 72;
	app->pageno = 1;
	app->ctx = fz_new_context(NULL, NULL, STORE_LIMIT);
	if (app->ctx)
		fz_register_document_handlers(app->ctx);
}

void pdfapp_open(pdfapp_t *app, char *filename, int fd, int reload)
{
	(void)fd; (void)reload;
	if (!app->ctx)
		return;

	fz_try(app->ctx)
	{
		app->doc = fz_open_document(app->ctx, filename);
		app->pagecount = fz_count_pages(app->ctx, app->doc);
	}
	fz_catch(app->ctx)
	{
		app->doc = NULL;
		app->pagecount = 0;
		return;
	}

	app->doctitle = filename;
	if (app->pageno < 1) app->pageno = 1;
	if (app->pageno > app->pagecount) app->pageno = app->pagecount;

	pdfapp_showpage(app, 1, 1, 1);
}

void pdfapp_close(pdfapp_t *app)
{
	if (!app->ctx)
		return;
	if (app->stext) fz_drop_stext_page(app->ctx, app->stext);
	if (app->page)  fz_drop_page(app->ctx, app->page);
	if (app->image) fz_drop_pixmap(app->ctx, app->image);
	if (app->doc)   fz_drop_document(app->ctx, app->doc);
	fz_drop_context(app->ctx);
	memset(app, 0, sizeof(*app));
}

/* ---- rendering ------------------------------------------------------- */

/* Render an arbitrary page into a fresh BGRA pixmap (caller owns the ref).
   Used by the continuous-scroll cache for neighbour pages. */
fz_pixmap *pdfapp_renderpage(pdfapp_t *app, int pageno)
{
	fz_pixmap *pix = NULL;
	fz_page *page = NULL;
	fz_matrix ctm = pdfapp_computectm(app);

	if (!app->doc || pageno < 1 || pageno > app->pagecount)
		return NULL;

	fz_var(page);
	fz_var(pix);
	fz_try(app->ctx)
	{
		page = fz_load_page(app->ctx, app->doc, pageno - 1);
		pix = render_bgra_white(app, page, ctm);
	}
	fz_always(app->ctx)
	{
		if (page) fz_drop_page(app->ctx, page);
	}
	fz_catch(app->ctx)
	{
		pix = NULL;
	}

	if (pix && app->grayscale)
		desaturate(pix);
	return pix;
}

void pdfapp_showpage(pdfapp_t *app, int loadpage, int drawpage, int repaint)
{
	app->ctm = pdfapp_computectm(app);

	if (loadpage)
		pdfapp_loadpage(app);

	if (drawpage && app->page)
	{
		if (app->image)
		{
			fz_drop_pixmap(app->ctx, app->image);
			app->image = NULL;
		}
		fz_try(app->ctx)
			app->image = render_bgra_white(app, app->page, app->ctm);
		fz_catch(app->ctx)
			app->image = NULL;

		if (app->image && app->grayscale)
			desaturate(app->image);
	}

	app->sel_valid = 0; /* highlight no longer valid after a re-render */

	if (repaint)
		winrepaint(app);
}

/* ---- input ----------------------------------------------------------- */

void pdfapp_onkey(pdfapp_t *app, int c)
{
	switch (c)
	{
	case '+':
	case '=':
		app->resolution *= ZOOMSTEP;
		if (app->resolution > MAXDPI) app->resolution = MAXDPI;
		pdfapp_showpage(app, 0, 1, 1);
		break;
	case '-':
		app->resolution /= ZOOMSTEP;
		if (app->resolution < MINDPI) app->resolution = MINDPI;
		pdfapp_showpage(app, 0, 1, 1);
		break;
	case 'L':
		app->rotate = (app->rotate + 270) % 360;
		pdfapp_showpage(app, 0, 1, 1);
		break;
	case 'R':
		app->rotate = (app->rotate + 90) % 360;
		pdfapp_showpage(app, 0, 1, 1);
		break;
	case 'c':
		app->grayscale ^= 1;
		pdfapp_showpage(app, 0, 1, 1);
		break;
	case 'g': /* first page */
		app->pageno = 1;
		app->pany = 0;
		pdfapp_showpage(app, 1, 1, 1);
		break;
	case 'G': /* last page */
		app->pageno = app->pagecount;
		app->pany = 0;
		pdfapp_showpage(app, 1, 1, 1);
		break;
	case '[': /* previous page */
		if (app->pageno > 1) app->pageno--;
		app->pany = 0;
		pdfapp_showpage(app, 1, 1, 1);
		break;
	case ']': /* next page */
		if (app->pageno < app->pagecount) app->pageno++;
		app->pany = 0;
		pdfapp_showpage(app, 1, 1, 1);
		break;
	}
}

/* ---- selection ------------------------------------------------------- */

fz_point pdfapp_devtopage(pdfapp_t *app, int px, int py)
{
	fz_point p;
	fz_matrix inv = fz_invert_matrix(app->ctm);

	p.x = (float)(px + (app->image ? app->image->x : 0));
	p.y = (float)(py + (app->image ? app->image->y : 0));
	return fz_transform_point(p, inv);
}

void pdfapp_invertselection(pdfapp_t *app)
{
	fz_quad quads[MAX_QUADS];
	int n, i;

	if (!app->sel_valid || !app->stext || !app->image)
		return;

	n = fz_highlight_selection(app->ctx, app->stext, app->sel_a, app->sel_b,
		quads, MAX_QUADS);
	for (i = 0; i < n; i++)
	{
		fz_quad q = fz_transform_quad(quads[i], app->ctm);
		fz_rect r = fz_rect_from_quad(q);
		invert_rect(app, (int)r.x0, (int)r.y0, (int)(r.x1 + 0.999f), (int)(r.y1 + 0.999f));
	}
}

char *pdfapp_copyselection(pdfapp_t *app)
{
	if (!app->sel_valid || !app->stext)
		return NULL;
	return fz_copy_selection(app->ctx, app->stext, app->sel_a, app->sel_b, 0);
}
