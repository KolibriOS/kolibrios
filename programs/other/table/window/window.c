// The main window: the toolbar over the formula bar and the grid.

#include "window.h"
#include "../sheet/undo.h"

#define WINDOW_SHARE 3    // quarters of the work area the window takes at first, at most
#define WINDOW_MIN_W 400
#define WINDOW_MIN_H 240
#define WINDOW_STYLE 0x73 // skinned, resizable, caption, client coordinates, not filled
#define WINDOW_ROLLED_UP 0x04 // function 9: a bit of window_state

// the client area round the cells: the buttons, the formula bar, the
// headers, the scrollbars and the margins
#define CHROME_W     (CELLS_X + SCROLL_SIZE + EDGE)
#define CHROME_H     (CELLS_Y + SCROLL_SIZE + EDGE)

#define FIRST_BUTTON 2    // + TOOL_NEW ... TOOL_HELP: the ids of the buttons, after the close one

static int window_x, window_y, window_w, window_h; // where the window opens
static int window_min_w; // the toolbar in it

// the toolbar: menus of one item side by side, the last one, the help, at
// the right edge
static MenuItem tools[] = { // by TOOL_*
	{ TOOL_NEW,  T_NEW,  "", ICON_NEW,  0 },
	{ TOOL_OPEN, T_OPEN, "", ICON_OPEN, 0 },
	{ TOOL_SAVE, T_SAVE, "", ICON_SAVE, 0 },
	{ TOOL_UNDO, T_UNDO, "", ICON_UNDO, 0 },
	{ TOOL_REDO, T_REDO, "", ICON_REDO, 0 },
	{ TOOL_HELP, T_HELP, "", ICON_HELP, 0 },
};

#define TOOLS ((int)(sizeof(tools) / sizeof(tools[0])))

static Menu toolbar[TOOLS];

// the tools from the left edge: their right end
static int place_tools(void)
{
	int i, x = EDGE;

	for (i = 0; i < TOOLS; i++) {
		menu_place(&toolbar[i], &tools[i], 1, x, GAP);
		x += toolbar[i].w + GAP;
	}
	return x - GAP;
}

// the tools as place_tools() put them, the help with its right on the
// right of the grid, which moves with the window; Undo and Redo greyed
// with nothing to undo or redo, a click on them dropped
static void draw_toolbar(void)
{
	int i;

	toolbar[TOOL_HELP].x = grid_right + SCROLL_SIZE - toolbar[TOOL_HELP].w;
	tools[TOOL_UNDO].disabled = !undo_steps();
	tools[TOOL_REDO].disabled = !redo_steps();
	for (i = 0; i < TOOLS; i++) {
		// function 8 adds a button each time, also outside the drawing of
		// the window: the one defined before goes first
		_ksys_delete_button(FIRST_BUTTON + i);
		if (tools[i].disabled)
			toolbar[i].hover = MENU_NONE;
		menu_draw(&toolbar[i], FIRST_BUTTON + i);
	}
}

void toolbar_mouse(int x, int y)
{
	int i;

	for (i = 0; i < TOOLS; i++)
		menu_mouse(&toolbar[i], x, y);
}

void toolbar_update(void)
{
	if (tools[TOOL_UNDO].disabled != !undo_steps() || tools[TOOL_REDO].disabled != !redo_steps())
		draw_toolbar();
}

// a greyed tool takes no click
void process_button(void)
{
	int id = _ksys_get_button(), tool = id - FIRST_BUTTON;

	if (id == CLOSE_BUTTON)
		tool_command(TOOL_CLOSE);
	else if (!tools[tool].disabled)
		tool_command(tool);
}

// the window as it opens: in the middle of the work area, WINDOW_SHARE of
// it at most but wide enough for the toolbar, its cells whole columns and
// rows, so that the last ones end on the lines of the scrollbars
void place_window(void)
{
	Area a;
	int cols, rows, skin_h = _ksys_get_skin_height();

	work_area(&a);
	window_min_w = max(place_tools() + EDGE + 2 * SKIN_BORDER, WINDOW_MIN_W);

	cols = (a.w * WINDOW_SHARE / 4 - 2 * SKIN_BORDER - CHROME_W) / COL_W;
	rows = (a.h * WINDOW_SHARE / 4 - skin_h - SKIN_BORDER - CHROME_H) / ROW_H;
	window_w = CHROME_W + cols * COL_W + 2 * SKIN_BORDER;
	while (window_w < window_min_w)
		window_w += COL_W;
	window_h = max(CHROME_H + rows * ROW_H + skin_h + SKIN_BORDER, WINDOW_MIN_H);
	window_x = a.left + (a.w - window_w) / 2;
	window_y = a.top + (a.h - window_h) / 2;
}

void draw_window(void)
{
	ksys_thread_t info;

	_ksys_start_draw();
	_ksys_create_window(window_x, window_y, window_w, window_h, NULL, sc.work, WINDOW_STYLE);
	set_title(); // a window defined without one has none, rolled up too
	_ksys_end_draw();

	_ksys_thread_info(&info, KSYS_THIS_SLOT);
	if (info.window_state & WINDOW_ROLLED_UP)
		return;
	if (info.winx_size < window_min_w || info.winy_size < WINDOW_MIN_H) {
		_ksys_change_window(-1, -1, max(info.winx_size, window_min_w), max(info.winy_size, WINDOW_MIN_H));
		return;
	}
	grid_right = info.clientwidth - EDGE - SCROLL_SIZE;
	grid_bottom = info.clientheight - EDGE - SCROLL_SIZE;

	// the window is not filled, so that nothing flashes: the margins here,
	// the rest is the buttons, the formula bar and the grid
	_ksys_draw_bar(0, 0, info.clientwidth, BAR_Y, sc.work);
	_ksys_draw_bar(0, BAR_Y, EDGE, info.clientheight - BAR_Y, sc.work);
	_ksys_draw_bar(info.clientwidth - EDGE, BAR_Y, EDGE, info.clientheight - BAR_Y, sc.work);
	_ksys_draw_bar(EDGE, BAR_Y + BAR_H, info.clientwidth - 2 * EDGE, GAP, sc.work);
	_ksys_draw_bar(EDGE, info.clientheight - EDGE, info.clientwidth - 2 * EDGE, EDGE, sc.work);

	draw_toolbar();
	draw_grid();
	if (drag == DRAG_COL || drag == DRAG_ROW) // its line, drawn over
		draw_drag_line();
}
