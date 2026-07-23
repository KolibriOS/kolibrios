#include "calc.h"

#include <clayer/boxlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

/* ============================ constants =========================== */

#define TABLE_VERSION "1.0"

// window
#define WND_W 718
#define WND_H 514
#define MENU_PANEL_HEIGHT 40
#define SCROLL_SIZE 16

// interface colors
#define GRID_COLOR 0xa0a0a0
#define TEXT_COLOR 0x000000
#define CELL_COLOR 0xffffff
#define CELL_COLOR_ACTIVE 0xe0e0ff
#define HEADER_CELL_COLOR 0xE9E7E3
#define HEADER_CELL_COLOR_ACTIVE 0xC4C5BA

// button IDs
#define SAVE_BUTTON 100
#define LOAD_BUTTON 101
#define NEW_BUTTON 102
#define SELECT_ALL_BUTTON 103
#define COL_HEAD_BUTTON 0x300
#define ROW_HEAD_BUTTON 0x400
#define CELL_BUTTON 0x500

// draw-less clickable button (BT_HIDE | BT_NOFRAME)
#define BT_NODRAW 0x60000000

// size_state values (cell resize / range select-drag)
#define SIZE_X 1
#define SIZE_Y 2
#define SIZE_SELECT 3
#define SIZE_DRAG 4

/* ======================= cell model (calc.c) ====================== */
// Defined here, shared with calc.c via extern. Bounds: columns A..CZ,
// rows 1..100; index 0 is the header row/column, so counts are one larger.

Table tbl = { 105, 101 }; // columns A..CZ, rows 1..100 (index 0 = headers)
const char *sFileSign = "KolibriTable File\n";

// clipboard
char ***buffer = NULL;
int buf_col, buf_row;

/* ============================= UI state =========================== */

static int buf_old_x, buf_old_y; // origin of the copied block

// selection
static int sel_x = 1, sel_y = 1;
static int sel_end_x = 1, sel_end_y = 1;
static int nx = 0, ny = 0; // one past the last visible column/row
static int display_formulas = 0;

// pixel scroll is the primary state; grid.firstx/firsty (first visible cell)
// and off_x/off_y (pixels of it hidden past the top-left edge) are derived
// from it in clamp_view()
static int scroll_x = 0, scroll_y = 0;
static int off_x = 0, off_y = 0;

// window work area + skin colors
static int cWidth, cHeight;
static ksys_colors_table_t sc;

// resize / range-drag state
static int size_mouse_x, size_mouse_y, size_id, size_state = 0;
static int drag_x, drag_y;
static int old_end_x, old_end_y;

// grid geometry: pixel rect of the table area + first visible cell
static struct GRID {
	int x, y, w, h;
	int firstx, firsty;
} grid = { 0, 0, 0, 0, 1, 1 };

// labels
static const char sFilename[] = "Filename:";
static const char sSave[] = "Save";
static const char sLoad[] = "Load";
static const char er_file_not_found[] = "'Cannot open file' -E";
static const char er_format[] = "'Error: bad format' -E";
static const char msg_save[] = "'File saved' -O";
static const char msg_save_error[] = "'Error saving file' -E";

// box_lib widgets
static char edit_text[256];
static edit_box cell_box = { 0, 9 * 8 - 6, WND_H - 16 - 32, 0xffffff, 0x94AECE, 0,
	0x808080, 0x10000000, sizeof(edit_text) - 1, edit_text, 0, 0 };

static char fname[256];
static edit_box file_box = { 160, 9 * 8 + 12, WND_H - 16 - 32, 0xffffff, 0x94AECE,
	0, 0x808080, 0x10000000, sizeof(fname) - 1, fname, 0, 0 };

static scrollbar scroll_v = { SCROLL_SIZE, 200, 398, 0, SCROLL_SIZE, 0, 115,
	15, 0, 0xeeeeee, 0xD2CED0, 0x555555, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 };
static scrollbar scroll_h = { 200, 0, SCROLL_SIZE, 0, SCROLL_SIZE, 0, 115,
	15, 0, 0xeeeeee, 0xD2CED0, 0x555555, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 };

#define is_edit (cell_box.flags & ed_focus)

/* ============================= functions ========================== */

static void draw_grid(void);
static void EventGridSelectAll(void);
static int col_left(int j);
static int row_top(int i);

// scroll the minimum amount so cell (cx,cy) is fully inside the viewport
static void ensure_visible(int cx, int cy)
{
	int aw = grid.w - tbl.w[0], ah = grid.h - tbl.h[0];
	int l = col_left(cx), r = l + tbl.w[cx];
	int t = row_top(cy), b = t + tbl.h[cy];
	if (l < scroll_x)
		scroll_x = l;
	else if (r > scroll_x + aw)
		scroll_x = r - aw;
	if (t < scroll_y)
		scroll_y = t;
	else if (b > scroll_y + ah)
		scroll_y = b - ah;
}

