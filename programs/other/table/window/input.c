// The input: the mouse on the grid, selecting, resizing columns and rows
// by their header lines, the fill handle, the scrollbars, the edit boxes,
// the menu of the right button; the keyboard, moving and selecting,
// editing, the commands.

#include "window.h"
#include "../sheet/changes.h"
#include "../sheet/undo.h"

#include <string.h>

/* ============================== mouse ============================= */

#define WHEEL_STEP   (3 * ROW_H) // pixels a wheel notch scrolls
#define RESIZE_ZONE  4          // pixels either side of a header line that resize
#define LINE_INVERT  0x01000000 // function 38: the line inverts what is under it
#define POINTER_SIZE 32         // function 37.4: a pointer is 32x32 ARGB

int drag;
static int drag_index, drag_pos; // the column or row resized, the line drawn
static int selecting; // what a drag of the selection began on: ON_*
static int fill_x, fill_y;       // the cell the fill handle is dragged to

// the mouse pointers over a header line (resize) and the fill handle, in
// the look of the kernel's: '#' black, 'o' white, '.' clear; rows of a
// width one after the other
#define POINTER_COL_W 21
static const char pointer_col[] =
	"....#...........#...."
	"...##...........##..."
	"..#o#...........#o#.."
	".#oo#############oo#."
	"#ooooooooooooooooooo#"
	".#oo#############oo#."
	"..#o#...........#o#.."
	"...##...........##..."
	"....#...........#....";
#define POINTER_ROW_W 9
static const char pointer_row[] =
	"....#...."
	"...#o#..."
	"..#ooo#.."
	".#ooooo#."
	"####o####"
	"...#o#..."
	"...#o#..."
	"...#o#..."
	"...#o#..."
	"...#o#..."
	"...#o#..."
	"...#o#..."
	"...#o#..."
	"...#o#..."
	"...#o#..."
	"...#o#..."
	"####o####"
	".#ooooo#."
	"..#ooo#.."
	"...#o#..."
	"....#....";
#define POINTER_FILL_W 11
static const char pointer_fill[] =
	"....###...."
	"....#o#...."
	"....#o#...."
	"....#o#...."
	"#####o#####"
	"#ooooooooo#"
	"#####o#####"
	"....#o#...."
	"....#o#...."
	"....#o#...."
	"....###....";
static void *cursor_col, *cursor_row, *cursor_fill;

// an inverted line across the grid at drag_pos, for resizing: drawn again,
// it is gone
void draw_drag_line(void)
{
	if (drag == DRAG_COL)
		_ksys_draw_line(drag_pos, GRID_Y, drag_pos, grid_bottom - 1, LINE_INVERT);
	else
		_ksys_draw_line(GRID_X, drag_pos, grid_right - 1, drag_pos, LINE_INVERT);
}

// a mouse pointer from a picture of rows w wide, its hot spot in the middle
static void *make_pointer(const char *picture, int w, int h)
{
	static uint32_t argb[POINTER_SIZE * POINTER_SIZE]; // the kernel takes a copy
	int x, y;

	memset(argb, 0, sizeof(argb));
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			if (picture[y * w + x] != '.')
				argb[y * POINTER_SIZE + x] = picture[y * w + x] == '#' ? 0xFF000000 : 0xFFFFFFFF;
	// function 37.4: the hot spot x in bits 24..31, y in 16..23
	return _ksys_load_cursor(argb, KSYS_CURSOR_INDIRECT | (w / 2) << 24 | (h / 2) << 16);
}

// the item of first..last whose end line is within RESIZE_ZONE of screen
// pixel m; 0 for none
static int edge_at(const int *pos, const int *size, int first, int last, int m)
{
	int i;

	for (i = first; i <= last; i++)
		if (abs(m - (pos[i] + size[i])) <= RESIZE_ZONE)
			return i;
	return 0;
}

// what the fill handle dragged to fill_x, fill_y covers: past the
// selection it grows to that cell, inside it shrinks to it
void fill_target(int *tx0, int *ty0, int *tx1, int *ty1)
{
	int x0, y0, x1, y1;

	sel_rect(&x0, &y0, &x1, &y1);
	*tx0 = min(x0, fill_x);
	*tx1 = fill_x < x0 ? x1 : fill_x;
	*ty0 = min(y0, fill_y);
	*ty1 = fill_y < y0 ? y1 : fill_y;
}

