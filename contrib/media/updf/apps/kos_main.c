/*==== INCLUDES ====*/

#include "mupdf/fitz.h"
#include "pdfapp.h"
#include "icons/allbtns.h"
#include "kolibri.h"
#include <string.h>


/*==== DATA ====*/

static char Title[1024] = "uPDF";
static pdfapp_t gapp;
char debugstr[256];
char do_not_blit=0;

// MuPDF 0.9 defined these in fitz.h; 1.19 does not (it uses fz_mini/etc).
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ABS(x)   ((x) < 0 ? -(x) : (x))

#define TOOLBAR_HEIGHT 34
struct proc_info Form;

#define DOCUMENT_BORDER 0x979797
#define DOCUMENT_BG 0xABABAB

#define SCROLL_STEP 50    // pixels moved per scroll gesture (arrow key / wheel notch)
#define PAGE_GAP    14    // gray gap between pages, like in a word processor
#define SIDE_MARGIN 8     // small side gap when fitting a page to the width

// keyboard scancodes (layout-independent, from fn 2 bits 16-23)
#define SC_ESC        0x01
#define SC_MINUS      0x0C  // main row -
#define SC_EQUAL      0x0D  // main row =/+
#define SC_R          0x13
#define SC_LBRACKET   0x1A  // [
#define SC_RBRACKET   0x1B  // ]
#define SC_G          0x22
#define SC_L          0x26
#define SC_W          0x11  // fit page to width
#define SC_S          0x1F  // toggle supersampling (sharper text)
#define SC_NUM_MINUS  0x4A  // keypad -
#define SC_NUM_PLUS   0x4E  // keypad +
#define SC_HOME       0x47
#define SC_UP         0x48
#define SC_PGUP       0x49
#define SC_END        0x4F
#define SC_DOWN       0x50
#define SC_PGDN       0x51

// vertical scrollbar in the WebView style: flat box_lib type 2, no arrows
#define SCROLL_W     15
#define SB_BG_COL    0xEEEEEE  // track
#define SB_FRONT_COL 0xBBBBBB  // runner
#define SB_MIN_RUN   10        // minimal runner height, as in box_lib

static int sb_run_y, sb_run_h;  // runner rect (client coords), for hit testing
static char sb_active;          // the document is longer than the viewport
static char sb_drag, sb_lmb_prev;
static int sb_drag_off;         // pointer offset inside the runner while dragging

// mouse interaction on the page: left = select text, right = drag (pan)
static char sel_dragging;       // left button held, building a selection
static char sel_has;            // a highlighted selection is on screen
static fz_point sel_anchor;     // selection start, in page space
static char pan_dragging;       // right button held, panning the page
static int  pan_last_my;        // last pointer y while panning

// look-ahead cache of rendered neighbour pages for continuous scrolling
#define PAGE_CACHE_N 3
static fz_pixmap *cache_pix[PAGE_CACHE_N];
static int cache_no[PAGE_CACHE_N];
static float cache_res = -1;
static int cache_rot = -1, cache_gray = -1;

short show_area_w = 65;
short show_area_x;

char key_mode_enter_page_number;
int new_page_number;

// zoom entry box between the +/- buttons (mirrors the page-number box)
#define ZOOM_AREA_W 50
static short zoom_area_x;
char key_mode_enter_zoom;
int new_zoom;

static char fit_pending = 1;    // fit the page to width on the first draw

const char *help[] = {
	"Keys:",
	" ",
	"PageUp / PageDown - previous / next page",
	"Home / End        - first / last page",
	"Up / Down arrow   - scroll",
	"+ / -             - zoom in / out",
	"w                 - fit page to width",
	"[ or l  /  ] or r - rotate page left / right",
	"g                 - grayscale on / off",
	"s                 - supersampling (sharper text) on / off",
	" ",
	"Mouse:",
	"Left drag         - select text (copies to clipboard)",
	"Right drag        - pan the page",
	"Wheel             - scroll",
	"Click page or zoom box - type a value",
	"  ",
	"Press Escape to hide help",
	0
};

/*==== CODE ====*/
// Prototypes //
void RunApp(char app[], char param[]);
void winblit(pdfapp_t *app);
void DrawPagination(void);
void HandleNewPageNumber(unsigned char key);
void ApplyNewPageNumber(void);
void DrawZoom(void);
void GetNewZoom(void);
void HandleNewZoom(unsigned char key);
void ApplyNewZoom(void);
void DrawMainWindow(void);
void FlushPageCache(void);
void SyncCacheParams(void);
void StashPixmap(int pageno, fz_pixmap *pix);
fz_pixmap *GetPageImage(int pageno);
int PageCenterX(fz_pixmap *pix);
void BlitPageSlice(fz_pixmap *pix, int vy, int srcy, int h);
void GoToPage(int n, int new_pany);
void NormalizeScrollPos(void);
int ViewW(void);
void GetDocMetrics(int *total, int *pos);
void DrawScrollbar(void);
void ScrollToDocPos(int pos);
void ScrollbarMouse(void);
void ClearSelection(void);
int ClientToPagePoint(int mx, int my, fz_point *pt);
void CopySelectionToClipboard(void);
void ProcessPageMouse(void);
void FitPageWidth(void);
void ToggleSupersampling(void);
unsigned CpuBaseMhz(void);


