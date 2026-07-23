#include "func.h"
#include "calc.h"

#include <clayer/boxlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

#define TABLE_VERSION "1.0"

// strings
static const char sFilename[] = "Filename:";
static const char sSave[] = "Save";
static const char sLoad[] = "Load";

static const char er_file_not_found[] = "'Cannot open file' -E";
static const char er_format[] = "'Error: bad format' -E";
static const char msg_save[] = "'File saved' -O";
static const char msg_save_error[] = "'Error saving file' -E";

// shared with calc.c
const char *sFileSign = "KolibriTable File\n";

// initial window size
#define WND_W 718
#define WND_H 514
static int cWidth;
static int cHeight;
static ksys_colors_table_t sc;

#define MENU_PANEL_HEIGHT 40

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

// the cell model (used by calc.c too). Bounds: columns A..CZ, rows 1..100
// (index 0 is the header row/column, so counts are one larger).
int col_count = 105, row_count = 101;
int *cell_w, *cell_h;
char ***cells;
char ***values;
int *cell_x, *cell_y;

// clipboard
char ***buffer = NULL;
int buf_col, buf_row;
int buf_old_x, buf_old_y;

// selection
int sel_x = 1, sel_y = 1;
int prev_x = 0, prev_y = 0;
int was_single_selection = 0;
int sel_end_x = 1, sel_end_y = 1;
int sel_moved = 0;
int sel_end_move = 0;
int nx = 0, ny = 0;

int display_formulas = 0; // show formulas instead of values

// edit boxes
static char edit_text[256];
static edit_box cell_box = { 0, 9 * 8 - 6, WND_H - 16 - 32, 0xffffff, 0x94AECE, 0,
	0x808080, 0x10000000, sizeof(edit_text) - 1, edit_text, 0, 0 };

static char fname[256];
static edit_box file_box = { 160, 9 * 8 + 12, WND_H - 16 - 32, 0xffffff, 0x94AECE,
	0, 0x808080, 0x10000000, sizeof(fname) - 1, fname, 0, 0 };

#define is_edit (cell_box.flags & ed_focus)

// scrollbars
#define SCROLL_SIZE 16
static scrollbar scroll_v = { SCROLL_SIZE, 200, 398, 0, SCROLL_SIZE, 0, 115,
	15, 0, 0xeeeeee, 0xD2CED0, 0x555555, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 };
static scrollbar scroll_h = { 200, 0, SCROLL_SIZE, 0, SCROLL_SIZE, 0, 115,
	15, 0, 0xeeeeee, 0xD2CED0, 0x555555, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 };

struct GRID {
	int x, y, w, h;
	int firstx, firsty; // cell x:y in the top left corner
} grid = { 0, 0, 0, 0, 1, 1 };

// resizing state
#define SIZE_X 1
#define SIZE_Y 2
#define SIZE_SELECT 3
#define SIZE_DRAG 4
static int size_mouse_x, size_mouse_y, size_id, size_state = 0;

// filling a range by dragging the bottom-right corner
static int drag_x, drag_y;
static int old_end_x, old_end_y;

static void draw_grid(void);
static void EventGridSelectAll(void);

// button define that first clears the previous one with the same id
static void def_button(int x, int y, int w, int h, uint32_t id, uint32_t color)
{
	_ksys_delete_button(id);
	_ksys_define_button(x, y, w, h, id, color);
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
	scroll_h.max_area = col_count - 1;
	scroll_h.cur_area = nx - grid.firstx - 1;
	scroll_h.position = grid.firstx - 1;
	scrollbar_h_draw(&scroll_h);

	// vertical
	scroll_v.xpos = grid.x + grid.w;
	scroll_v.ypos = 0;
	scroll_v.ysize = grid.h + 1;
	scroll_v.all_redraw = 1;
	scroll_v.max_area = row_count - 1;
	scroll_v.cur_area = ny - grid.firsty - 1;
	scroll_v.position = grid.firsty - 1;
	scrollbar_v_draw(&scroll_v);
}

static void start_edit(int x, int y)
{
	int ch = 0;
	if (x < grid.firstx || x > nx - 1) {
		grid.firstx = x;
		ch = 1;
	}
	if (y < grid.firsty || y > ny - 1) {
		grid.firsty = y;
		ch = 1;
	}
	if (ch) {
		sel_moved = 1;
		draw_grid();
	}

	file_box.flags &= ~ed_focus;

	cell_box.flags = ed_focus;
	cell_box.left = cell_x[x] + 1;
	cell_box.top = cell_y[y];
	cell_box.width = cell_w[x] - 2;
	memset(edit_text, 0, sizeof(edit_text));
	if (cells[x][y])
		strcpy(edit_text, cells[x][y]);
	cell_box.offset = cell_box.shift = cell_box.shift_old = 0;
	cell_box.pos = cell_box.size = strlen(edit_text);

	edit_box_draw(&cell_box);
	edit_box_draw(&file_box);
}

