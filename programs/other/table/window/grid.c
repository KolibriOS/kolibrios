// The grid: where its cells are on the screen for the scroll, the
// selection, and the drawing of it all with its headers and scrollbars.

#include "window.h"
#include "../sheet/refs.h"

#include <stdio.h>
#include <string.h>

uint32_t mid_color; // between work_light and work: headers, scrollbars
static uint32_t sel_color; // work_light a quarter of the way to work_button: the selection

int grid_right, grid_bottom;
int sel_x = 1, sel_y = 1, end_x = 1, end_y = 1;
int show_formulas;
int scroll_x, scroll_y;
int first_x, last_x, first_y, last_y;
int col_x[COLS + 1], row_y[ROWS + 1];

// box_lib draws a vertical scrollbar xsize + 1 wide and ysize high, a
// horizontal one xsize wide and ysize + 1 high, a button btn_height + 1
scrollbar scroll_v = { .xsize = SCROLL_SIZE - 1, .btn_height = SCROLL_SIZE - 1, .ar_offset = ARROW_STEP };
scrollbar scroll_h = { .ysize = SCROLL_SIZE - 1, .btn_height = SCROLL_SIZE - 1, .ar_offset = ARROW_STEP };

/* ============================ geometry ============================ */

int clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

// pixels from the first column (or row) to item n of sizes
static int offset_of(const int *size, int n)
{
	int i, s = 0;

	for (i = 1; i < n; i++)
		s += size[i];
	return s;
}

// one direction of the grid: of count items scrolled by scroll, the first
// and last shown between screen pixels start and end, and where they are
static void layout_axis(const int *size, int count, int scroll, int start, int end,
                        int *pos, int *first, int *last)
{
	int i = 1, s = 0;

	while (i < count && s + size[i] <= scroll)
		s += size[i++];
	*first = i;
	s = start - (scroll - s);
	for (;;) {
		pos[i] = s;
		s += size[i];
		if (i == count || s >= end)
			break;
		i++;
	}
	*last = i;
}

// a column as wide as the widest text it shows with its grid line on the
// left (the first one shown is cut at the line of the headers), an empty
// one as a new one
int fit_width(int x)
{
	char buf[32];
	const char *s;
	int y, len = 0;

	for (y = 1; y <= ROWS; y++) {
		const char *text = cells[x][y].text;
		// a formula shows a number: once the column is as wide as the
		// longest one, it is not made
		if (!text || (text[0] == '=' && !show_formulas && len >= NUMBER_MAX))
			continue;
		s = cell_show(x, y, show_formulas, buf);
		len = max(len, (int)strlen(s));
	}
	return len ? clamp(len * FONT_W + 2 * CELL_PAD + 1, MIN_SIZE, MAX_SIZE) : COL_W;
}

// the table and the view in pixels of the cells, the scrollbars' areas;
// the scroll kept inside the table, then the cells shown
void layout_grid(void)
{
	scroll_h.max_area = offset_of(col_w, COLS + 1);
	scroll_h.cur_area = VIEW_W;
	scroll_v.max_area = offset_of(row_h, ROWS + 1);
	scroll_v.cur_area = VIEW_H;
	scroll_x = clamp(scroll_x, 0, max(0, scroll_h.max_area - scroll_h.cur_area));
	scroll_y = clamp(scroll_y, 0, max(0, scroll_v.max_area - scroll_v.cur_area));
	layout_axis(col_w, COLS, scroll_x, CELLS_X, grid_right, col_x, &first_x, &last_x);
	layout_axis(row_h, ROWS, scroll_y, CELLS_Y, grid_bottom, row_y, &first_y, &last_y);
}

// one direction: the scroll moved the least so that item n of sizes is
// wholly in a view of view pixels; one larger than the view from its start
static int scroll_into(int scroll, const int *size, int n, int view)
{
	int start = offset_of(size, n);

	if (start < scroll || size[n] > view)
		return start;
	if (start + size[n] > scroll + view)
		return start + size[n] - view;
	return scroll;
}