// button define that first clears the previous one with the same id
static void def_button(int x, int y, int w, int h, uint32_t id, uint32_t color)
{
	_ksys_delete_button(id);
	_ksys_define_button(x, y, w, h, id, color);
}

// text with a KolibriOS fn4 font flag (0x10 small, 0x90 big); ksys.h has no
// font-flag wrapper, hence the raw syscall
static void draw_text(int x, int y, unsigned font, uint32_t color, const char *s, int len)
{
	__asm__ __volatile__(
		"int $0x40"
		:
		: "a"(4), "b"((x << 16) | (y & 0xFFFF)),
		  "c"((font << 24) | (color & 0xFFFFFF)), "d"(s), "S"(len)
		: "memory");
}

// line; invert=1 draws an XOR (rubber-band) line (fn38 invert, not in ksys.h)
static void draw_line(int x1, int y1, int x2, int y2, uint32_t color, int invert)
{
	uint32_t edx = invert ? 0x01000000 : color;
	__asm__ __volatile__(
		"int $0x40"
		:
		: "a"(38), "b"((x1 << 16) | (x2 & 0xFFFF)),
		  "c"((y1 << 16) | (y2 & 0xFFFF)), "d"(edx)
		: "memory");
}

// rectangle outline
static void draw_region(int x, int y, int w, int h, uint32_t color)
{
	_ksys_draw_line(x, y, x + w - 2, y, color);
	_ksys_draw_line(x, y + 1, x, y + h - 1, color);
	_ksys_draw_line(x + w - 1, y, x + w - 1, y + h - 2, color);
	_ksys_draw_line(x + 1, y + h - 1, x + w - 1, y + h - 1, color);
}

// text clipped to area_w pixels (small font)
static void draw_cut_text(int x, int y, int area_w, uint32_t color, const char *s)
{
	int len;
	if (!s)
		return;
	len = strlen(s);
	if (len * 8 > area_w)
		len = area_w / 8;
	draw_text(x, y, 0x10, color, s, len);
}

static void DrawSelectedFrame(int x, int y, int w, int h, uint32_t col)
{
	_ksys_draw_bar(x, y, w, 2, col);         // up
	_ksys_draw_bar(x, y, 2, h, col);         // left
	_ksys_draw_bar(x, y + h - 2, w - 2 - 3, 2, col); // bottom
	_ksys_draw_bar(x + w - 2, y, 2, h - 2 - 3, col); // right
	_ksys_draw_bar(x + w - 4, y + h - 4, 4, 4, col);
}

static void DrawScrolls(void)
{
	// horizontal
	scroll_h.xpos = 0;
	scroll_h.ypos = grid.y + grid.h;
	scroll_h.xsize = grid.w + SCROLL_SIZE + 1;
	scroll_h.all_redraw = 1;
	scroll_h.max_area = col_left(tbl.cols);   // total content width, px
	scroll_h.cur_area = grid.w - tbl.w[0];    // viewport width, px
	scroll_h.position = scroll_x;
	scrollbar_h_draw(&scroll_h);

	// vertical
	scroll_v.xpos = grid.x + grid.w;
	scroll_v.ypos = 0;
	scroll_v.ysize = grid.h + 1;
	scroll_v.all_redraw = 1;
	scroll_v.max_area = row_top(tbl.rows);    // total content height, px
	scroll_v.cur_area = grid.h - tbl.h[0];    // viewport height, px
	scroll_v.position = scroll_y;
	scrollbar_v_draw(&scroll_v);
}

static void start_edit(int x, int y)
{
	int sx0 = scroll_x, sy0 = scroll_y;
	ensure_visible(x, y);
	if (scroll_x != sx0 || scroll_y != sy0)
		draw_grid();

	file_box.flags &= ~ed_focus;

	cell_box.flags = ed_focus;
	cell_box.left = tbl.x[x] + 1;
	cell_box.top = tbl.y[y];
	cell_box.width = tbl.w[x] - 2;
	memset(edit_text, 0, sizeof(edit_text));
	if (tbl.cells[x][y])
		strcpy(edit_text, tbl.cells[x][y]);
	cell_box.offset = cell_box.shift = cell_box.shift_old = 0;
	cell_box.pos = cell_box.size = strlen(edit_text);

	edit_box_draw(&cell_box);
	edit_box_draw(&file_box);
}