// the column whose header line is at a point, for resizing; 0 for none
static int col_edge_at(int mx, int my)
{
	if (!in_rect(mx, my, CELLS_X, GRID_Y, grid_right, CELLS_Y))
		return 0;
	return edge_at(col_x, col_w, first_x, last_x, mx);
}

static int row_edge_at(int mx, int my)
{
	if (!in_rect(mx, my, GRID_X, CELLS_Y, CELLS_X, grid_bottom))
		return 0;
	return edge_at(row_y, row_h, first_y, last_y, my);
}

// a point on the fill handle or next to it
static int on_handle(int mx, int my)
{
	int x0, y0, x1, y1, right, bottom;

	sel_rect(&x0, &y0, &x1, &y1);
	right = col_line(x1 + 1);
	bottom = row_line(y1 + 1);
	return handle_shown(right, bottom) && abs(mx - right) <= HANDLE && abs(my - bottom) <= HANDLE;
}

// the pointer for what a drag does, or would do at a point: the system
// one for anything else
static void set_pointer(int mx, int my)
{
	static void *current;
	void *wanted = NULL;
	int hover = drag == DRAG_NONE;

	if (drag == DRAG_COL || (hover && col_edge_at(mx, my)))
		wanted = cursor_col;
	else if (drag == DRAG_ROW || (hover && row_edge_at(mx, my)))
		wanted = cursor_row;
	else if (drag == DRAG_FILL || (hover && on_handle(mx, my)))
		wanted = cursor_fill;
	if (wanted != current) {
		_ksys_set_cursor(wanted);
		current = wanted;
	}
}

static void start_resize(int kind, int index, int pos)
{
	stop_edit(); // its box would stay where the column or row was
	draw_grid();
	drag = kind;
	drag_index = index;
	drag_pos = pos;
	draw_drag_line();
}

// a column or a row to a size, in the limits a file holds; undoable
static void set_size(int *size, int i, int v)
{
	v = clamp(v, MIN_SIZE, MAX_SIZE);
	stop_edit();
	if (size[i] != v) {
		undo_save_size(size, i);
		size[i] = v;
	}
	draw_grid();
}

// what a point of the grid is on: ON_*
static int part_at(int mx, int my)
{
	if (my < CELLS_Y)
		return ON_COLS;
	return mx < CELLS_X ? ON_ROWS : ON_CELLS;
}

static void mouse_down(int mx, int my, int shift, int twice)
{
	int x, y, i;

	if (!in_rect(mx, my, GRID_X, GRID_Y, grid_right, grid_bottom))
		return;

	// a header line resizes, a double click on it fits the column to its
	// texts or the row to the usual height; the fill handle fills
	if ((i = col_edge_at(mx, my)) != 0) {
		if (twice)
			set_size(col_w, i, fit_width(i));
		else
			start_resize(DRAG_COL, i, col_x[i] + col_w[i]); // from the line, not the mouse
		return;
	}
	if ((i = row_edge_at(mx, my)) != 0) {
		if (twice)
			set_size(row_h, i, ROW_H);
		else
			start_resize(DRAG_ROW, i, row_y[i] + row_h[i]);
		return;
	}
	if (on_handle(mx, my)) {
		stop_edit();
		drag = DRAG_FILL;
		fill_x = max(sel_x, end_x); // at the handle: the selection itself
		fill_y = max(sel_y, end_y);
		draw_grid(); // an edited text in its cell, without its box
		return;
	}

	// the corner selects all; a header its column or row, with Shift from
	// the active one on, and a drag over the headers more of them
	if (my < CELLS_Y && mx < CELLS_X) {
		select_range(1, 1, COLS, ROWS);
		return;
	}
	cell_under(mx, my, &x, &y);
	if (!x || !y)
		return;
	selecting = part_at(mx, my);
	if (selecting == ON_COLS)
		select_range(shift ? sel_x : x, 1, x, ROWS);
	else if (selecting == ON_ROWS)
		select_range(1, shift ? sel_y : y, COLS, y);
	else if (shift)
		select_range(sel_x, sel_y, x, y);
	else if (twice && !editor && x == sel_x && y == sel_y) {
		start_edit(1, &cell_box); // the first click chose the cell
		return;
	} else if (x == sel_x && y == sel_y && editor)
		return; // the cell being edited, out of its box: the editing goes on
	else
		select_cell(x, y);
	drag = DRAG_SELECT;
}