// scroll the least so that cell x, y is wholly in view
void scroll_to(int x, int y)
{
	scroll_x = scroll_into(scroll_x, col_w, x, VIEW_W);
	scroll_y = scroll_into(scroll_y, row_h, y, VIEW_H);
}

// the item of first..last under screen pixel m, 0 for none
static int item_at(const int *pos, const int *size, int first, int last, int m)
{
	int i;

	for (i = first; i <= last; i++)
		if (m >= pos[i] && m < pos[i] + size[i])
			return i;
	return 0;
}

// the cell under a point, the point first moved into the cells
void cell_under(int mx, int my, int *x, int *y)
{
	*x = item_at(col_x, col_w, first_x, last_x, clamp(mx, CELLS_X, grid_right - 1));
	*y = item_at(row_y, row_h, first_y, last_y, clamp(my, CELLS_Y, grid_bottom - 1));
}

// the grid line on the screen left of column x, or over row y, wherever
// the scroll puts it; x + 1 the one right of it, y + 1 under it
int col_line(int x)
{
	return CELLS_X + offset_of(col_w, x) - scroll_x;
}

int row_line(int y)
{
	return CELLS_Y + offset_of(row_h, y) - scroll_y;
}

// the fill handle at right, bottom is in the cells shown
int handle_shown(int right, int bottom)
{
	return in_rect(right, bottom, CELLS_X, CELLS_Y, grid_right, grid_bottom);
}

// the selection with x0 <= x1, y0 <= y1
void sel_rect(int *x0, int *y0, int *x1, int *y1)
{
	*x0 = min(sel_x, end_x);
	*x1 = max(sel_x, end_x);
	*y0 = min(sel_y, end_y);
	*y1 = max(sel_y, end_y);
}

/* ============================= drawing ============================ */

// a bar cut to the grid, x0 and y0 the top left it may not cross
static void bar_clip(int x, int y, int w, int h, int x0, int y0, uint32_t color)
{
	if (x < x0) {
		w -= x0 - x;
		x = x0;
	}
	if (y < y0) {
		h -= y0 - y;
		y = y0;
	}
	w = min(w, grid_right - x);
	h = min(h, grid_bottom - y);
	if (w > 0 && h > 0)
		_ksys_draw_bar(x, y, w, h, color);
}

// a bar cut to the cells, off the lines right of and under the headers:
// they are drawn last
void cell_bar(int x, int y, int w, int h, uint32_t color)
{
	bar_clip(x, y, w, h, CELLS_X + 1, CELLS_Y + 1, color);
}

// a box of the grid: its lines on the top and left, inside it colour; cut
// at x0, y0, inside it off the lines there: they are drawn last, and so do
// not flicker
static void draw_box(int x, int y, int w, int h, int x0, int y0, uint32_t color)
{
	bar_clip(x, y, w, 1, x0, y0, sc.work_graph);
	bar_clip(x, y, 1, h, x0, y0, sc.work_graph);
	bar_clip(x + 1, y + 1, w - 1, h - 1, x0 + 1, y0 + 1, color);
}

// len characters at x, their capitals in the middle of a row at y of
// height h under its grid line: the whole characters from left to the
// right of the grid, if their rows are all from top to its bottom
static void draw_label(int x, int y, int h, const char *s, int len, int left, int top, uint32_t color)
{
	int ty = y + 1 + CAPS_Y(h - 1); // under the grid line

	if (x < left) { // the characters cut at the left skipped
		int skip = (left - x + FONT_W - 1) / FONT_W;
		s += skip;
		len -= skip;
		x += skip * FONT_W;
	}
	len = min(len, (grid_right - x) / FONT_W);
	if (len > 0 && ty >= top && ty + FONT_H <= grid_bottom)
		_ksys_draw_text(s, x, ty, len, FONT_CP866 | color);
}

// a text in the middle of a box of width w at x, y, height h, seen from
// left on
void draw_centred(int x, int y, int w, int h, const char *s, int left, uint32_t color)
{
	int len = strlen(s);

	draw_label(x + 1 + (w - 1 - len * FONT_W) / 2, y, h, s, len, left, y, color);
}