// The MuPDF 1.19 glue (pdfapp.c) only calls back into winrepaint().
void wintitle(pdfapp_t *app, char *s, char param[])
{
	sprintf(Title,"%s - uPDF", strrchr(param, '/') + 1 );
}

void winclose(pdfapp_t *app)
{
	pdfapp_close(&gapp);
	exit(0);
}

void RunOpenApp(char name[])
{
	char cmd[250] = "*pdf* ";
	strcat(cmd, name);
	RunApp("/sys/lod", cmd);
}


void winrepaint(pdfapp_t *app)
{
	winblit(&gapp);
}


/* == continuous scroll: page cache =============================== */

void FlushPageCache(void)
{
	int i;
	for (i = 0; i < PAGE_CACHE_N; i++)
	{
		if (cache_pix[i]) fz_drop_pixmap(gapp.ctx, cache_pix[i]);
		cache_pix[i] = NULL;
		cache_no[i] = 0;
	}
}

// drop all cached pages if the view parameters have changed
void SyncCacheParams(void)
{
	if (cache_res != gapp.resolution || cache_rot != gapp.rotate || cache_gray != gapp.grayscale)
	{
		FlushPageCache();
		cache_res = gapp.resolution;
		cache_rot = gapp.rotate;
		cache_gray = gapp.grayscale;
	}
}

// keep a reference to an already rendered page (e.g. the outgoing current
// page on a flip) so scrolling back to it is instant
void StashPixmap(int pageno, fz_pixmap *pix)
{
	int i, slot = -1;

	if (!pix) return;
	SyncCacheParams(); // the pixmap is rendered with the current params

	for (i = 0; i < PAGE_CACHE_N; i++)
		if (cache_pix[i] && cache_no[i] == pageno) return; // already cached

	for (i = 0; i < PAGE_CACHE_N; i++)
		if (!cache_pix[i]) { slot = i; break; }

	if (slot < 0) // evict the entry farthest from this page
	{
		slot = 0;
		for (i = 1; i < PAGE_CACHE_N; i++)
			if (ABS(cache_no[i] - pageno) > ABS(cache_no[slot] - pageno)) slot = i;
		fz_drop_pixmap(gapp.ctx, cache_pix[slot]);
	}
	cache_pix[slot] = fz_keep_pixmap(gapp.ctx, pix);
	cache_no[slot] = pageno;
}

// pixmap of any page: the current one comes from pdfapp, neighbours
// are rendered on demand and cached
fz_pixmap *GetPageImage(int pageno)
{
	int i, slot;

	if (pageno == gapp.pageno) return gapp.image;
	if (pageno < 1 || pageno > gapp.pagecount) return NULL;

	SyncCacheParams();
	for (i = 0; i < PAGE_CACHE_N; i++)
		if (cache_pix[i] && cache_no[i] == pageno) return cache_pix[i];

	slot = -1;
	for (i = 0; i < PAGE_CACHE_N; i++)
		if (!cache_pix[i]) { slot = i; break; }
	if (slot < 0) // evict the entry farthest from the current page
	{
		slot = 0;
		for (i = 1; i < PAGE_CACHE_N; i++)
			if (ABS(cache_no[i] - gapp.pageno) > ABS(cache_no[slot] - gapp.pageno)) slot = i;
		fz_drop_pixmap(gapp.ctx, cache_pix[slot]);
		cache_pix[slot] = NULL;
	}
	cache_pix[slot] = pdfapp_renderpage(&gapp, pageno);
	cache_no[slot] = cache_pix[slot] ? pageno : 0;
	return cache_pix[slot];
}

/* == continuous scroll: view composition ========================= */

// width of the page area: the client width minus the scrollbar strip
int ViewW(void)
{
	return Form.cwidth - SCROLL_W;
}

int PageCenterX(fz_pixmap *pix)
{
	if (ViewW() > pix->w) return (ViewW() - pix->w) / 2;
	return 0;
}

// draw a horizontal slice of a page at viewport line vy:
// side backgrounds, 1px borders and the page image itself
void BlitPageSlice(fz_pixmap *pix, int vy, int srcy, int h)
{
	int wc = PageCenterX(pix);

	if (wc > 0)
	{
		kol_paint_bar(0, TOOLBAR_HEIGHT + vy, wc - 1, h, DOCUMENT_BG);
		kol_paint_bar(wc - 1, TOOLBAR_HEIGHT + vy, 1, h, DOCUMENT_BORDER);
		kol_paint_bar(wc + pix->w, TOOLBAR_HEIGHT + vy, 1, h, DOCUMENT_BORDER);
		if (ViewW() > wc + pix->w + 1)
			kol_paint_bar(wc + pix->w + 1, TOOLBAR_HEIGHT + vy,
				ViewW() - wc - pix->w - 1, h, DOCUMENT_BG);
	}

	kos_blit(wc + Form.cleft,
		Form.ctop + TOOLBAR_HEIGHT + vy,
		ViewW() - wc,
		h,
		0,
		srcy,
		pix->w,
		pix->h,
		pix->stride, // row stride (1.19 pixmap)
		pix->samples     // image
	);
}