// a selection or a fill dragged to the cell under a point (moved into the
// cells); 1 if it moved
static int drag_to(int mx, int my)
{
	int x, y;

	cell_under(mx, my, &x, &y);
	if (!x || !y)
		return 0;
	if (drag == DRAG_SELECT && selecting == ON_COLS)
		y = ROWS;
	if (drag == DRAG_SELECT && selecting == ON_ROWS)
		x = COLS;
	if (drag == DRAG_SELECT && (x != end_x || y != end_y)) {
		end_x = x;
		end_y = y;
		return 1;
	}
	if (drag == DRAG_FILL && (x != fill_x || y != fill_y)) {
		fill_x = x;
		fill_y = y;
		return 1;
	}
	return 0;
}

static void mouse_move(int mx, int my)
{
	if (drag == DRAG_COL || drag == DRAG_ROW) {
		draw_drag_line();
		drag_pos = drag == DRAG_COL ? clamp(mx, col_x[drag_index] + MIN_SIZE, grid_right - 1)
		                            : clamp(my, row_y[drag_index] + MIN_SIZE, grid_bottom - 1);
		draw_drag_line();
	} else if (drag_to(mx, my))
		draw_selection();
}

// a selection or a fill held past an edge of the cells scrolls on, a step
// each SCROLL_TICK the main loop waits
static void auto_scroll(int mx, int my)
{
	int dx = mx < CELLS_X ? -1 : (mx >= grid_right ? 1 : 0);
	int dy = my < CELLS_Y ? -1 : (my >= grid_bottom ? 1 : 0);
	int old_x = scroll_x, old_y = scroll_y;

	// whole columns from their headers scroll across only, rows down only
	if (drag == DRAG_SELECT && selecting == ON_COLS)
		dy = 0;
	if (drag == DRAG_SELECT && selecting == ON_ROWS)
		dx = 0;
	if (!dx && !dy)
		return;
	scroll_x += dx * ARROW_STEP;
	scroll_y += dy * ARROW_STEP;
	layout_grid(); // at an end of the table: as it was
	drag_to(mx, my);
	if (scroll_x != old_x || scroll_y != old_y) // the mouse moving redraws the rest
		draw_grid();
}

static void mouse_up(void)
{
	int x0, y0, x1, y1, tx0, ty0, tx1, ty1;
	int ended = drag;

	if (drag == DRAG_COL || drag == DRAG_ROW)
		draw_drag_line(); // away: it may cross more than the grid
	drag = DRAG_NONE; // before the grid is drawn: no fill shown
	if (ended == DRAG_COL)
		set_size(col_w, drag_index, drag_pos - col_x[drag_index]);
	else if (ended == DRAG_ROW)
		set_size(row_h, drag_index, drag_pos - row_y[drag_index]);
	else if (ended == DRAG_FILL) {
		// the selection the source, what the fill covers the new selection
		sel_rect(&x0, &y0, &x1, &y1);
		fill_target(&tx0, &ty0, &tx1, &ty1);
		if (tx0 != x0 || ty0 != y0 || tx1 != x1 || ty1 != y1) { // not let go at the handle
			table_fill(x0, y0, x1, y1, tx0, ty0, tx1, ty1);
			select_range(tx0, ty0, tx1, ty1);
		}
	}
}

