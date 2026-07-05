/*==== INCLUDES ====*/

#include "fitz.h"
#include "mupdf.h"
#include "pdfapp.h"
#include "icons/allbtns.h"
#include "kolibri.h"


/*==== DATA ====*/

static char Title[1024] = "uPDF";
static pdfapp_t gapp;
char debugstr[256];
char do_not_blit=0;

#define TOOLBAR_HEIGHT 34
struct proc_info Form;

#define DOCUMENT_BORDER 0x979797
#define DOCUMENT_BG 0xABABAB

#define SCROLL_STEP 50    // pixels moved per scroll gesture (arrow key / wheel notch)
#define PAGE_GAP    14    // gray gap between pages, like in a word processor

// keyboard scancodes (layout-independent, from fn 2 bits 16-23)
#define SC_ESC        0x01
#define SC_MINUS      0x0C  // main row -
#define SC_EQUAL      0x0D  // main row =/+
#define SC_R          0x13
#define SC_LBRACKET   0x1A  // [
#define SC_RBRACKET   0x1B  // ]
#define SC_G          0x22
#define SC_L          0x26
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
static int  sel_x0, sel_y0;     // selection anchor, device coordinates
static char pan_dragging;       // right button held, panning the page
static int  pan_last_my;        // last pointer y while panning

// look-ahead cache of rendered neighbour pages for continuous scrolling
#define PAGE_CACHE_N 3
static fz_pixmap *cache_pix[PAGE_CACHE_N];
static int cache_no[PAGE_CACHE_N];
static int cache_res = -1, cache_rot = -1, cache_gray = -1;

short show_area_w = 65;
short show_area_x;

char key_mode_enter_page_number;
int new_page_number;