// compose the viewport: tail of the current page, the gap, the head of
// the next page(s) - like the continuous page view in a word processor
void winblit(pdfapp_t *app)
{
	int vh, vy, page_top, k, wc;
	fz_pixmap *pix;

	if (do_not_blit) return;
	if (Form.cwidth == 0) return; // window is not drawn yet

	if (key_mode_enter_page_number==1) HandleNewPageNumber(0); else DrawPagination();
	if (key_mode_enter_zoom==1) HandleNewZoom(0); else DrawZoom();

	gapp.panx = 0;

	vh = Form.cheight - TOOLBAR_HEIGHT;
	if (vh <= 0) return;

	vy = 0;                  // how much of the viewport is painted already
	page_top = -gapp.pany;   // viewport y of the current page top
	k = gapp.pageno;

	while (vy < vh && (pix = GetPageImage(k)) != NULL)
	{
		int page_bot = page_top + pix->h;
		wc = PageCenterX(pix);

		// border above the page, on the last row of the gap
		if (page_top > 0 && page_top - 1 < vh)
			kol_paint_bar(wc ? wc - 1 : 0, TOOLBAR_HEIGHT + page_top - 1,
				MIN(pix->w + 2, ViewW()), 1, DOCUMENT_BORDER);

		if (page_bot > vy)
		{
			int h = MIN(page_bot, vh) - vy;
			BlitPageSlice(pix, vy, vy - page_top, h);
			vy += h;
		}
		if (vy >= vh) break;

		page_top = page_bot;

		if (k < gapp.pagecount) // inter-page gap
		{
			int gap_bot = page_bot + PAGE_GAP;
			if (gap_bot > vy)
			{
				kol_paint_bar(0, TOOLBAR_HEIGHT + vy, ViewW(),
					MIN(gap_bot, vh) - vy, DOCUMENT_BG);
				// border under the page
				if (page_bot >= 0 && page_bot < vh)
					kol_paint_bar(wc ? wc - 1 : 0, TOOLBAR_HEIGHT + page_bot,
						MIN(pix->w + 2, ViewW()), 1, DOCUMENT_BORDER);
				vy = MIN(gap_bot, vh);
			}
			page_top = gap_bot;
		}

		k++;
	}

	if (vy < vh) // area below the last page
		kol_paint_bar(0, TOOLBAR_HEIGHT + vy, ViewW(), vh - vy, DOCUMENT_BG);

	DrawScrollbar();
}

/* == vertical scrollbar (WebView / box_lib type 2 style) ========= */

// document height and current position in pixels; pages are assumed
// to be as tall as the current one (true for typical PDFs)
void GetDocMetrics(int *total, int *pos)
{
	int unit = gapp.image->h + PAGE_GAP;
	*total = gapp.pagecount * unit - PAGE_GAP;
	*pos = (gapp.pageno - 1) * unit + gapp.pany;
}

void DrawScrollbar(void)
{
	int VH = Form.cheight - TOOLBAR_HEIGHT;
	int x0 = ViewW();
	int total, pos, run, pos2;

	GetDocMetrics(&total, &pos);

	kol_paint_bar(x0, TOOLBAR_HEIGHT, SCROLL_W, VH, SB_BG_COL); // track
	if (total <= VH)
	{
		sb_active = 0;
		return;
	}

	// box_lib formulas: runner size and offset inside the track
	run = (int)((long long)VH * VH / total);
	if (run < SB_MIN_RUN) run = SB_MIN_RUN;
	if (run > VH) run = VH;
	pos2 = (int)((long long)(VH - run) * pos / (total - VH));
	if (pos2 > VH - run) pos2 = VH - run;
	if (pos2 < 0) pos2 = 0;

	// flat runner: 1px inset on the left, 1px track-colored lines
	// on top and bottom (line_col == bckg_col in the WebView scheme)
	kol_paint_bar(x0 + 1, TOOLBAR_HEIGHT + pos2 + 1, SCROLL_W - 1, run - 2, SB_FRONT_COL);

	sb_active = 1;
	sb_run_y = TOOLBAR_HEIGHT + pos2;
	sb_run_h = run;
}

// jump so that the viewport top lands at the given document position
void ScrollToDocPos(int pos)
{
	int unit = gapp.image->h + PAGE_GAP;
	int page;

	ClearSelection();
	if (pos < 0) pos = 0;
	page = pos / unit + 1;
	if (page > gapp.pagecount) page = gapp.pagecount;
	if (page != gapp.pageno)
		GoToPage(page, pos - (page - 1) * unit);
	else
		gapp.pany = pos - (page - 1) * unit;
	NormalizeScrollPos();
	winblit(&gapp);
}