static void stop_edit(void)
{
	if (is_edit) {
		cell_box.flags &= ~ed_focus;
		if (cells[sel_x][sel_y])
			free(cells[sel_x][sel_y]);
		if (strlen(edit_text) > 0) {
			cells[sel_x][sel_y] = malloc(strlen(edit_text) + 1);
			strcpy(cells[sel_x][sel_y], edit_text);
		} else
			cells[sel_x][sel_y] = NULL;
		calculate_values();
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

static void check_sel(void)
{
	int sx0 = grid.firstx, sy0 = grid.firsty;

	if (sel_x >= nx - 1)
		grid.firstx++;
	if (sel_y >= ny - 1)
		grid.firsty++;

	if (sel_x < grid.firstx)
		grid.firstx = sel_x;
	if (sel_y < grid.firsty)
		grid.firsty = sel_y;

	if (sx0 != grid.firstx || sy0 != grid.firsty)
		sel_moved = 0; // must redraw everything
}

static void move_selection(int new_x, int new_y)
{
	sel_moved = 1;
	stop_edit();
	prev_x = sel_x;
	prev_y = sel_y;
	sel_x = new_x;
	if (sel_x < 1)
		sel_x = 1;
	if (sel_x > col_count - 1)
		sel_x = col_count - 1;
	sel_end_x = sel_x;
	sel_y = new_y;
	if (sel_y < 1)
		sel_y = 1;
	if (sel_y > row_count - 1)
		sel_y = row_count - 1;
	sel_end_y = sel_y;
	check_sel();
	draw_grid();
}

// x within [low, high] (order-independent)
static int is_between(int x, int low, int high)
{
	return ((low < high) ? (x >= low && x <= high) : (x >= high && x <= low));
}

static void clear_cell_slow(int px, int py)
{
	int i;
	int x0 = cell_w[0];
	for (i = grid.firstx; i < px; i++)
		x0 += cell_w[i];
	int x1 = x0 + cell_w[px];
	int y0 = cell_h[0];
	for (i = grid.firsty; i < py; i++)
		y0 += cell_h[i];
	int y1 = y0 + cell_h[py];
	_ksys_draw_bar(x0 + 1, y0 + 1, x1 - x0 - 1, y1 - y0 - 1, 0xffffff);
}

#define is_x_changed(v) ((v) == sel_x || (v) == prev_x)
#define is_y_changed(v) ((v) == sel_y || (v) == prev_y)

static void DrawCell(int x, int y, int w, int h, uint32_t id, uint32_t bg_color, char *text, int header)
{
	int small = 0;
	if (x > grid.x + grid.w || w > grid.w || w <= 0)
		return;
	if (x + w > grid.x + grid.w) {
		w = grid.x + grid.w - x;
		small = 1;
	}
	if (y + h > grid.y + grid.h) {
		h = grid.y + grid.h - y;
		small = 1;
	}
	_ksys_draw_bar(x, y, w, h, bg_color);
	if (!small) {
		if (id)
			def_button(x + 5, y, w - 10, h - 1, id + BT_NODRAW, 0);
		if (header)
			draw_text(x + w / 2 - strlen(text) * 4, h / 2 - 7 + y, 0x90, TEXT_COLOR, text, 0);
		else
			draw_cut_text(x + 3, h / 2 - 7 + y, w - 7, TEXT_COLOR, text);
	}
}

// clamp the top-left visible cell so the viewport stays inside the real table:
// never scroll past the last row/column, and never leave a gap after them
static void clamp_view(void)
{
	int avail, sum, f;

	avail = grid.w - cell_w[0];
	sum = 0;
	for (f = col_count - 1; f >= 1; f--) {
		sum += cell_w[f];
		if (sum > avail) {
			f++;
			break;
		}
	}
	if (f < 1)
		f = 1;
	if (f > col_count - 1)
		f = col_count - 1;
	if (grid.firstx > f)
		grid.firstx = f;
	if (grid.firstx < 1)
		grid.firstx = 1;

	avail = grid.h - cell_h[0];
	sum = 0;
	for (f = row_count - 1; f >= 1; f--) {
		sum += cell_h[f];
		if (sum > avail) {
			f++;
			break;
		}
	}
	if (f < 1)
		f = 1;
	if (f > row_count - 1)
		f = row_count - 1;
	if (grid.firsty > f)
		grid.firsty = f;
	if (grid.firsty < 1)
		grid.firsty = 1;
}

static void draw_grid(void)
{
	int i, j;
	long x0 = 0, y0 = 0, x = 0, y = 0;
	int right_edge = 0, bottom_edge = 0;
	uint32_t bg_color;
	clamp_view();
	_ksys_draw_bar(0, 0, cell_w[0], cell_h[0], HEADER_CELL_COLOR); // left top cell
	def_button(0, 0, cell_w[0] - 4, cell_h[0] - 4, SELECT_ALL_BUTTON + BT_NODRAW, 0);

	nx = ny = 0;

	// clear the area around the selected cell
	if (sel_moved) {
		clear_cell_slow(sel_x, sel_y);
		clear_cell_slow(prev_x, prev_y);
	}

	// column headers + vertical lines
	cell_x[0] = 0;
	x = cell_w[0];
	nx = 1;
	for (i = 1; i < col_count && x - x0 < grid.w; i++) {
		cell_x[i] = -1;
		if (i >= grid.firstx) {
			if (is_between(i, sel_x, sel_end_x))
				bg_color = HEADER_CELL_COLOR_ACTIVE;
			else
				bg_color = HEADER_CELL_COLOR;
			_ksys_draw_bar(x - x0, 0, 1, grid.h, GRID_COLOR);
			DrawCell(x - x0 + 1, 0, cell_w[i] - 1, cell_h[0], i + COL_HEAD_BUTTON, bg_color, cells[i][0], 1);
			cell_x[i] = x - x0;
		} else
			x0 += cell_w[i];
		x += cell_w[i];
		nx++;
	}
	right_edge = x - x0; // right edge of the last visible column

	// row headers + horizontal lines
	y = cell_h[0];
	ny = 1;
	cell_y[0] = 0;
	for (i = 1; i < row_count && y - y0 < grid.h; i++) {
		cell_y[i] = -1;
		if (i >= grid.firsty) {
			if (is_between(i, sel_y, sel_end_y))
				bg_color = HEADER_CELL_COLOR_ACTIVE;
			else
				bg_color = HEADER_CELL_COLOR;
			_ksys_draw_bar(0, y - y0, grid.w, 1, GRID_COLOR);
			DrawCell(0, y - y0 + 1, cell_w[0], cell_h[i] - 1, i + ROW_HEAD_BUTTON, bg_color, cells[0][i], 1);
			cell_y[i] = y - y0;
		} else
			y0 += cell_h[i];
		y += cell_h[i];
		ny++;
	}
	bottom_edge = y - y0; // bottom edge of the last visible row

	// blank out any strip past the last column/row so nothing stale shows
	if (right_edge < grid.w)
		_ksys_draw_bar(right_edge, 0, grid.w - right_edge, grid.h, CELL_COLOR);
	if (bottom_edge < grid.h)
		_ksys_draw_bar(0, bottom_edge, grid.w, grid.h - bottom_edge, CELL_COLOR);

	// cells themselves
	y = cell_h[0];
	for (i = grid.firsty; i < ny; i++) {
		x = cell_w[0];
		for (j = grid.firstx; j < nx; j++) {
			if (i && j) { // headers already drawn
				int draw_frame_selection = 0;
				int error = 0;
				char *text;
				bg_color = CELL_COLOR;

				if (is_between(j, sel_x, sel_end_x) && is_between(i, sel_y, sel_end_y)
				    && ((!sel_moved) || (is_x_changed(j) && is_y_changed(i)))) {
					if (i == sel_y && j == sel_x) {
						draw_frame_selection = 1;
						drag_x = x + cell_w[j] - 4;
						drag_y = y + cell_h[i] - 4;
					} else
						bg_color = CELL_COLOR_ACTIVE; // selected but not main
				}

				if (values[j][i] && values[j][i][0] == '#') {
					text = cells[j][i];
					error = 1;
				} else
					text = (values[j][i] && !display_formulas ? values[j][i] : cells[j][i]);

				DrawCell(x + 1, y + 1, cell_w[j] - 1, cell_h[i] - 1, 0, bg_color, text, 0);
				if (draw_frame_selection && j < nx - 1 && i < ny - 1)
					DrawSelectedFrame(x + 1, y, cell_w[j] - 1, cell_h[i] + 1, TEXT_COLOR);
				else if (error)
					draw_region(x + 1, y + 1, cell_w[j] - 1, cell_h[i] - 1, 0xff0000);
			}
			x += cell_w[j];
		}
		y += cell_h[i];
	}
	DrawScrolls();
}

// very fast grid draw while resizing cells (XOR lines)
static void draw_size_grid(void)
{
	if (size_state == SIZE_X) {
		int x, x0, i;
		x = cell_w[0];
		x0 = 0;
		for (i = 1; i < col_count && x - x0 + cell_w[i] < grid.w - 10; i++) {
			if (i >= grid.firstx) {
				if (i >= size_id)
					draw_line(x - x0, 0, x - x0, grid.h, 0, 1);
			} else
				x0 += cell_w[i];
			x += cell_w[i];
		}
		draw_line(x - x0, 0, x - x0, grid.h, 0, 1);
	} else {
		int y, y0, i;
		y = cell_h[0];
		y0 = 0;
		for (i = 1; i < col_count && y - y0 + cell_h[i] < grid.h - 10; i++) {
			if (i >= grid.firsty) {
				if (i >= size_id)
					draw_line(0, y - y0, grid.w, y - y0, 0, 1);
			} else
				y0 += cell_h[i];
			y += cell_h[i];
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

	int x0 = cell_x[k0] - 1;
	int x1 = cell_x[k1] + cell_w[k1] + 1;
	int y0 = cell_y[n0] - 1;
	int y1 = cell_y[n1] + cell_h[n1] + 1;
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
	_ksys_create_window(110, 40, WND_W, WND_H, "Table v" TABLE_VERSION, 0x40FFFFFF, 0x73);
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

	sel_moved = 0;
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

	if (sel_end_move)
		sel_moved = 0;
	draw_grid();
	sel_moved = 0;

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
		grid.firsty += vert;
		if (grid.firsty < 1)
			grid.firsty = 1;
		draw_grid(); // clamp_view() caps the lower edge
		return;
	}

	if (!size_state) { // do not handle scrollbars while selecting cells
		if (!scroll_h.delta2)
			scrollbar_v_mouse(&scroll_v);
		if (scroll_v.position != grid.firsty - 1) {
			stop_edit();
			grid.firsty = scroll_v.position + 1;
			draw_grid();
		}

		if (!scroll_v.delta2)
			scrollbar_h_mouse(&scroll_h);
		if (scroll_h.position != grid.firstx - 1) {
			stop_edit();
			grid.firstx = scroll_h.position + 1;
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
		} else if (mouse_y <= cell_h[0]) {
			int kx = -1;
			for (i = 0; i < col_count - 1; i++)
				if (mouse_x >= cell_x[i] + cell_w[i] - 5 && mouse_x <= cell_x[i + 1] + 5) {
					kx = i;
					break;
				}
			if (kx != -1) {
				size_id = kx;
				size_state = SIZE_X;
			}
		} else if (mouse_x <= cell_w[0]) {
			int ky = -1;
			for (i = 0; i < row_count - 1; i++)
				if (mouse_y >= cell_y[i] + cell_h[i] - 5 && mouse_y <= cell_y[i + 1] + 5) {
					ky = i;
					break;
				}
			if (ky != -1) {
				size_id = ky;
				size_state = SIZE_Y;
			}
		} else if (mouse_x <= cell_x[nx - 1] && mouse_y <= cell_y[ny - 1]) { // click on cell
			int kx = -1, ky = -1;
			was_single_selection = sel_x == sel_end_x && sel_y == sel_end_y;
			for (i = 0; i < col_count - 1; i++)
				if (mouse_x >= cell_x[i] && mouse_x <= cell_x[i] + cell_w[i]) {
					kx = i;
					break;
				}
			for (i = 0; i < row_count - 1; i++)
				if (mouse_y >= cell_y[i] && mouse_y <= cell_y[i] + cell_h[i]) {
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
		sel_moved = 0; // for a good redraw

		if (size_state == SIZE_DRAG)
			fill_cells(sel_x, sel_y, sel_end_x, sel_end_y, old_end_x, old_end_y);

		size_state = 0;
		draw_grid(); // everything shifted - refresh
		return;
	}
	if (size_state == SIZE_X && mouse_x != size_mouse_x) {
		draw_size_grid();
		cell_w[size_id] += mouse_x - size_mouse_x;
		if (cell_w[size_id] < 15)
			cell_w[size_id] = 15;
		else if (cell_w[size_id] > grid.w / 2)
			cell_w[size_id] = grid.w / 2;
		draw_size_grid();
	}
	if (size_state == SIZE_Y && mouse_y != size_mouse_y) {
		draw_size_grid();
		cell_h[size_id] += mouse_y - size_mouse_y;
		if (cell_h[size_id] < 15)
			cell_h[size_id] = 15;
		else if (cell_h[size_id] > grid.h / 2)
			cell_h[size_id] = grid.h / 2;
		draw_size_grid();
	}
	if ((size_state == SIZE_SELECT || size_state == SIZE_DRAG) && (mouse_x != size_mouse_x || mouse_y != size_mouse_y)) {
		int kx = -1, ky = -1;
		draw_drag();
		for (i = 0; i < col_count - 1; i++)
			if (mouse_x >= cell_x[i] && mouse_x <= cell_x[i + 1]) {
				kx = i;
				break;
			}
		for (i = 0; i < row_count - 1; i++)
			if (mouse_y >= cell_y[i] && mouse_y <= cell_y[i + 1]) {
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
		else if (sel_end_x >= col_count)
			sel_end_x = col_count - 1;
		sel_moved = sel_end_move = 1;
	}
	if (dy != 0 && shift) {
		sel_end_y += dy;
		if (sel_end_y <= 1)
			sel_end_y = 1;
		else if (sel_end_y >= row_count)
			sel_end_y = row_count - 1;
		sel_moved = sel_end_move = 1;
	}
	if (dx || dy) {
		if (!shift) {
			if ((sel_end_x + dx) >= (col_count - 1)) {
				dx = 0;
			} else if ((sel_end_y + dy) >= (row_count - 1)) {
				dy = 0;
			} else
				move_selection(sel_x + dx, sel_y + dy);
		} else {
			sel_moved = 0;
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
	sel_moved = 0;
	sel_end_move = 0;

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
					if (i + x0 >= col_count || j + y0 >= row_count)
						continue;
					if (cells[i + x0][j + y0])
						free(cells[i + x0][j + y0]);
					if (buffer[i][j]) {
						cf_x0 = buf_old_x;
						cf_y0 = buf_old_y;
						cf_x1 = buf_old_x + buf_col;
						cf_y1 = buf_old_y + buf_row;
						cells[i + x0][j + y0] = change_formula(buffer[i][j], delta_x, delta_y);
					} else
						cells[i + x0][j + y0] = NULL;
				}

			calculate_values();
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
					if (cells[i + x0][j + y0]) {
						if (KSYS_SCANCODE_C == key_scancode) {
							buffer[i][j] = malloc(strlen(cells[i + x0][j + y0]) + 1);
							strcpy(buffer[i][j], cells[i + x0][j + y0]);
						} else {
							buffer[i][j] = cells[i + x0][j + y0];
							cells[i + x0][j + y0] = NULL;
						}
					} else
						buffer[i][j] = NULL;
				}
			}
			if (key_ascii == 24)
				calculate_values();
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
			shift_selection(col_count - (nx - grid.firstx) - 1 - sel_x, 0, shift);
			break;
		case KSYS_SCANCODE_EXT_DELETE: {
			int i, j;
			int n0 = min(sel_x, sel_end_x);
			int n1 = max(sel_x, sel_end_x);
			int k0 = min(sel_y, sel_end_y);
			int k1 = max(sel_y, sel_end_y);

			for (i = n0; i <= n1; i++)
				for (j = k0; j <= k1; j++)
					if (cells[i][j]) {
						free(cells[i][j]);
						cells[i][j] = NULL;
					}
			calculate_values();
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
	r = LoadFile(fname);
	if (r > 0) {
		calculate_values();
		sel_moved = 0;
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
	sel_end_x = col_count - 1;
	sel_end_y = row_count - 1;
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
		reinit();
		draw_grid();
		break;

	case SAVE_BUTTON:
		stop_edit();
		if (SaveFile(fname))
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
		sel_end_y = row_count - 1;
		stop_edit();
		draw_grid();
		return;
	} else if (button >= ROW_HEAD_BUTTON && button < CELL_BUTTON) {
		sel_end_y = sel_y = button - ROW_HEAD_BUTTON;
		sel_x = 1;
		sel_end_x = col_count - 1;
		stop_edit();
		draw_grid();
		return;
	}
}

int main(int argc, char **argv)
{
	init();
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