static void stop_edit(void)
{
	if (is_edit) {
		cell_box.flags &= ~ed_focus;
		if (tbl.cells[sel_x][sel_y])
			free(tbl.cells[sel_x][sel_y]);
		if (strlen(edit_text) > 0) {
			tbl.cells[sel_x][sel_y] = malloc(strlen(edit_text) + 1);
			strcpy(tbl.cells[sel_x][sel_y], edit_text);
		} else
			tbl.cells[sel_x][sel_y] = NULL;
		table_recalc();
	}
}

static void cancel_edit(void)
{
	if (!is_edit)
		return;
	cell_box.flags &= ~ed_focus;
	memset(edit_text, 0, 256);
	draw_grid();
}

static void move_selection(int new_x, int new_y)
{
	stop_edit();
	sel_x = new_x;
	if (sel_x < 1)
		sel_x = 1;
	if (sel_x > tbl.cols - 1)
		sel_x = tbl.cols - 1;
	sel_end_x = sel_x;
	sel_y = new_y;
	if (sel_y < 1)
		sel_y = 1;
	if (sel_y > tbl.rows - 1)
		sel_y = tbl.rows - 1;
	sel_end_y = sel_y;
	ensure_visible(sel_x, sel_y);
	draw_grid();
}

// x within [low, high] (order-independent)
static int is_between(int x, int low, int high)
{
	return ((low < high) ? (x >= low && x <= high) : (x >= high && x <= low));
}

// pixel offset of data column j / row i from the start of the data area
static int col_left(int j) { int i, s = 0; for (i = 1; i < j; i++) s += tbl.w[i]; return s; }
static int row_top(int i)  { int k, s = 0; for (k = 1; k < i; k++) s += tbl.h[k]; return s; }

// fill a bar, clipped to the rectangle [cx0,cx1) x [cy0,cy1)
static void bar_clip(int x, int y, int w, int h, int cx0, int cy0, int cx1, int cy1, uint32_t c)
{
	if (x < cx0) { w -= cx0 - x; x = cx0; }
	if (y < cy0) { h -= cy0 - y; y = cy0; }
	if (x + w > cx1) w = cx1 - x;
	if (y + h > cy1) h = cy1 - y;
	if (w > 0 && h > 0)
		_ksys_draw_bar(x, y, w, h, c);
}

// clamp scrolling to the real table bounds in pixels, then split back into
// (first visible cell, sub-cell offset) so the last cell sits exactly flush
static void clamp_view(void)
{
	int max;

	max = col_left(tbl.cols) - (grid.w - tbl.w[0]); // content - viewport
	if (max < 0) max = 0;
	if (scroll_x > max) scroll_x = max;
	if (scroll_x < 0) scroll_x = 0;
	grid.firstx = 1;
	while (grid.firstx < tbl.cols - 1 && col_left(grid.firstx + 1) <= scroll_x)
		grid.firstx++;
	off_x = scroll_x - col_left(grid.firstx);

	max = row_top(tbl.rows) - (grid.h - tbl.h[0]);
	if (max < 0) max = 0;
	if (scroll_y > max) scroll_y = max;
	if (scroll_y < 0) scroll_y = 0;
	grid.firsty = 1;
	while (grid.firsty < tbl.rows - 1 && row_top(grid.firsty + 1) <= scroll_y)
		grid.firsty++;
	off_y = scroll_y - row_top(grid.firsty);
}