void ScrollbarMouse(void)
{
	int mp, mx, my, lmb, VH, total, pos;

	// fn 37/1 returns coordinates relative to the client area,
	// signed 16-bit (negative when the cursor is above/left of it)
	mp = kol_mouse_posw();
	mx = (short)(mp >> 16);
	my = (short)(mp & 0xFFFF);
	lmb = kol_mouse_btn() & 1;
	VH = Form.cheight - TOOLBAR_HEIGHT;

	if (!sb_active || !lmb)
	{
		sb_drag = 0;
		sb_lmb_prev = lmb;
		return;
	}

	GetDocMetrics(&total, &pos);

	if (sb_drag)
	{
		// runner follows the pointer
		int pos2 = my - sb_drag_off - TOOLBAR_HEIGHT;
		int space = VH - sb_run_h;
		if (space > 0)
		{
			int newpos = (int)((long long)pos2 * (total - VH) / space);
			if (newpos > total - VH) newpos = total - VH;
			ScrollToDocPos(newpos);
		}
	}
	else if (!sb_lmb_prev
		&& mx >= ViewW() && mx < Form.cwidth
		&& my >= TOOLBAR_HEIGHT && my < Form.cheight)
	{
		if (my >= sb_run_y && my < sb_run_y + sb_run_h)
		{
			sb_drag = 1;
			sb_drag_off = my - sb_run_y;
		}
		else if (my < sb_run_y) ScrollToDocPos(pos - VH); // page up
		else                    ScrollToDocPos(pos + VH); // page down
	}
	sb_lmb_prev = lmb;
}

/* == text selection & drag-to-pan =============================== */

// restore the pixels under the current highlight (if any); the caller
// must repaint. Called before any scroll/zoom/rotate/page flip so the
// pixmap is never stashed or re-rendered with inverted pixels in it.
void ClearSelection(void)
{
	if (sel_has)
	{
		pdfapp_invertselection(&gapp);
		sel_has = 0;
	}
	sel_dragging = 0;
}

// map a client point to a page-space point (what MuPDF's selection API
// wants). Coordinates are clamped to the page; the return value tells
// whether the original point was actually within the page rectangle.
int ClientToPagePoint(int mx, int my, fz_point *pt)
{
	int wc = PageCenterX(gapp.image);
	int px = mx - wc;                          // pixel col within the page
	int py = my - TOOLBAR_HEIGHT + gapp.pany;  // pixel row within the page
	int inside = (px >= 0 && px < gapp.image->w && py >= 0 && py < gapp.image->h);

	if (px < 0) px = 0; else if (px > gapp.image->w) px = gapp.image->w;
	if (py < 0) py = 0; else if (py > gapp.image->h) py = gapp.image->h;
	*pt = pdfapp_devtopage(&gapp, px, py);
	return inside;
}

// copy the selected text (UTF-8, from MuPDF) to the KolibriOS clipboard
// (fn 54: 12-byte header {size, type=text, encoding=UTF-8} + data)
void CopySelectionToClipboard(void)
{
	static char clip[12 + 65536];
	char *text;
	int n, total;

	text = pdfapp_copyselection(&gapp);
	if (!text)
		return;
	n = strlen(text);
	if (n > 65535) n = 65535;
	if (n == 0) { fz_free(gapp.ctx, text); return; }

	total = 12 + n;
	// 12-byte header, little-endian, written byte-wise to avoid aliasing
	clip[0] = total; clip[1] = total >> 8; clip[2] = total >> 16; clip[3] = total >> 24;
	clip[4] = clip[5] = clip[6] = clip[7] = 0;  // type: text
	clip[8] = clip[9] = clip[10] = clip[11] = 0; // encoding: 0 = UTF-8
	memcpy(clip + 12, text, n);
	kol_clip_set(total, clip);
	fz_free(gapp.ctx, text);
}

// left button = select text, right button = grab & drag the page.
// Called on every mouse event unless the scrollbar owns the drag.
void ProcessPageMouse(void)
{
	int mp = kol_mouse_posw();
	int mx = (short)(mp >> 16);
	int my = (short)(mp & 0xFFFF);
	int btn = kol_mouse_btn();
	int left = btn & 1, right = btn & 2;

	/* --- right button: grab & drag (hand tool) --- */
	if (right)
	{
		if (!pan_dragging)
		{
			if (my >= TOOLBAR_HEIGHT && mx >= 0 && mx < ViewW())
			{
				ClearSelection();
				winblit(&gapp);
				pan_dragging = 1;
				pan_last_my = my;
			}
		}
		else
		{
			int d = my - pan_last_my;
			pan_last_my = my;
			if (d)
			{
				gapp.pany -= d;       // content follows the cursor
				NormalizeScrollPos();
				winblit(&gapp);
			}
		}
		return;
	}
	pan_dragging = 0;

	/* --- left button: text selection --- */
	if (left && mx >= 0 && mx < ViewW() && my >= TOOLBAR_HEIGHT)
	{
		fz_point pt;
		int inside = ClientToPagePoint(mx, my, &pt);
		if (!sel_dragging)
		{
			if (!inside) return;      // start only on the page itself
			ClearSelection();
			winblit(&gapp);
			sel_anchor = pt;
			sel_dragging = 1;
		}
		else
		{
			if (sel_has) pdfapp_invertselection(&gapp); // lift old highlight
			gapp.sel_a = sel_anchor;
			gapp.sel_b = pt;
			gapp.sel_valid = 1;
			pdfapp_invertselection(&gapp);
			sel_has = 1;
			winblit(&gapp);
		}
	}
	else if (sel_dragging) // left released
	{
		sel_dragging = 0;
		if (sel_has) CopySelectionToClipboard();
	}
}