const char *help[] = {
	"Keys:",
	" ",
	"PageUp     - go to previous page",
	"PageDown   - go to next page",
	"Home       - go to first page",
	"End        - go to last page",
	"Down arrow - scroll down",
	"Up arrow   - scroll up",
	"+/-        - zoom in/out",
	"[ or l     - rotate page 90 deg to the left",
	"] or r     - rotate page 90 deg to the right",
	"g          - grayscale on/off",
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
int ClientToPageDev(int mx, int my, int *dx, int *dy);
void CopySelectionToClipboard(void);
void ProcessPageMouse(void);


// not implemented yet
void wincursor(pdfapp_t *app, int curs) { }
void winhelp(pdfapp_t *app) { }
void winresize(pdfapp_t *app, int w, int h) { }
void windocopy(pdfapp_t *app) { }
void winopenuri(pdfapp_t *app, char *buf) { }
void winrepaintsearch(pdfapp_t *app) { }


void winwarn(pdfapp_t *app, char *msg)
{
	fprintf(stderr, "mupdf: %s\n", msg);
}


void winerror(pdfapp_t *app, fz_error error)
{
	fz_catch(error, "aborting");
	exit(1);
}


char *winpassword(pdfapp_t *app, char *filename)
{
	char *r = "";
	return r;
	random();
}


void wintitle(pdfapp_t *app, char *s, char param[])
{
	sprintf(Title,"%s - uPDF", strrchr(param, '/') + 1 );
}


void winreloadfile(pdfapp_t *app)
{
	//pdfapp_close(app);
	//pdfapp_open(app, filename, 0, 1);
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
		if (cache_pix[i]) fz_drop_pixmap(cache_pix[i]);
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
		fz_drop_pixmap(cache_pix[slot]);
	}
	cache_pix[slot] = fz_keep_pixmap(pix);
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
		fz_drop_pixmap(cache_pix[slot]);
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
		pix->w * pix->n, // stride
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

// map a client point to device coordinates inside the current page.
// Coordinates are clamped to the page; the return value tells whether
// the original point was actually within the page rectangle.
int ClientToPageDev(int mx, int my, int *dx, int *dy)
{
	int wc = PageCenterX(gapp.image);
	int px = mx - wc;                          // pixel x within the page
	int py = my - TOOLBAR_HEIGHT + gapp.pany;  // pixel y within the page
	int inside = (px >= 0 && px < gapp.image->w && py >= 0 && py < gapp.image->h);

	if (px < 0) px = 0; else if (px > gapp.image->w) px = gapp.image->w;
	if (py < 0) py = 0; else if (py > gapp.image->h) py = gapp.image->h;
	*dx = px + gapp.image->x;
	*dy = py + gapp.image->y;
	return inside;
}

// UTF-8 encode a single code point (<= 0xFFFF); returns bytes written
static int utf8_put(unsigned c, char *out)
{
	if (c < 0x80) { out[0] = c; return 1; }
	if (c < 0x800) { out[0] = 0xC0 | (c >> 6); out[1] = 0x80 | (c & 0x3F); return 2; }
	out[0] = 0xE0 | (c >> 12);
	out[1] = 0x80 | ((c >> 6) & 0x3F);
	out[2] = 0x80 | (c & 0x3F);
	return 3;
}

// extract the selected text and push it to the KolibriOS clipboard
// (fn 54: 12-byte header {size, type=text, encoding=UTF-8} + data)
void CopySelectionToClipboard(void)
{
	static unsigned short ucs[4096];
	static char clip[12 + 4096 * 3 + 1];
	int i, n = 0;

	int total;

	pdfapp_oncopy(&gapp, ucs, 4096);
	for (i = 0; ucs[i]; i++)
		n += utf8_put(ucs[i], clip + 12 + n);
	if (n == 0)
		return;

	total = 12 + n;
	// 12-byte header, little-endian, written byte-wise to avoid aliasing
	clip[0] = total; clip[1] = total >> 8; clip[2] = total >> 16; clip[3] = total >> 24;
	clip[4] = clip[5] = clip[6] = clip[7] = 0;  // type: text
	clip[8] = clip[9] = clip[10] = clip[11] = 0; // encoding: 0 = UTF-8
	kol_clip_set(total, clip);
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
	int dx, dy;

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
		int inside = ClientToPageDev(mx, my, &dx, &dy);
		if (!sel_dragging)
		{
			if (!inside) return;      // start only on the page itself
			ClearSelection();
			winblit(&gapp);
			sel_x0 = dx;
			sel_y0 = dy;
			sel_dragging = 1;
		}
		else
		{
			if (sel_has) pdfapp_invertselection(&gapp); // lift old highlight
			gapp.selr.x0 = MIN(sel_x0, dx);
			gapp.selr.x1 = MAX(sel_x0, dx);
			gapp.selr.y0 = MIN(sel_y0, dy);
			gapp.selr.y1 = MAX(sel_y0, dy);
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
		if (gapp.image) fz_drop_pixmap(gapp.image);
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
	new_page_number = gapp.pageno;
	key_mode_enter_page_number = 1;
	HandleNewPageNumber(0);
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

void DrawToolbarButton(int x, char image_id)
{
	kol_btn_define(x, 5, 26-1, 24-1, 10 + image_id + BT_HIDE, 0);
	kol_paint_image(x, 5, 26, 24, image_id * 24 * 26 * 3 + toolbar_image);
}

void DrawMainWindow(void)
{
	kol_paint_bar(0, 0, Form.cwidth, TOOLBAR_HEIGHT - 1, 0xe1e1e1); // bar on the top (buttons holder)
	kol_paint_bar(0, TOOLBAR_HEIGHT - 1, Form.cwidth, 1, 0x7F7F7F);
	DrawToolbarButton(8,0); //open_folder
	DrawToolbarButton(42,1); //magnify -
	DrawToolbarButton(67,2);  //magnify +
	DrawToolbarButton(101,6); //rotate left
	DrawToolbarButton(126,7); //rotate right
	DrawToolbarButton(Form.cwidth - 160,3); //show help
	show_area_x = Form.cwidth - show_area_w - 34;
	DrawToolbarButton(show_area_x - 26,4); //prev page
	DrawToolbarButton(show_area_x + show_area_w,5); //nex page
	kol_btn_define(show_area_x-1,  5, show_area_w+1, 23, 20 + BT_HIDE, 0xA4A4A4);
	kol_paint_bar(show_area_x,  5, show_area_w, 1, 0xA4A4A4);
	kol_paint_bar(show_area_x, 28, show_area_w, 1, 0xA4A4A4);
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
	fz_accelerate();
	pdfapp_init(&gapp);
	gapp.scrw = 600;
	gapp.scrh = 400;
	gapp.resolution = resolution;
	gapp.pageno = pageno;
	pdfapp_open(&gapp, full_argv, 0, 0);
	wintitle(&gapp, 0, full_argv);
	
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
				kol_wnd_define(screen_max_x / 2 - 350-50+kos_random(50), 
				screen_max_y / 2 - 300-50+kos_random(50), 
				700, 600, 0x73000000, 0x800000FF, Title);
				kol_paint_end();
				kol_process_info(-1, (char*)&Form);
				
				if (Form.window_state & 4) continue; // if Rolled-up
				
				// Minimal size (640x480)
				if (Form.width < 640) kol_wnd_change(-1, -1, 640, -1);
				if (Form.height < 480)  kol_wnd_change(-1, -1, -1, 480);
				
				DrawMainWindow();
				break;

			case evKey:
				key = kos_get_key_full();
				if (key & 0xFF) break;           // no key in buffer
				scan  = (key >> 16) & 0xFF;       // layout-independent scancode
				ascii = (key >> 8) & 0xFF;        // ASCII, for digit entry only

				// page-number entry still needs literal digits/enter/bs/esc
				if (key_mode_enter_page_number)
				{
					HandleNewPageNumber(ascii);
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
					kos_text(20, TOOLBAR_HEIGHT + 20, 0x90000000, "uPDF for KolibriOS v1.5", 0);
					kos_text(21, TOOLBAR_HEIGHT + 20, 0x90000000, "uPDF for KolibriOS v1.5", 0);
					for (ii=0; help[ii]!=0; ii++) {
						kos_text(20, TOOLBAR_HEIGHT + 60 + ii * 15, 0x80000000, help[ii], 0);
					}
				}
				if(butt==14) pdfapp_onkey(&gapp, '['); //previous page
				if(butt==15) pdfapp_onkey(&gapp, ']'); //next page
				if(butt==16) PageRotateLeft();
				if(butt==17) PageRotateRight();
				if(butt==20) GetNewPageNumber();
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