static void draw_grid(void)
{
	int i, j, sx, sy;
	int W = grid.w, H = grid.h, cw0 = tbl.w[0], ch0 = tbl.h[0];
	uint32_t bg;

	clamp_view();

	// screen x/y of every column/row left/top edge (-1 if off-screen).
	// off_x/off_y shift the first visible cell partly under the headers.
	sx = cw0 - off_x;
	nx = 1;
	tbl.x[0] = 0;
	for (j = 1; j < tbl.cols; j++) {
		tbl.x[j] = (j >= grid.firstx && sx < W) ? sx : -1;
		if (tbl.x[j] >= 0)
			nx = j + 1;
		if (j >= grid.firstx)
			sx += tbl.w[j];
	}
	sy = ch0 - off_y;
	ny = 1;
	tbl.y[0] = 0;
	for (i = 1; i < tbl.rows; i++) {
		tbl.y[i] = (i >= grid.firsty && sy < H) ? sy : -1;
		if (tbl.y[i] >= 0)
			ny = i + 1;
		if (i >= grid.firsty)
			sy += tbl.h[i];
	}

	// cells (bars clipped to the data area; a half-scrolled first row/column
	// spills under the headers, which are painted on top afterwards)
	for (i = grid.firsty; i < ny; i++) {
		int cy = tbl.y[i], h = tbl.h[i];
		if (cy < 0)
			continue;
		for (j = grid.firstx; j < nx; j++) {
			int cx = tbl.x[j], w = tbl.w[j];
			char *text;
			if (cx < 0)
				continue;
			bg = CELL_COLOR;
			if (is_between(j, sel_x, sel_end_x) && is_between(i, sel_y, sel_end_y)) {
				if (i == sel_y && j == sel_x) {
					drag_x = cx + w - 4;
					drag_y = cy + h - 4;
				} else
					bg = CELL_COLOR_ACTIVE;
			}
			text = (tbl.values[j][i] && tbl.values[j][i][0] == '#') ? tbl.cells[j][i]
			     : (tbl.values[j][i] && !display_formulas ? tbl.values[j][i] : tbl.cells[j][i]);
			bar_clip(cx + 1, cy + 1, w - 1, h - 1, cw0, ch0, W, H, bg);
			if (cx >= cw0 && cy >= ch0) {
				if (text)
					draw_cut_text(cx + 3, cy + h / 2 - 7, w - 7, TEXT_COLOR, text);
				if (i == sel_y && j == sel_x)
					DrawSelectedFrame(cx + 1, cy, w - 1, h + 1, TEXT_COLOR);
				else if (tbl.values[j][i] && tbl.values[j][i][0] == '#')
					draw_region(cx + 1, cy + 1, w - 1, h - 1, 0xff0000);
			}
		}
	}

	// column headers (top band)
	_ksys_draw_bar(cw0, 0, W - cw0, ch0, HEADER_CELL_COLOR);
	for (j = grid.firstx; j < nx; j++) {
		int cx = tbl.x[j], w = tbl.w[j];
		if (cx < 0)
			continue;
		bg = is_between(j, sel_x, sel_end_x) ? HEADER_CELL_COLOR_ACTIVE : HEADER_CELL_COLOR;
		bar_clip(cx, 0, w, ch0, cw0, 0, W, ch0, bg);
		if (cx >= cw0)
			_ksys_draw_bar(cx, 0, 1, H, GRID_COLOR); // full-height column line
		def_button(cx + 5, 0, w - 10, ch0 - 1, j + COL_HEAD_BUTTON + BT_NODRAW, 0);
		if (tbl.cells[j][0] && cx + w / 2 - (int)strlen(tbl.cells[j][0]) * 4 >= cw0)
			draw_text(cx + w / 2 - strlen(tbl.cells[j][0]) * 4, ch0 / 2 - 7, 0x90, TEXT_COLOR, tbl.cells[j][0], 0);
	}

	// row headers (left band)
	_ksys_draw_bar(0, ch0, cw0, H - ch0, HEADER_CELL_COLOR);
	for (i = grid.firsty; i < ny; i++) {
		int cy = tbl.y[i], h = tbl.h[i];
		if (cy < 0)
			continue;
		bg = is_between(i, sel_y, sel_end_y) ? HEADER_CELL_COLOR_ACTIVE : HEADER_CELL_COLOR;
		bar_clip(0, cy, cw0, h, 0, ch0, cw0, H, bg);
		if (cy >= ch0)
			_ksys_draw_bar(0, cy, W, 1, GRID_COLOR); // full-width row line
		def_button(5, cy, cw0 - 10, h - 1, i + ROW_HEAD_BUTTON + BT_NODRAW, 0);
		if (tbl.cells[0][i] && cy + h / 2 - 7 >= ch0)
			draw_text(cw0 / 2 - (int)strlen(tbl.cells[0][i]) * 4, cy + h / 2 - 7, 0x90, TEXT_COLOR, tbl.cells[0][i], 0);
	}

	// corner
	_ksys_draw_bar(0, 0, cw0, ch0, HEADER_CELL_COLOR);
	def_button(0, 0, cw0 - 4, ch0 - 4, SELECT_ALL_BUTTON + BT_NODRAW, 0);

	// fixed dividers between the headers and the table (independent of scroll)
	_ksys_draw_bar(cw0, 0, 1, H, GRID_COLOR);
	_ksys_draw_bar(0, ch0, W, 1, GRID_COLOR);

	DrawScrolls();
}

// very fast grid draw while resizing cells (XOR lines)
static void draw_size_grid(void)
{
	if (size_state == SIZE_X) {
		int x, x0, i;
		x = tbl.w[0];
		x0 = 0;
		for (i = 1; i < tbl.cols && x - x0 + tbl.w[i] < grid.w - 10; i++) {
			if (i >= grid.firstx) {
				if (i >= size_id)
					draw_line(x - x0, 0, x - x0, grid.h, 0, 1);
			} else
				x0 += tbl.w[i];
			x += tbl.w[i];
		}
		draw_line(x - x0, 0, x - x0, grid.h, 0, 1);
	} else {
		int y, y0, i;
		y = tbl.h[0];
		y0 = 0;
		for (i = 1; i < tbl.cols && y - y0 + tbl.h[i] < grid.h - 10; i++) {
			if (i >= grid.firsty) {
				if (i >= size_id)
					draw_line(0, y - y0, grid.w, y - y0, 0, 1);
			} else
				y0 += tbl.h[i];
			y += tbl.h[i];
		}
		draw_line(0, y - y0, grid.w, y - y0, 0, 1);
	}
}