/* == continuous scroll: position management ====================== */

// switch the current page keeping the given scroll offset;
// reuses a cached pixmap when possible to avoid re-rendering
void GoToPage(int n, int new_pany)
{
	fz_pixmap *cached = NULL;
	int i;

	// keep the outgoing page: scrolling back to it will be instant
	StashPixmap(gapp.pageno, gapp.image);

	for (i = 0; i < PAGE_CACHE_N; i++)
		if (cache_pix[i] && cache_no[i] == n)
		{
			cached = cache_pix[i];
			cache_pix[i] = NULL;
			cache_no[i] = 0;
			break;
		}

	gapp.pageno = n;
	gapp.pany = new_pany;

	if (cached)
	{
		if (gapp.image) fz_drop_pixmap(gapp.ctx, gapp.image);
		gapp.image = cached;
		pdfapp_showpage(&gapp, 1, 0, 0); // reload page_list/text/links only
	}
	else
	{
		pdfapp_showpage(&gapp, 1, 1, 0); // full render, pany is kept
	}
}

// bring (pageno, pany) back into range, flipping pages when the
// viewport top has crossed a page boundary
void NormalizeScrollPos(void)
{
	int maxs, rem;

	// crossed the gap downwards: the next page becomes current
	while (gapp.pageno < gapp.pagecount && gapp.pany >= gapp.image->h + PAGE_GAP)
		GoToPage(gapp.pageno + 1, gapp.pany - gapp.image->h - PAGE_GAP);

	// scrolled above the page top: the previous page becomes current
	while (gapp.pany < 0)
	{
		if (gapp.pageno <= 1) { gapp.pany = 0; break; }
		rem = gapp.pany;
		GoToPage(gapp.pageno - 1, 0);
		gapp.pany = rem + gapp.image->h + PAGE_GAP;
	}

	// do not scroll past the bottom of the last page
	if (gapp.pageno == gapp.pagecount)
	{
		maxs = gapp.image->h - (Form.cheight - TOOLBAR_HEIGHT);
		if (maxs < 0) maxs = 0;
		if (gapp.pany > maxs) gapp.pany = maxs;
	}
	if (gapp.pany < 0) gapp.pany = 0;
}


void GetNewPageNumber(void)
{
	key_mode_enter_zoom = 0;          // the two input boxes are mutually exclusive
	new_page_number = gapp.pageno;
	key_mode_enter_page_number = 1;
	winblit(&gapp);                  // repaint the page (drop any highlight) + draw the box
}

void HandleNewPageNumber(unsigned char key)
{
	char label_new_page[8];

	if ((key >= '0') && (key <= '9')) 
	{
		new_page_number = new_page_number * 10 + key - '0';
	}
	if (key == ASCII_KEY_BS)
	{
		new_page_number /= 10;
	}
	if (key == ASCII_KEY_ENTER)
	{
		ApplyNewPageNumber();
		return;
	}
	if (key==ASCII_KEY_ESC)
	{
		key_mode_enter_page_number = 0;
		DrawMainWindow();
		return;
	}

	itoa(new_page_number, label_new_page, 10);
	strcat(label_new_page, "_");
	kol_paint_bar(show_area_x,  6, show_area_w, 22, 0xFDF88E);
	kos_text(show_area_x + show_area_w/2 - strlen(label_new_page)*6/2, 14, 0x000000, label_new_page, strlen(label_new_page));

	if (new_page_number > gapp.pagecount) ApplyNewPageNumber();
}

void ApplyNewPageNumber(void)
{
	key_mode_enter_page_number = 0;
	gapp.pageno = new_page_number -1;
	pdfapp_onkey(&gapp, ']');
}

void DrawPagination(void)
{
	char pages_display[12];
	kol_paint_bar(show_area_x,  6, show_area_w, 22, 0xF4F4F4);
	sprintf (pages_display, "%d/%d", gapp.pageno, gapp.pagecount);
	kos_text(show_area_x + show_area_w/2 - strlen(pages_display)*6/2, 14, 0x000000, pages_display, strlen(pages_display));
}

// current zoom as a percentage (72 dpi == 100%)
int ZoomPercent(void)
{
	return (int)(gapp.resolution / 72.0f * 100.0f + 0.5f);
}

void DrawZoom(void)
{
	char s[12];
	kol_paint_bar(zoom_area_x, 6, ZOOM_AREA_W, 22, 0xF4F4F4);
	sprintf(s, "%d%%", ZoomPercent());
	kos_text(zoom_area_x + ZOOM_AREA_W/2 - strlen(s)*6/2, 14, 0x000000, s, strlen(s));
}

void GetNewZoom(void)
{
	key_mode_enter_page_number = 0;  // the two input boxes are mutually exclusive
	new_zoom = 0;                    // type the wanted zoom from scratch
	key_mode_enter_zoom = 1;
	winblit(&gapp);                  // repaint the page (drop any highlight) + draw the box
}