// a text in a cell, or a row header, of width w at x, y, height h, seen
// from left on: a number right aligned in the part seen, "###" if it does
// not fit there; else left aligned and cut at the cell's end
static void draw_cell_text(int x, int y, int w, int h, const char *s, int number, int left, uint32_t color)
{
	static const char hashes[] = "################################"; // longer than a number
	int len = strlen(s), fit;

	if (number) { // the box only its part seen
		int right = min(x + w, grid_right);
		x = max(x, left);
		w = right - x;
	}
	fit = (w - 2 * CELL_PAD) / FONT_W;
	if (len > fit) {
		len = fit;
		if (number) {
			s = hashes;
			len = min(fit, (int)sizeof(hashes) - 1);
		}
	}
	draw_label(number ? x + w - CELL_PAD - len * FONT_W : x + CELL_PAD, y, h, s, len, left, CELLS_Y + 1, color);
}

// the selection as drawn, x0 <= x1, y0 <= y1: while the fill handle is
// dragged, the cells the fill would cover; the scroll and the cells it was
// drawn with
static int drawn_x0, drawn_y0, drawn_x1, drawn_y1;
static int drawn_scroll_x, drawn_scroll_y, drawn_recalcs;
static int drawn_editor; // the box over the cell, which may stand out of a short row

static void take_selection(void)
{
	if (drag == DRAG_FILL)
		fill_target(&drawn_x0, &drawn_y0, &drawn_x1, &drawn_y1);
	else
		sel_rect(&drawn_x0, &drawn_y0, &drawn_x1, &drawn_y1);
	drawn_scroll_x = scroll_x;
	drawn_scroll_y = scroll_y;
	drawn_recalcs = table_recalcs;
	drawn_editor = editor != NULL;
}

// the cells x0..x1, y0..y1, shown ones, with their lines on the left and
// top, cut at the headers
static void draw_cells(int x0, int y0, int x1, int y1)
{
	int x, y;
	char buf[32];

	for (y = y0; y <= y1; y++)
		for (x = x0; x <= x1; x++) {
			int px = col_x[x], py = row_y[y], w = col_w[x], h = row_h[y];
			const char *s;
			int in_sel = x >= drawn_x0 && x <= drawn_x1 && y >= drawn_y0 && y <= drawn_y1
			             && !(x == sel_x && y == sel_y);

			draw_box(px, py, w, h, CELLS_X, CELLS_Y, in_sel ? sel_color : sc.work_light);
			s = cell_show(x, y, show_formulas, buf);
			if (s)
				draw_cell_text(px, py, w, h, s, cells[x][y].kind == VALUE_NUMBER && !show_formulas,
				               CELLS_X + 1, sc.work_text);
		}
}

// the frame of the selection, 2 pixels on its outer grid lines, the fill
// handle at its corner, as the grid was drawn
void draw_frame(void)
{
	int left = col_line(drawn_x0), top = row_line(drawn_y0);
	int right = col_line(drawn_x1 + 1), bottom = row_line(drawn_y1 + 1);

	cell_bar(left, top, right - left + 1, 2, sc.work_button);
	cell_bar(left, bottom - 1, right - left + 1, 2, sc.work_button);
	cell_bar(left, top, 2, bottom - top + 1, sc.work_button);
	cell_bar(right - 1, top, 2, bottom - top + 1, sc.work_button);
	if (handle_shown(right, bottom))
		cell_bar(right - HANDLE / 2, bottom - HANDLE / 2, HANDLE, HANDLE, sc.work_button);
}

// the headers of the columns x0..x1 and of the rows y0..y1, shown ones,
// the selected ones in the button colour
static void draw_headers(int x0, int y0, int x1, int y1)
{
	int x, y;
	char buf[8];

	for (x = x0; x <= x1; x++) {
		int px = col_x[x], w = col_w[x], sel = x >= drawn_x0 && x <= drawn_x1;
		draw_box(px, GRID_Y, w, ROW_H, CELLS_X, GRID_Y, sel ? sc.work_button : mid_color);
		col_name(x, buf);
		draw_centred(px, GRID_Y, w, ROW_H, buf, CELLS_X + 1,
		             sel ? sc.work_button_text : sc.work_text);
	}
	for (y = y0; y <= y1; y++) {
		int py = row_y[y], h = row_h[y], sel = y >= drawn_y0 && y <= drawn_y1;
		draw_box(GRID_X, py, HEADER_W, h, GRID_X, CELLS_Y, sel ? sc.work_button : mid_color);
		sprintf(buf, "%d", y);
		draw_cell_text(GRID_X, py, HEADER_W, h, buf, 1, GRID_X,
		               sel ? sc.work_button_text : sc.work_text);
	}
}