// fast draw of the selected area while dragging with the mouse
#define DCOLOR 0
#define DINVERT 1
static void draw_drag(void)
{
	int k0 = min(sel_x, sel_end_x);
	int k1 = max(sel_x, sel_end_x);
	int n0 = min(sel_y, sel_end_y);
	int n1 = max(sel_y, sel_end_y);

	int x0 = tbl.x[k0] - 1;
	int x1 = tbl.x[k1] + tbl.w[k1] + 1;
	int y0 = tbl.y[n0] - 1;
	int y1 = tbl.y[n1] + tbl.h[n1] + 1;
	if (x0 > grid.w - 1)
		x0 = grid.w - 1;
	if (x1 > grid.w - 1)
		x1 = grid.w - 1;
	if (y0 > grid.h - 1)
		y0 = grid.h - 1;
	if (y1 > grid.h - 1)
		y1 = grid.h - 1;

	draw_line(x0, y0, x0, y1, DCOLOR, DINVERT);
	draw_line(x0, y0, x1, y0, DCOLOR, DINVERT);
	draw_line(x1, y0, x1, y1, DCOLOR, DINVERT);
	draw_line(x0, y1, x1, y1, DCOLOR, DINVERT);
}

static void draw_window(void)
{
	ksys_thread_t info;
	int panel_y, BTX;

	_ksys_start_draw();
	_ksys_create_window(110, 40, WND_W, WND_H, "Table " TABLE_VERSION, 0x40FFFFFF, 0x73);
	_ksys_end_draw();

	_ksys_get_system_colors(&sc);

	_ksys_thread_info(&info, KSYS_THIS_SLOT);
	cWidth = info.winx_size - 9;
	cHeight = info.winy_size - _ksys_get_skin_height() - 4;

	grid.x = 0;
	grid.y = 0;
	grid.w = cWidth - SCROLL_SIZE - 1;
	grid.h = cHeight - MENU_PANEL_HEIGHT - SCROLL_SIZE;

	if (info.window_state & 0x04)
		return; // rolled up

	if (cWidth < 430) {
		_ksys_change_window(-1, -1, 450, -1);
		return;
	}
	if (cHeight < 250) {
		_ksys_change_window(-1, -1, -1, 300);
		return;
	}

	if (is_edit)
		stop_edit();

	panel_y = cHeight - MENU_PANEL_HEIGHT + 1;
	_ksys_draw_bar(0, panel_y, cWidth, MENU_PANEL_HEIGHT - 1, sc.work_area);
	draw_text(3 + 1, panel_y + 14, 0x90, sc.work_text, sFilename, 0);

	file_box.top = panel_y + 10;
	file_box.width = cWidth - 265;
	BTX = cWidth - 190;
#define BTW 70
	def_button(BTX + 25, file_box.top, BTW, 21, SAVE_BUTTON, sc.work_area);
	draw_text(BTX + 25 + (BTW - (int)strlen(sSave) * 8) / 2, panel_y + 14, 0x90, sc.work_text, sSave, 0);
	def_button(BTX + 25 + BTW + 5, file_box.top, BTW, 21, LOAD_BUTTON, sc.work_area);
	draw_text(BTX + 25 + BTW + 5 + (BTW - (int)strlen(sLoad) * 8) / 2, panel_y + 14, 0x90, sc.work_text, sLoad, 0);

	draw_grid();

	if (is_edit)
		edit_box_draw(&cell_box);
	edit_box_draw(&file_box);
}