void HandleNewZoom(unsigned char key)
{
	char lbl[12];

	if (key >= '0' && key <= '9')
		new_zoom = new_zoom * 10 + key - '0';
	if (key == ASCII_KEY_BS)
		new_zoom /= 10;
	if (key == ASCII_KEY_ENTER)
	{
		if (new_zoom == 0)           // nothing typed: cancel, like Esc
		{
			key_mode_enter_zoom = 0;
			DrawMainWindow();
			return;
		}
		ApplyNewZoom();
		return;
	}
	if (key == ASCII_KEY_ESC)
	{
		key_mode_enter_zoom = 0;
		DrawMainWindow();
		return;
	}

	itoa(new_zoom, lbl, 10);
	strcat(lbl, "_");
	kol_paint_bar(zoom_area_x, 6, ZOOM_AREA_W, 22, 0xFDF88E);
	kos_text(zoom_area_x + ZOOM_AREA_W/2 - strlen(lbl)*6/2, 14, 0x000000, lbl, strlen(lbl));

	if (new_zoom > 1600) ApplyNewZoom();
}

void ApplyNewZoom(void)
{
	int oldh;
	key_mode_enter_zoom = 0;
	if (new_zoom < 10)   new_zoom = 10;
	if (new_zoom > 1600) new_zoom = 1600;
	oldh = (gapp.image && gapp.image->h > 0) ? gapp.image->h : 1;
	gapp.resolution = new_zoom / 100.0f * 72.0f;
	do_not_blit = 1;
	pdfapp_showpage(&gapp, 0, 1, 0);           // re-render at the new zoom
	do_not_blit = 0;
	if (gapp.image)
	{
		gapp.pany = (int)((long long)gapp.pany * gapp.image->h / oldh);
		NormalizeScrollPos();
	}
	DrawMainWindow();
}

void DrawToolbarButton(int x, char image_id)
{
	kol_btn_define(x, 5, 26-1, 24-1, 10 + image_id + BT_HIDE, 0);
	kol_paint_image(x, 5, 26, 24, image_id * 24 * 26 * 3 + toolbar_image);
}

void DrawMainWindow(void)
{
	kol_paint_bar(0, 0, Form.cwidth, TOOLBAR_HEIGHT - 1, 0xe1e1e1); // bar on the top (buttons holder)
	kol_paint_bar(0, TOOLBAR_HEIGHT - 1, Form.cwidth, 1, 0x7F7F7F);
	// left: open folder, info/help, rotate left/right
	DrawToolbarButton(8, 0);    //open_folder
	DrawToolbarButton(42, 3);   //info / help
	DrawToolbarButton(76, 6);   //rotate left
	DrawToolbarButton(101, 7);  //rotate right

	// center: page navigation  < N/M >
	show_area_x = (Form.cwidth - (26 + show_area_w + 26)) / 2 + 26;
	DrawToolbarButton(show_area_x - 26, 4); //prev page (touches box left)
	kol_btn_define(show_area_x - 1, 5, show_area_w + 1, 23, 20 + BT_HIDE, 0xA4A4A4);
	kol_paint_bar(show_area_x, 5, show_area_w, 1, 0xA4A4A4);
	kol_paint_bar(show_area_x, 28, show_area_w, 1, 0xA4A4A4);
	DrawToolbarButton(show_area_x + show_area_w, 5); //next page (touches box right)

	// right: the zoom block ( - [100%] + ), edge-anchored
	zoom_area_x = Form.cwidth - 8 - 26 - ZOOM_AREA_W;   // box left; zoom + is rightmost
	DrawToolbarButton(zoom_area_x - 26, 1);           //magnify - (touches box left)
	kol_btn_define(zoom_area_x - 1, 5, ZOOM_AREA_W + 1, 23, 21 + BT_HIDE, 0xA4A4A4);
	kol_paint_bar(zoom_area_x, 5, ZOOM_AREA_W, 1, 0xA4A4A4);
	kol_paint_bar(zoom_area_x, 28, ZOOM_AREA_W, 1, 0xA4A4A4);
	DrawToolbarButton(zoom_area_x + ZOOM_AREA_W, 2);  //magnify + (touches box right)
	if (fit_pending && gapp.image)   // open the document fitted to width
	{
		FitPageWidth();
		fit_pending = 0;
	}
	ClearSelection();
	NormalizeScrollPos();
	winblit(&gapp);
}


/* Actions */

void PageScrollDown(void)
{
	gapp.pany += SCROLL_STEP;
	NormalizeScrollPos();
	winblit(&gapp);
}


void PageScrollUp(void)
{
	gapp.pany -= SCROLL_STEP; 
	NormalizeScrollPos();
	winblit(&gapp);
}

void PageScroll(signed int delta)
{
	ClearSelection();
	gapp.pany += delta;
	NormalizeScrollPos();
	winblit(&gapp);
}


void RunApp(char app[], char param[])
{
	kol_struct70 r;
	r.p00 = 7;
	r.p04 = 0;
	r.p08 = param;
	r.p12 = 0;
	r.p16 = 0;
	r.p20 = 0;
	r.p21 = app;
	kol_file_70(&r);
}