void draw_grid(void)
{
	if (editor) // the box over the cell stays on it
		scroll_to(sel_x, sel_y);
	layout_grid();
	take_selection();

	draw_cells(first_x, first_y, last_x, last_y);
	draw_frame();
	draw_box(GRID_X, GRID_Y, HEADER_W, ROW_H, GRID_X, GRID_Y, mid_color); // the corner
	draw_headers(first_x, first_y, last_x, last_y);

	// the lines right of and under the headers; the headers draw the outer
	// lines on the top and left, the scrollbars the others
	_ksys_draw_bar(CELLS_X, GRID_Y, 1, grid_bottom - GRID_Y, sc.work_graph);
	_ksys_draw_bar(GRID_X, CELLS_Y, grid_right - GRID_X, 1, sc.work_graph);

	// the scrollbars, their lines on the outer lines of the grid
	scroll_h.xpos = GRID_X;
	scroll_h.ypos = grid_bottom;
	scroll_h.xsize = grid_right - GRID_X + 1;
	scroll_h.position = scroll_x;
	scroll_h.all_redraw = 1;
	scrollbar_h_draw(&scroll_h);

	scroll_v.xpos = grid_right;
	scroll_v.ypos = GRID_Y;
	scroll_v.ysize = grid_bottom - GRID_Y + 1;
	scroll_v.position = scroll_y;
	scroll_v.all_redraw = 1;
	scrollbar_v_draw(&scroll_v);

	// the square between the scrollbars, framed like them: its top and left
	// lines are theirs, drawn again
	_ksys_draw_bar(grid_right + 1, grid_bottom + 1, SCROLL_SIZE - 2, SCROLL_SIZE - 2, sc.work);
	draw_bevel(grid_right, grid_bottom, SCROLL_SIZE, SCROLL_SIZE, sc.work_graph, sc.work_graph);

	edit_draw();
}

void draw_selection(void)
{
	int x0 = drawn_x0, y0 = drawn_y0, x1 = drawn_x1, y1 = drawn_y1; // the one drawn

	layout_grid();
	if (editor || drawn_editor || scroll_x != drawn_scroll_x || scroll_y != drawn_scroll_y
	    || table_recalcs != drawn_recalcs) {
		draw_grid();
		return;
	}
	take_selection();
	// the cells under either selection, and the column and row after them:
	// the frame and the fill handle reach into those; the ones shown
	x0 = max(min(x0, drawn_x0), first_x);
	y0 = max(min(y0, drawn_y0), first_y);
	x1 = min(max(x1, drawn_x1) + 1, last_x);
	y1 = min(max(y1, drawn_y1) + 1, last_y);
	draw_cells(x0, y0, x1, y1);
	draw_frame();
	draw_headers(x0, y0, x1, y1);
	edit_draw();
}

// colour a with quarters of colour b in it, channel by channel
static uint32_t mix(uint32_t a, uint32_t b, int quarters)
{
	uint32_t c = 0;
	int k;

	for (k = 0; k < 24; k += 8) // blue, green, red
		c |= (((4 - quarters) * (a >> k & 0xFF) + quarters * (b >> k & 0xFF)) / 4) << k;
	return c;
}

// the system colours and the ones made of them
void read_colors(void)
{
	read_skin();
	mid_color = mix(sc.work_light, sc.work, 2);
	sel_color = mix(sc.work_light, sc.work_button, 1);
	scroll_h.back_color = scroll_v.back_color = sc.work;
	scroll_h.front_color = scroll_v.front_color = mid_color;
	scroll_h.line_color = scroll_v.line_color = sc.work_graph;
}
