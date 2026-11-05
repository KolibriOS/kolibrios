// The editing of the active cell, in the box over it and in the formula
// bar at once: one text, two box_lib edit boxes. editor is the box with
// the focus, NULL when not editing; box_lib drops the focus by itself on a
// click outside a box, so it is not the sign of editing.

#include "window.h"
#include "../sheet/refs.h"
#include "../sheet/undo.h"

#include <string.h>

#define EDIT_H    22 // a box_lib edit box for the 8x16 font, with its borders
#define BOX_INSET (CELL_PAD - 2) // box_lib draws its text 2 px inside its left edge
#define BOX_TOP   2 // under the top line: the text where a cell has it

static char edit_text[CELL_MAX + 2]; // box_lib ends the text with two zeros
edit_box cell_box = { .max = CELL_MAX, .text = edit_text };
edit_box bar_box = { .max = CELL_MAX, .text = edit_text };
edit_box *editor;

// the lines the edit boxes draw over: a box EDIT_H high BOX_TOP under a
// top line ends on the bottom line of the formula bar and on the frame of
// the active cell; they go again after a box
void draw_box_lines(void)
{
	_ksys_draw_bar(GRID_X, BAR_Y + BAR_H - 1, grid_right + SCROLL_SIZE - GRID_X, 1, sc.work_graph);
	if (editor)
		draw_frame();
}

// the text of a cell into the edit boxes, the caret at its end
static void load_edit_text(const char *text)
{
	strncpy(edit_text, text ? text : "", CELL_MAX); // zero padded
	cell_box.size = cell_box.pos = bar_box.size = bar_box.pos = strlen(edit_text);
	cell_box.offset = bar_box.offset = 0;
}

// an edit box in the colours of a cell, its borders in its background:
// the frame of the cell and the lines of the bar are drawn over them
static void box_colors(edit_box *box)
{
	box->color = box->focus_border_color = box->blur_border_color = sc.work_light;
	box->shift_color = sc.work_button;
	box->text_color = FONT_CP866 | sc.work_text;
}

// the formula bar as a row of the grid: the name of the active cell, then
// its text as typed; the box over the cell when editing
void edit_draw(void)
{
	int x0 = GRID_X + NAME_W, x1 = grid_right + SCROLL_SIZE - 1; // the line after the name, the right line
	char name[8];

	draw_bevel(GRID_X, BAR_Y, x1 + 1 - GRID_X, BAR_H, sc.work_graph, sc.work_graph);
	_ksys_draw_bar(x0, BAR_Y, 1, BAR_H, sc.work_graph);
	_ksys_draw_bar(GRID_X + 1, BAR_Y + 1, NAME_W - 1, BAR_H - 2, mid_color);
	_ksys_draw_bar(x0 + 1, BAR_Y + 1, x1 - x0 - 1, BAR_H - 2, sc.work_light);
	cell_name(sel_x, sel_y, name);
	draw_centred(GRID_X, BAR_Y, NAME_W, ROW_H, name, GRID_X, sc.work_text);

	box_colors(&bar_box);
	box_colors(&cell_box);
	if (!editor)
		load_edit_text(cells[sel_x][sel_y].text);
	bar_box.left = x0 + BOX_INSET; // the text CELL_PAD from the line, as in a cell
	bar_box.top = BAR_Y + BOX_TOP;
	bar_box.width = x1 - BOX_INSET - bar_box.left;
	edit_box_draw(&bar_box);
	if (editor) {
		cell_box.left = col_x[sel_x] + BOX_INSET;
		cell_box.top = row_y[sel_y] + BOX_TOP;
		// a column wider than the view: the box to the scrollbar
		cell_box.width = min(col_w[sel_x], grid_right - col_x[sel_x]) - 2 * BOX_INSET;
		edit_box_draw(&cell_box);
	}
	draw_box_lines();
}

// the focus to one of the two edit boxes, or none
void set_editor(edit_box *box)
{
	editor = box;
	cell_box.flags = box == &cell_box ? ed_focus : 0;
	bar_box.flags = box == &bar_box ? ed_focus : 0;
}

// editing the active cell in the box over it or in the formula bar, with
// its text or empty
void start_edit(int keep, edit_box *box)
{
	end_x = sel_x; // the selection only the cell: the frame is the cell's
	end_y = sel_y;
	load_edit_text(keep ? cells[sel_x][sel_y].text : NULL);
	set_editor(box);
	draw_grid();
}

// after the box with the focus changed the text: the other one follows
void edit_follow(void)
{
	edit_box *other = editor == &cell_box ? &bar_box : &cell_box;

	other->size = editor->size;
	other->pos = editor->pos;
	edit_box_draw(other);
	draw_box_lines();
}

// the edited text into the cell; nothing when not editing
void stop_edit(void)
{
	const char *old = cells[sel_x][sel_y].text;

	if (!editor)
		return;
	set_editor(NULL);
	if (strcmp(old ? old : "", edit_text) == 0)
		return; // as it was: nothing to undo
	undo_save(sel_x, sel_y, sel_x, sel_y);
	cell_set(sel_x, sel_y, edit_text);
	table_recalc();
}

void cancel_edit(void)
{
	set_editor(NULL);
	draw_grid();
}

// a point in an edit box, its borders included (EDIT_H: clayer/boxlib.h
// has the height field at a wrong offset)
int in_box(edit_box *box, int x, int y)
{
	return in_rect(x, y, box->left, box->top, box->left + box->width + 1, box->top + EDIT_H);
}