void process_mouse(void)
{
	static int was_down;
	ksys_pos_t m = _ksys_get_mouse_pos(KSYS_MOUSE_WINDOW_POS);
	uint32_t buttons = _ksys_get_mouse_eventstate();
	int down = buttons & KSYS_MOUSE_LBUTTON_PRESSED, pressed = down && !was_down;
	int wheel = (int16_t)_ksys_get_mouse_wheels(); // the low word: the vertical wheel

	toolbar_mouse(m.x, m.y);
	set_pointer(m.x, m.y);

	// the right button on the cells or on the headers, not the corner: the
	// menu, for the selection, or for the cell, column or row clicked out of it
	if ((buttons & MOUSE_RIGHT_CLICK) && drag == DRAG_NONE
	    && in_rect(m.x, m.y, GRID_X, GRID_Y, grid_right, grid_bottom) && (m.x >= CELLS_X || m.y >= CELLS_Y)) {
		int target = part_at(m.x, m.y);
		int x, y, x0, y0, x1, y1, cx0, cy0, cx1, cy1;
		cell_under(m.x, m.y, &x, &y);
		if (!x || !y)
			return;
		stop_edit();
		// what the click is on: the cell, or its column or row; selected
		// unless the selection has it
		cx0 = cx1 = x;
		cy0 = cy1 = y;
		if (target == ON_ROWS) {
			cx0 = 1;
			cx1 = COLS;
		}
		if (target == ON_COLS) {
			cy0 = 1;
			cy1 = ROWS;
		}
		sel_rect(&x0, &y0, &x1, &y1);
		if (cx0 < x0 || cx1 > x1 || cy0 < y0 || cy1 > y1)
			select_range(cx0, cy0, cx1, cy1);
		else
			draw_grid();
		m = _ksys_get_mouse_pos(KSYS_MOUSE_SCREEN_POS);
		context_menu(target, m.x, m.y);
		return;
	}

	// the wheel and the scrollbars, unless a drag of the grid holds the mouse
	if (drag == DRAG_NONE) {
		scrollbar_v_mouse(&scroll_v);
		scrollbar_h_mouse(&scroll_h);
		if (wheel || (int)scroll_v.position != scroll_y || (int)scroll_h.position != scroll_x) {
			stop_edit();
			scroll_y = scroll_v.position + wheel * WHEEL_STEP;
			scroll_x = scroll_h.position;
			draw_grid();
		}
		if (wheel || scroll_v.delta2 || scroll_h.delta2)
			return;
	}

	// a click in the formula bar, or in the box over the cell while editing,
	// edits there; the box with the focus takes its clicks and the drags
	// that began in it
	if (pressed) {
		edit_box *hit = in_box(&bar_box, m.x, m.y) ? &bar_box : (editor && in_box(&cell_box, m.x, m.y) ? &cell_box : NULL);
		if (hit && !editor)
			start_edit(1, hit);
		else if (hit && hit != editor) {
			set_editor(hit);
			draw_grid();
		}
	}
	if (editor && ((editor->flags & ed_mouse_on) || (pressed && in_box(editor, m.x, m.y)))) {
		edit_box_mouse(editor);
		draw_box_lines();
		was_down = down;
		return;
	}

	if (pressed)
		mouse_down(m.x, m.y, _ksys_get_control_key_state() & SHIFT_KEYS, buttons & MOUSE_DOUBLE_CLICK);
	else if (down)
		mouse_move(m.x, m.y);
	else if (was_down)
		mouse_up();
	was_down = down;
}

// a tick of a drag of the selection or the fill: past an edge it scrolls
// on; the button let go out of the window, where no mouse event comes,
// ends it
void drag_tick(void)
{
	ksys_pos_t m = _ksys_get_mouse_pos(KSYS_MOUSE_WINDOW_POS);

	if (_ksys_get_mouse_buttons() & KSYS_MOUSE_LBUTTON_PRESSED)
		auto_scroll(m.x, m.y);
	else
		process_mouse();
}

// the pointers of the drags, once; a picture without its zero is its rows
void mouse_init(void)
{
	cursor_col = make_pointer(pointer_col, POINTER_COL_W, (sizeof(pointer_col) - 1) / POINTER_COL_W);
	cursor_row = make_pointer(pointer_row, POINTER_ROW_W, (sizeof(pointer_row) - 1) / POINTER_ROW_W);
	cursor_fill = make_pointer(pointer_fill, POINTER_FILL_W, (sizeof(pointer_fill) - 1) / POINTER_FILL_W);
}

/* =============================== keys ============================= */

// the codes of the moving keys in the kernel keymap: by the code, not the
// scancode, the keypad types digits with Num Lock on
#define KEY_LEFT     176
#define KEY_DOWN     177
#define KEY_UP       178
#define KEY_RIGHT    179
#define KEY_HOME     180
#define KEY_END      181
#define KEY_DELETE   182
#define KEY_PGDOWN   183
#define KEY_PGUP     184
#define KEY_INSERT   185