static void process_mouse(void)
{
	int mouse_btn, ckeys, shift;
	int mouse_x, mouse_y, i;
	ksys_pos_t mp;
	int vert = (int16_t)(_ksys_get_mouse_wheels() & 0xFFFF);

	if (vert != 0) {
		stop_edit();
		scroll_y += vert * 60; // ~3 rows per wheel notch
		draw_grid();           // clamp_view() clamps scroll_y
		return;
	}

	if (!size_state) { // do not handle scrollbars while selecting cells
		if (!scroll_h.delta2)
			scrollbar_v_mouse(&scroll_v);
		if (scroll_v.position != scroll_y) {
			stop_edit();
			scroll_y = scroll_v.position;
			draw_grid();
		}

		if (!scroll_v.delta2)
			scrollbar_h_mouse(&scroll_h);
		if (scroll_h.position != scroll_x) {
			stop_edit();
			scroll_x = scroll_h.position;
			draw_grid();
		}
	}
	if (scroll_v.delta2 || scroll_h.delta2)
		return;

	if (is_edit)
		edit_box_mouse(&cell_box);
	edit_box_mouse(&file_box);

	mp = _ksys_get_mouse_pos(KSYS_MOUSE_WINDOW_POS);
	mouse_btn = _ksys_get_mouse_buttons();
	mouse_x = mp.x; // fn37.1 is already work-area relative
	mouse_y = mp.y;

	if (is_edit && mouse_x >= (int)cell_box.left && mouse_x <= (int)cell_box.left + (int)cell_box.width
	    && mouse_y >= (int)cell_box.top && mouse_y <= (int)cell_box.top + 22)
		return;

	mouse_btn &= 0x0001;

	if (mouse_btn) {
		if (mouse_y < 0)
			return; // over header
		if (mouse_y > grid.y + grid.h)
			return;
	}

	ckeys = _ksys_get_control_key_state();
	shift = ckeys & 0x3;

	if (!size_state && !mouse_btn)
		return;
	if (mouse_btn && !size_state) { // LMB down
		if (mouse_x >= drag_x && mouse_x <= drag_x + 4 && mouse_y >= drag_y && mouse_y <= drag_y + 4) {
			size_state = SIZE_DRAG;
			old_end_x = sel_end_x;
			old_end_y = sel_end_y;
		} else if (mouse_y <= tbl.h[0]) {
			int kx = -1;
			for (i = 0; i < tbl.cols - 1; i++)
				if (mouse_x >= tbl.x[i] + tbl.w[i] - 5 && mouse_x <= tbl.x[i + 1] + 5) {
					kx = i;
					break;
				}
			if (kx != -1) {
				size_id = kx;
				size_state = SIZE_X;
			}
		} else if (mouse_x <= tbl.w[0]) {
			int ky = -1;
			for (i = 0; i < tbl.rows - 1; i++)
				if (mouse_y >= tbl.y[i] + tbl.h[i] - 5 && mouse_y <= tbl.y[i + 1] + 5) {
					ky = i;
					break;
				}
			if (ky != -1) {
				size_id = ky;
				size_state = SIZE_Y;
			}
		} else if (mouse_x < grid.w && mouse_y < grid.h) { // click on cell
			int kx = -1, ky = -1;
			for (i = grid.firstx; i < nx; i++)
				if (tbl.x[i] >= 0 && mouse_x >= tbl.x[i] && mouse_x <= tbl.x[i] + tbl.w[i]) {
					kx = i;
					break;
				}
			for (i = grid.firsty; i < ny; i++)
				if (tbl.y[i] >= 0 && mouse_y >= tbl.y[i] && mouse_y <= tbl.y[i] + tbl.h[i]) {
					ky = i;
					break;
				}
			if (kx != -1 && ky != -1) {
				if (!shift)
					move_selection(kx, ky);
				else {
					sel_end_x = kx;
					sel_end_y = ky;
				}
				size_state = SIZE_SELECT;
			}
		}
		if (size_state) {
			size_mouse_x = mouse_x;
			size_mouse_y = mouse_y;
		}
		return;
	} else if (!mouse_btn && size_state) {
		if (size_state == SIZE_DRAG)
			table_fill(sel_x, sel_y, sel_end_x, sel_end_y, old_end_x, old_end_y);

		size_state = 0;
		draw_grid(); // everything shifted - refresh
		return;
	}
	if (size_state == SIZE_X && mouse_x != size_mouse_x) {
		draw_size_grid();
		tbl.w[size_id] += mouse_x - size_mouse_x;
		if (tbl.w[size_id] < 15)
			tbl.w[size_id] = 15;
		else if (tbl.w[size_id] > grid.w / 2)
			tbl.w[size_id] = grid.w / 2;
		draw_size_grid();
	}
	if (size_state == SIZE_Y && mouse_y != size_mouse_y) {
		draw_size_grid();
		tbl.h[size_id] += mouse_y - size_mouse_y;
		if (tbl.h[size_id] < 15)
			tbl.h[size_id] = 15;
		else if (tbl.h[size_id] > grid.h / 2)
			tbl.h[size_id] = grid.h / 2;
		draw_size_grid();
	}
	if ((size_state == SIZE_SELECT || size_state == SIZE_DRAG) && (mouse_x != size_mouse_x || mouse_y != size_mouse_y)) {
		int kx = -1, ky = -1;
		draw_drag();
		for (i = grid.firstx; i < nx; i++)
			if (tbl.x[i] >= 0 && mouse_x <= tbl.x[i] + tbl.w[i]) {
				kx = i;
				break;
			}
		for (i = grid.firsty; i < ny; i++)
			if (tbl.y[i] >= 0 && mouse_y <= tbl.y[i] + tbl.h[i]) {
				ky = i;
				break;
			}
		if (kx != -1)
			sel_end_x = kx;
		if (ky != -1)
			sel_end_y = ky;
		if (size_state == SIZE_DRAG) {
			if (abs(sel_end_x - sel_x) > 0)
				sel_end_y = old_end_y;
			else if (abs(sel_end_y - sel_y) > 0)
				sel_end_x = old_end_x;
		}
		draw_drag();
	}
	size_mouse_x = mouse_x;
	size_mouse_y = mouse_y;
}