// Scale the page so it fills the view width minus a small side margin,
// re-rendering the current page. Neighbours re-render lazily (the cache
// is keyed on resolution). Called once on open and bound to the 'w' key.
void FitPageWidth(void)
{
	int target;
	if (!gapp.image || gapp.image->w <= 0 || Form.cwidth == 0)
		return;
	target = ViewW() - 2 * SIDE_MARGIN;
	if (target < 50)
		return;
	gapp.resolution = gapp.resolution * (float)target / gapp.image->w;
	if (gapp.resolution < 18)   gapp.resolution = 18;
	if (gapp.resolution > 1200) gapp.resolution = 1200;
	pdfapp_snapzoom(&gapp);          // 96%-ish fit -> clean 100%
	pdfapp_showpage(&gapp, 0, 1, 0); /* re-render current page, no repaint */
}

// toggle 2x supersampling (sharper text) on/off at runtime; re-renders
// the current page and drops the cache (neighbours were at the old factor)
void ToggleSupersampling(void)
{
	char msg[96];
	long px;
	gapp.ss_factor = (gapp.ss_factor > 1) ? 1 : 2;
	FlushPageCache();
	do_not_blit = 1;
	pdfapp_showpage(&gapp, 0, 1, 0);
	do_not_blit = 0;
	NormalizeScrollPos();
	winblit(&gapp);
	// diagnostic: gapp.image is the 1x display pixmap; if its area >= the SS
	// guard (2.5M px) supersampling is skipped even when ss_factor==2
	px = (long)gapp.image->w * gapp.image->h;
	sprintf(msg, "uPDF: SS ss_factor=%d, page %dx%d=%ld px %s\n",
		gapp.ss_factor, gapp.image->w, gapp.image->h, px,
		(gapp.ss_factor > 1 && px < 2500000) ? "-> SS ACTIVE" : "-> SS skipped");
	kol_board_puts(msg);
}

void PageZoomIn(void)
{
	int oldh = gapp.image->h;
	ClearSelection();
	do_not_blit = 1;
	pdfapp_onkey(&gapp, '+');
	do_not_blit = 0;
	// keep the same relative position on the page
	gapp.pany = (int)((long long)gapp.pany * gapp.image->h / oldh);
	NormalizeScrollPos();
	winblit(&gapp);
}


void PageZoomOut(void)
{
	int oldh = gapp.image->h;
	ClearSelection();
	do_not_blit = 1;
	pdfapp_onkey(&gapp, '-');
	do_not_blit = 0;
	gapp.pany = (int)((long long)gapp.pany * gapp.image->h / oldh);
	NormalizeScrollPos();
	winblit(&gapp);
}

void PageRotateLeft(void)
{
	ClearSelection();
	do_not_blit = 1;
	pdfapp_onkey(&gapp, 'L');
	do_not_blit = 0;
	gapp.pany = 0;
	NormalizeScrollPos();
	winblit(&gapp);
}

void PageRotateRight(void)
{
	ClearSelection();
	do_not_blit = 1;
	pdfapp_onkey(&gapp, 'R');
	do_not_blit = 0;
	gapp.pany = 0;
	NormalizeScrollPos();
	winblit(&gapp);
}

