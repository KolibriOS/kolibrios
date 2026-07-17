/*
 * uPDF glue layer for MuPDF 1.19 (KolibriOS port).
 * Bridges the KolibriOS GUI (kos_main.c) to the fz_context-based API.
 * Keeps the same function names the GUI expects, so the continuous-scroll,
 * scrollbar, drag-pan and selection code needs only minimal changes.
 */

#ifndef PDFAPP_H
#define PDFAPP_H

#include "mupdf/fitz.h"

typedef struct pdfapp_s pdfapp_t;

struct pdfapp_s
{
	/* MuPDF state */
	fz_context   *ctx;
	fz_document  *doc;
	fz_page      *page;      /* current page object */
	fz_pixmap    *image;     /* current page rendered as BGRA */
	fz_stext_page *stext;    /* current page text, for selection */
	fz_matrix     ctm;       /* page-space -> device(pixmap) transform */

	char         *doctitle;

	/* view state */
	int   pagecount;
	int   pageno;            /* 1-based */
	float resolution;        /* dpi; 72 == 100% */
	int   rotate;            /* degrees */
	int   grayscale;
	int   ss_factor;         /* supersampling factor (1 = off) for sharper text */

	/* scroll offset (managed by the GUI) */
	int   panx, pany;

	/* text selection, in page space */
	fz_point sel_a, sel_b;
	int      sel_valid;

	/* legacy fields the GUI still sets during init */
	int scrw, scrh, shrinkwrap, fd, reload;
};

/* lifecycle */
void pdfapp_init(pdfapp_t *app);
void pdfapp_open(pdfapp_t *app, char *filename, int fd, int reload);
void pdfapp_close(pdfapp_t *app);

/* rendering */
void       pdfapp_showpage(pdfapp_t *app, int loadpage, int drawpage, int repaint);
fz_pixmap *pdfapp_renderpage(pdfapp_t *app, int pageno);

/* input: zoom/rotate/grayscale/navigation, same key codes as before
   ( + - L R c g G [ ] ) */
void pdfapp_onkey(pdfapp_t *app, int c);

/* snap the current zoom to exactly 100% if it is within ~7% of it */
void pdfapp_snapzoom(pdfapp_t *app);

/* selection: a/b are in page space; the GUI supplies them from clicks */
void  pdfapp_invertselection(pdfapp_t *app); /* toggle highlight in image */
char *pdfapp_copyselection(pdfapp_t *app);   /* fz-allocated UTF-8, or NULL */

/* map a device pixel (relative to the current image) to a page-space point */
fz_point pdfapp_devtopage(pdfapp_t *app, int px, int py);

#endif