static void shift_selection(int dx, int dy, int shift)
{
	if (dx != 0 && shift) {
		sel_end_x += dx;
		if (sel_end_x <= 1)
			sel_end_x = 1;
		else if (sel_end_x >= tbl.cols)
			sel_end_x = tbl.cols - 1;
	}
	if (dy != 0 && shift) {
		sel_end_y += dy;
		if (sel_end_y <= 1)
			sel_end_y = 1;
		else if (sel_end_y >= tbl.rows)
			sel_end_y = tbl.rows - 1;
	}
	if (dx || dy) {
		if (!shift) {
			if ((sel_end_x + dx) > (tbl.cols - 1)) {
				dx = 0;
			} else if ((sel_end_y + dy) > (tbl.rows - 1)) {
				dy = 0;
			} else
				move_selection(sel_x + dx, sel_y + dy);
		} else {
			stop_edit();
			draw_grid();
		}
	}
}

static void process_key(void)
{
	int ckeys, shift, ctrl;
	ksys_oskey_t k;
	int key_ascii, key_scancode;

	ckeys = _ksys_get_control_key_state();
	shift = ckeys & 0x3;
	ctrl = ckeys & 0x0c;

	k = _ksys_get_key();
	key_ascii = k.code;
	key_scancode = k.ctrl_key;

	if (cell_box.flags & ed_focus) {
		if (KSYS_SCANCODE_ENTER == key_scancode) {
			stop_edit();
			draw_grid();
		} else if (KSYS_SCANCODE_ESC == key_scancode)
			cancel_edit();
		else
			edit_box_key_safe(&cell_box, k);
	} else if (file_box.flags & ed_focus) {
		edit_box_key_safe(&file_box, k);
		return;
	} else if (ctrl) {
		switch (key_scancode) {
		case KSYS_SCANCODE_A:
			EventGridSelectAll();
			break;
		case KSYS_SCANCODE_V: {
			int i, j, x0, y0, delta_x, delta_y;
			x0 = min(sel_x, sel_end_x);
			y0 = min(sel_y, sel_end_y);
			delta_x = x0 - buf_old_x;
			delta_y = y0 - buf_old_y;

			for (i = 0; i < buf_col; i++)
				for (j = 0; j < buf_row; j++) {
					if (i + x0 >= tbl.cols || j + y0 >= tbl.rows)
						continue;
					if (tbl.cells[i + x0][j + y0])
						free(tbl.cells[i + x0][j + y0]);
					if (buffer[i][j]) {
						cf_x0 = buf_old_x;
						cf_y0 = buf_old_y;
						cf_x1 = buf_old_x + buf_col;
						cf_y1 = buf_old_y + buf_row;
						tbl.cells[i + x0][j + y0] = table_shift_formula(buffer[i][j], delta_x, delta_y);
					} else
						tbl.cells[i + x0][j + y0] = NULL;
				}

			table_recalc();
			draw_grid();
			break;
		}
		case KSYS_SCANCODE_X:
		case KSYS_SCANCODE_C: {
			int i, j, x0, y0;

			freeBuffer();

			buf_col = abs(sel_end_x - sel_x) + 1;
			buf_row = abs(sel_end_y - sel_y) + 1;
			x0 = min(sel_x, sel_end_x);
			y0 = min(sel_y, sel_end_y);
			buf_old_x = x0;
			buf_old_y = y0;

			buffer = malloc(buf_col * sizeof(char **));
			for (i = 0; i < buf_col; i++) {
				buffer[i] = malloc(buf_row * sizeof(char *));
				for (j = 0; j < buf_row; j++) {
					if (tbl.cells[i + x0][j + y0]) {
						if (KSYS_SCANCODE_C == key_scancode) {
							buffer[i][j] = malloc(strlen(tbl.cells[i + x0][j + y0]) + 1);
							strcpy(buffer[i][j], tbl.cells[i + x0][j + y0]);
						} else {
							buffer[i][j] = tbl.cells[i + x0][j + y0];
							tbl.cells[i + x0][j + y0] = NULL;
						}
					} else
						buffer[i][j] = NULL;
				}
			}
			if (key_ascii == 24)
				table_recalc();
			draw_grid();
			break;
		}
		case KSYS_SCANCODE_F:
			display_formulas = !display_formulas;
			draw_grid();
			break;
		}
	} else
		switch (key_scancode) {
		case KSYS_SCANCODE_EXT_UP:
			shift_selection(0, -1, shift);
			break;
		case KSYS_SCANCODE_EXT_LEFT:
			shift_selection(-1, 0, shift);
			break;
		case KSYS_SCANCODE_EXT_RIGHT:
			shift_selection(1, 0, shift);
			break;
		case KSYS_SCANCODE_EXT_DOWN:
			shift_selection(0, 1, shift);
			break;
		case KSYS_SCANCODE_EXT_PGDOWN:
			shift_selection(0, ny - grid.firsty - 1, shift);
			break;
		case KSYS_SCANCODE_EXT_PGUP:
			shift_selection(0, -(ny - grid.firsty), shift);
			break;
		case KSYS_SCANCODE_EXT_HOME:
			shift_selection(-sel_x + 1, 0, shift);
			break;
		case KSYS_SCANCODE_EXT_END:
			shift_selection(tbl.cols - (nx - grid.firstx) - 1 - sel_x, 0, shift);
			break;
		case KSYS_SCANCODE_EXT_DELETE: {
			int i, j;
			int n0 = min(sel_x, sel_end_x);
			int n1 = max(sel_x, sel_end_x);
			int k0 = min(sel_y, sel_end_y);
			int k1 = max(sel_y, sel_end_y);

			for (i = n0; i <= n1; i++)
				for (j = k0; j <= k1; j++)
					if (tbl.cells[i][j]) {
						free(tbl.cells[i][j]);
						tbl.cells[i][j] = NULL;
					}
			table_recalc();
			draw_grid();
			break;
		}
		case KSYS_SCANCODE_F2:
			start_edit(sel_x, sel_y);
			break;
		case KSYS_SCANCODE_F5:
			draw_grid();
			break;
		default:
			start_edit(sel_x, sel_y);
			edit_box_key_safe(&cell_box, k);
			break;
		}
}