// CPU base frequency in MHz. sysfn 18.5 returns the clock modulo 2^32 and
// so misreads CPUs faster than ~4.29 GHz as slow ones. CPUID leaf 16h
// (Intel Skylake+, i.e. exactly the CPUs that can exceed 4.29 GHz) reports
// the base frequency in MHz directly, so prefer it; CPUs old enough to
// lack the leaf cannot wrap 18.5 anyway.
unsigned CpuBaseMhz(void)
{
	unsigned a, b, c, d;
	asm volatile ("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0));
	if (a >= 0x16)
	{
		asm volatile ("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0x16), "c"(0));
		if (a)
			return a;
	}
	return kol_system_cpufreq() / 1000000;
}

int main (int argc, char* argv[])
{
	char ii, mouse_wheels_state;
	
	// argv without spaces
	char full_argv[1024];
	for (int i = 1; i<argc; i++) {
		if (i != 1) strcat(full_argv, " ");
		strcat(full_argv, argv[i]);
	}
	
	if (argc == 1) {
		RunOpenApp(argv[0]);
		exit(0);
	}

	char buf[128];
	int resolution = 72;
	int pageno = 1;
	pdfapp_init(&gapp);
	gapp.scrw = 600;
	gapp.scrh = 400;
	gapp.resolution = resolution;
	gapp.pageno = pageno;
	pdfapp_open(&gapp, full_argv, 0, 0);
	wintitle(&gapp, 0, full_argv);

	// Supersampling (2x render for sharper text) is ~4x the work per page.
	// On a slow CPU (< 1 GHz: PI/PII/PIII era) it would crawl, so switch it
	// off automatically. A 0/bogus reading leaves the default on (assume a
	// capable machine).
	{
		unsigned mhz = CpuBaseMhz();
		char msg[64];
		if (mhz > 0 && mhz < 1000)
			gapp.ss_factor = 1;
		sprintf(msg, "uPDF: CPU %u MHz, supersampling %s\n",
			mhz, gapp.ss_factor > 1 ? "on" : "off");
		kol_board_puts(msg);
	}
	
	int butt, key, scan, ascii, screen_max_x, screen_max_y;
	kos_screen_max(&screen_max_x, &screen_max_y);
	// mouse events only for the active window, as WebView does (EVM_MOUSE_FILTER)
	kol_event_mask(EVENT_REDRAW+EVENT_KEY+EVENT_BUTTON+EVENT_MOUSE_CHANGE+EVENT_MOUSE_WINDOW_MASK);

	for(;;)
	{
		switch(kol_event_wait())
		{
			case evReDraw:
				// gapp.shrinkwrap = 2;
				kol_paint_start();
				kol_wnd_define(
					(screen_max_x - (screen_max_y*8/10 + 20)) / 2 -15+kos_random(30),
					screen_max_y/20+kos_random(20),
					screen_max_y*8/10 + 20,
					screen_max_y*9/10,
					0x73000000, 0x800000FF, Title
				);
				kol_paint_end();
				kol_process_info(-1, (char*)&Form);
				
				if (Form.window_state & 4) continue; // if Rolled-up
				
				// Minimal size (640x480)
				if ((Form.width < 639) && (screen_max_x >= 639)) kol_wnd_change(-1, -1, 639, -1);
				if ((Form.height < 479) && (screen_max_y >= 479)) kol_wnd_change(-1, -1, -1, 479);
				
				DrawMainWindow();
				break;

			case evKey:
				key = kos_get_key_full();
				if (key & 0xFF) break;           // no key in buffer
				scan  = (key >> 16) & 0xFF;       // layout-independent scancode
				ascii = (key >> 8) & 0xFF;        // ASCII, for digit entry only

				// page-number / zoom entry needs literal digits/enter/bs/esc
				if (key_mode_enter_page_number)
				{
					HandleNewPageNumber(ascii);
					break;
				}
				if (key_mode_enter_zoom)
				{
					HandleNewZoom(ascii);
					break;
				}

				ClearSelection(); // any hotkey drops the current selection

				switch (scan)
				{
					case SC_ESC:    DrawMainWindow(); break; // close help
					case SC_PGDN:   PageScroll(Form.cheight - TOOLBAR_HEIGHT - SCROLL_STEP); break;
					case SC_PGUP:   PageScroll(-Form.cheight + TOOLBAR_HEIGHT + SCROLL_STEP); break;
					case SC_HOME:   pdfapp_onkey(&gapp, 'g'); break; // first page
					case SC_END:    pdfapp_onkey(&gapp, 'G'); break; // last page
					case SC_G:      pdfapp_onkey(&gapp, 'c'); break; // grayscale on/off
					case SC_W:      FitPageWidth(); NormalizeScrollPos(); winblit(&gapp); break; // fit width
					case SC_S:      ToggleSupersampling(); break; // sharper text on/off
					case SC_L:
					case SC_LBRACKET: PageRotateLeft(); break;
					case SC_R:
					case SC_RBRACKET: PageRotateRight(); break;
					case SC_DOWN:   PageScroll(SCROLL_STEP); break;
					case SC_UP:     PageScroll(-SCROLL_STEP); break;
					case SC_MINUS:
					case SC_NUM_MINUS: PageZoomOut(); break;
					case SC_EQUAL:
					case SC_NUM_PLUS:  PageZoomIn(); break;
				}
				break;

			case evButton:
				butt = kol_btn_get();
				if(butt==1) exit(0);
				if (butt!=13) ClearSelection(); // any toolbar action but help drops selection
				if(butt==10) RunOpenApp(argv[0]);
				if(butt==11) PageZoomOut(); //magnify -
				if(butt==12) PageZoomIn(); //magnify +
				if(butt==13) //show help
				{
					kol_paint_bar(0, TOOLBAR_HEIGHT, Form.cwidth, Form.cheight - TOOLBAR_HEIGHT, 0xF2F2F2);
					kos_text(20, TOOLBAR_HEIGHT + 20, 0x81000000, "uPDF for KolibriOS v2.1", 0);
					kos_text(21, TOOLBAR_HEIGHT + 20, 0x81000000, "uPDF for KolibriOS v2.1", 0);
					for (ii=0; help[ii]!=0; ii++) {
						kos_text(20, TOOLBAR_HEIGHT + 60 + ii * 20, 0x90000000, help[ii], 0);
					}
				}
				if(butt==14) pdfapp_onkey(&gapp, '['); //previous page
				if(butt==15) pdfapp_onkey(&gapp, ']'); //next page
				if(butt==16) PageRotateLeft();
				if(butt==17) PageRotateRight();
				if(butt==20) GetNewPageNumber();
				if(butt==21) GetNewZoom();
				break;

			case evMouse:
				if (mouse_wheels_state = kos_get_mouse_wheels())
				{
					if (mouse_wheels_state>0) PageScroll(SCROLL_STEP);
					if (mouse_wheels_state<0) PageScroll(-SCROLL_STEP);
				}
				ScrollbarMouse();
				if (!sb_drag) ProcessPageMouse(); // page area: select / drag
				break;
		}
	}
}