// a key, by its code and scancode, that types a character: the kernel
// gives the function, lock and Win keys letter and digit codes too
static int types_char(int code, int s)
{
	if (code < ' ' || (code >= KEY_LEFT && code <= KEY_INSERT))
		return 0;
	return !(s >= KSYS_SCANCODE_CAPSLOCK && s <= KSYS_SCANCODE_SCRLOCK) // F1..F10 between
	    && s != KSYS_SCANCODE_F11 && s != KSYS_SCANCODE_F12
	    && !(s >= KSYS_SCANCODE_EXT_LWIN && s <= KSYS_SCANCODE_EXT_MENU);
}

void process_key(void)
{
	int keys = _ksys_get_control_key_state();
	int shift = keys & SHIFT_KEYS, ctrl = keys & CTRL_KEYS;
	int cx = shift ? end_x : sel_x, cy = shift ? end_y : sel_y; // what moves
	int page = max(1, VIEW_H / ROW_H - 1); // the rows PgUp and PgDn move
	ksys_oskey_t k = _ksys_get_key();

	if (drag != DRAG_NONE) // the mouse is moving the grid: no key under it
		return;
	// the toolbar, in editing too: the text goes into its cell first
	if (ctrl && k.ctrl_key == KSYS_SCANCODE_N) {
		tool_command(TOOL_NEW);
		return;
	}
	if (ctrl && k.ctrl_key == KSYS_SCANCODE_O) {
		tool_command(TOOL_OPEN);
		return;
	}
	if (ctrl && k.ctrl_key == KSYS_SCANCODE_S) {
		tool_command(TOOL_SAVE);
		return;
	}
	if (k.ctrl_key == KSYS_SCANCODE_F1) {
		tool_command(TOOL_HELP);
		return;
	}
	if (k.ctrl_key == KSYS_SCANCODE_ENTER) {
		select_cell(sel_x, sel_y + 1); // an edited text into its cell first
		return;
	}
	if (editor) {
		if (k.ctrl_key == KSYS_SCANCODE_ESC)
			cancel_edit();
		else {
			edit_box_key_safe(editor, k);
			edit_follow();
		}
		return;
	}
	if (ctrl) {
		switch (k.ctrl_key) {
		case KSYS_SCANCODE_A: select_range(1, 1, COLS, ROWS); break;
		case KSYS_SCANCODE_X: run_command(CMD_CUT); break;
		case KSYS_SCANCODE_C: run_command(CMD_COPY); break;
		case KSYS_SCANCODE_V: run_command(CMD_PASTE); break;
		case KSYS_SCANCODE_Z: tool_command(TOOL_UNDO); break;
		case KSYS_SCANCODE_Y: tool_command(TOOL_REDO); break;
		case KSYS_SCANCODE_F:
			show_formulas = !show_formulas;
			draw_grid();
			break;
		}
		return;
	}
	if (k.ctrl_key == KSYS_SCANCODE_F2) {
		start_edit(1, &cell_box);
		return;
	}
	switch (k.code) {
	case KEY_UP:     cy--; break;
	case KEY_DOWN:   cy++; break;
	case KEY_LEFT:   cx--; break;
	case KEY_RIGHT:  cx++; break;
	case KEY_PGUP:   cy -= page; break;
	case KEY_PGDOWN: cy += page; break;
	case KEY_HOME:   cx = 1; break;
	case KEY_END:    cx = COLS; break;
	case KEY_DELETE:
		run_command(CMD_CLEAR);
		return;
	default: // a character starts editing the cell with it
		if (types_char(k.code, k.ctrl_key)) {
			start_edit(0, &cell_box);
			edit_box_key_safe(&cell_box, k);
			edit_follow();
		}
		return;
	}
	// the active cell, or with Shift the other corner, to cx, cy
	if (shift) {
		end_x = clamp(cx, 1, COLS);
		end_y = clamp(cy, 1, ROWS);
		scroll_to(end_x, end_y);
		draw_selection();
	} else
		select_cell(cx, cy);
}