static void EventLoadFile(void)
{
	int r;
	stop_edit();
	r = table_load(fname);
	if (r > 0) {
		table_recalc();
		draw_grid();
	} else {
		const char *result = NULL;
		if (r == -1)
			result = er_file_not_found;
		else if (r == -2)
			result = er_format;
		_ksys_exec("/sys/@notify", (char *)result);
	}
}

static void EventGridSelectAll(void)
{
	sel_y = 1;
	sel_x = 1;
	sel_end_x = tbl.cols - 1;
	sel_end_y = tbl.rows - 1;
	stop_edit();
	draw_grid();
}

static void process_button(void)
{
	uint32_t button = _ksys_get_button();
	switch (button) {
	case 1:
		_ksys_exit();

	case NEW_BUTTON:
		table_reset();
		draw_grid();
		break;

	case SAVE_BUTTON:
		stop_edit();
		if (table_save(fname))
			_ksys_exec("/sys/@notify", (char *)msg_save);
		else
			_ksys_exec("/sys/@notify", (char *)msg_save_error);
		break;

	case LOAD_BUTTON:
		EventLoadFile();
		break;

	case SELECT_ALL_BUTTON:
		EventGridSelectAll();
		break;
	}
	if (button >= COL_HEAD_BUTTON && button < ROW_HEAD_BUTTON) {
		sel_end_x = sel_x = button - COL_HEAD_BUTTON;
		sel_y = 1;
		sel_end_y = tbl.rows - 1;
		stop_edit();
		draw_grid();
		return;
	} else if (button >= ROW_HEAD_BUTTON && button < CELL_BUTTON) {
		sel_end_y = sel_y = button - ROW_HEAD_BUTTON;
		sel_x = 1;
		sel_end_x = tbl.cols - 1;
		stop_edit();
		draw_grid();
		return;
	}
}

int main(int argc, char **argv)
{
	table_init();
	if (argc > 1) {
		strcpy(fname, argv[1]);
		file_box.size = file_box.pos = strlen(fname);
		EventLoadFile();
	}
	_ksys_set_event_mask(KSYS_EVM_REDRAW | KSYS_EVM_KEY | KSYS_EVM_BUTTON | KSYS_EVM_MOUSE | KSYS_EVM_MOUSE_FILTER);
	for (;;) {
		switch (_ksys_wait_event()) {
		case KSYS_EVENT_MOUSE:
			process_mouse();
			break;
		case KSYS_EVENT_KEY:
			process_key();
			break;
		case KSYS_EVENT_BUTTON:
			process_button();
			break;
		case KSYS_EVENT_REDRAW:
			draw_window();
			break;
		}
	}
	return 0;
}
