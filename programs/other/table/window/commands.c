// What the toolbar, the keys and the context menu do; the file of the
// table: New, Open and Save with the dialog of proc_lib, its name in the
// title, the question before changes are lost, @notify when a file fails.

#include "window.h"
#include "../files/encoding.h"
#include "../files/file.h"
#include "../sheet/changes.h"
#include "../sheet/undo.h"

#include <clayer/dialog.h>
#include <stdio.h>
#include <string.h>

/* ============================== files ============================= */

#define NOTIFY_ERROR   'E' // the icons of @notify
#define NOTIFY_WARNING 'W'
#define NOTIFY_OK      'O'

// the files the dialog shows, its Filter menu all: their extensions,
// each ended by a zero, and a zero; proc_lib copies size + 1 bytes
#define EXTENSIONS "CSV\0"
static struct {
	unsigned int size;
	char extensions[sizeof(EXTENSIONS)];
} __attribute__((__packed__)) dialog_filter = { sizeof(dialog_filter) - 1, EXTENSIONS };

// the open and save dialog of proc_lib, its buffers as
// kolibri_new_open_dialog() makes them
static char dialog_procinfo[1024], dialog_dir[4096], dialog_file[4096], dialog_name[256];
static open_dialog dialog = {
	.mode = OPEN,
	.procinfo = dialog_procinfo,
	.draw_window = draw_window, // the main window while the dialog is open
	.com_area_name = sz_com_area_name,
	.opendir_path = dialog_dir,
	.dir_default_path = sz_dir_default_path,
	.start_path = sz_start_path,
	.openfile_path = dialog_file,
	.filename_area = dialog_name,
	.filter_area = (od_filter *)&dialog_filter,
	.x_size = 420,
	.y_size = 320,
};

static char path[4096]; // the file, empty for a new table
static char program[32]; // the name of the program and its version in cp866, as the title and @notify show it

// @notify: the name of the program, under it the text after name, when
// it is not ""; in cp866
static void notify(const char *name, int text, char icon)
{
	char cp866[80], msg[400];

	utf8_to_cp866(texts[text], cp866, sizeof(cp866));
	snprintf(msg, sizeof(msg), "'%s\n%s%s%s' -t%c", program, name, *name ? " " : "", cp866, icon);
	_ksys_exec("/sys/@notify", msg);
}

void files_init(void)
{
	utf8_to_cp866(texts[T_TITLE], program, sizeof(program));
	OpenDialog_init(&dialog);
}

// of the table, without its folders; "" for a new one
static const char *file_name(void)
{
	const char *name = strrchr(path, '/');

	return name ? name + 1 : path;
}

void set_title(void)
{
	static char title[300]; // the kernel keeps it, not a copy (function 71)
	const char *file = file_name();

	snprintf(title, sizeof(title), "%s%s%s", program, *file ? " - " : "", file);
	window_title(title, KSYS_TITLE_CP866);
}

// the table from a file, the columns wider than new ones as wide as their
// texts; one that cannot be read leaves the current table. One more than
// the table holds is not its file: saved over, the rest would be lost
void load_file(const char *name)
{
	int x, loaded = table_load(name);

	if (loaded == LOAD_NONE) {
		notify("", T_NO_FILE, NOTIFY_ERROR);
		return;
	}
	undo_forget();
	path[0] = '\0';
	if (loaded == LOAD_WHOLE)
		strncpy(path, name, sizeof(path) - 1);
	else
		notify("", T_PART, NOTIFY_WARNING);
	for (x = 1; x <= COLS; x++)
		col_w[x] = max(fit_width(x), COL_W);
	sel_x = sel_y = end_x = end_y = 1;
	scroll_x = scroll_y = 0;
}

static void new_table(void)
{
	table_clear();
	undo_forget();
	new_file();
	path[0] = '\0';
	set_title();
	select_cell(1, 1);
}

static void open_file(void)
{
	draw_grid();
	dialog.mode = OPEN;
	OpenDialog_start(&dialog);
	if (dialog.status != SUCCESS)
		return;
	load_file(dialog.openfile_path);
	set_title();
	draw_grid();
}

// to the file of the table, or one the dialog asks for when it has none;
// 1 if saved
static int save_file(void)
{
	if (!*path) {
		dialog.mode = SAVE;
		strcpy(dialog.filename_area, "table.csv");
		OpenDialog_start(&dialog);
		if (dialog.status != SUCCESS)
			return 0;
		strncpy(path, dialog.openfile_path, sizeof(path) - 1);
		set_title();
	}
	if (!table_save(path)) {
		notify("", T_NOT_SAVED, NOTIFY_ERROR);
		return 0;
	}
	return 1;
}

// before the table is gone: its changes saved if the user wants; 0 if
// they cancel, and the table stays
static int changes_kept(void)
{
	enum { ASK_SAVE, ASK_DONT_SAVE, ASK_CANCEL };
	static const MenuItem question[] = {
		{ ASK_SAVE,      T_SAVE,         "", ICON_SAVE,      0 },
		{ ASK_DONT_SAVE, T_DONT_SAVE,    "", ICON_DONT_SAVE, 0 },
		{ ASK_CANCEL,    T_CANCEL,       "", ICON_CANCEL,    0 },
	};

	if (!table_changed)
		return 1;
	draw_grid(); // the edit stopped: its text in its cell, the values it changes, under the question
	switch (menu_ask(question, sizeof(question) / sizeof(question[0]), T_TITLE, draw_window)) {
	case ASK_SAVE:
		if (!save_file())
			return 0;
		notify(file_name(), T_SAVED, NOTIFY_OK); // the window may close at once, @notify stays
		return 1;
	case ASK_DONT_SAVE:
		return 1;
	}
	return 0;
}

/* ============================ commands ============================ */

// undo or redo: the cells changed back selected
static void undo_redo(int redo)
{
	int x0, y0, x1, y1;

	// no step: an edit the button stopped shown in its cell; a step of the
	// sizes only has no cells
	if (table_undo(redo, &x0, &y0, &x1, &y1) && x1 >= x0)
		select_range(x0, y0, x1, y1);
	else
		draw_grid();
}

void tool_command(int tool)
{
	if (tool == TOOL_HELP) {
		help_open();
		return;
	}
	stop_edit(); // an edited text into its cell, drawn before a dialog
	switch (tool) {
	case TOOL_NEW:
		if (changes_kept())
			new_table();
		break;
	case TOOL_OPEN:
		if (changes_kept())
			open_file();
		break;
	case TOOL_SAVE:
		draw_grid();
		save_file();
		break;
	case TOOL_UNDO:
	case TOOL_REDO:
		undo_redo(tool == TOOL_REDO);
		break;
	case TOOL_CLOSE:
		if (changes_kept()) {
			help_close();
			_ksys_exit();
		}
	}
}

// the active cell x0, y0 and the other corner x1, y1
void select_range(int x0, int y0, int x1, int y1)
{
	stop_edit();
	sel_x = x0;
	sel_y = y0;
	end_x = x1;
	end_y = y1;
	draw_selection();
}

// the active cell to x, y, the selection only it, kept in view
void select_cell(int x, int y)
{
	x = clamp(x, 1, COLS);
	y = clamp(y, 1, ROWS);
	scroll_to(x, y);
	select_range(x, y, x, y);
}

void run_command(int cmd)
{
	int x0, y0, x1, y1;

	sel_rect(&x0, &y0, &x1, &y1);
	switch (cmd) {
	case CMD_CUT:
	case CMD_COPY:
		clip_copy(x0, y0, x1, y1, cmd == CMD_CUT);
		break;
	case CMD_PASTE:
		clip_paste(x0, y0);
		break;
	case CMD_CLEAR:
		table_erase(x0, y0, x1, y1);
		break;
	}
	draw_grid();
}

// the menu of the right button at a point of the screen, one row a
// command: the selection, and on the headers its columns or rows
// inserted or deleted
void context_menu(int target, int x, int y)
{
	// the rows of the menu; what cannot be done greyed; the cells have the
	// ones over the separator
	enum { ROW_CUT, ROW_COPY, ROW_PASTE, ROW_CLEAR, ROW_SEPARATOR, ROW_BEFORE, ROW_AFTER, ROW_DELETE, ROW_COUNT };
	static MenuItem items[] = {
		{ CMD_CUT,           T_CUT,          "Ctrl+X", ICON_CUT,    0 },
		{ CMD_COPY,          T_COPY,         "Ctrl+C", ICON_COPY,   0 },
		{ CMD_PASTE,         T_PASTE,        "Ctrl+V", ICON_PASTE,  0 },
		{ CMD_CLEAR,         T_CLEAR,        "Del",    ICON_CLEAR,  0 },
		{ 0,                 MENU_SEPARATOR, "",       0,           0 },
		{ CMD_INSERT_BEFORE, 0,              "",       0,           0 }, // by columns or rows, below
		{ CMD_INSERT_AFTER,  0,              "",       0,           0 },
		{ CMD_DELETE,        T_DELETE,       "",       ICON_DELETE, 0 },
	};
	int cols = target == ON_COLS, x0, y0, x1, y1, first, last, n, cmd;

	sel_rect(&x0, &y0, &x1, &y1);
	first = cols ? x0 : y0;
	last = cols ? x1 : y1;
	n = last - first + 1;
	items[ROW_PASTE].disabled = !clip_cells();
	items[ROW_BEFORE].text = cols ? T_INSERT_LEFT : T_INSERT_ABOVE;
	items[ROW_BEFORE].image = cols ? ICON_LEFT : ICON_UP;
	items[ROW_AFTER].text = cols ? T_INSERT_RIGHT : T_INSERT_BELOW;
	items[ROW_AFTER].image = cols ? ICON_RIGHT : ICON_DOWN;
	// an insertion pushes the last columns or rows out: only empty ones
	items[ROW_BEFORE].disabled = items[ROW_AFTER].disabled = !table_room(cols, n);
	if (last + n > (cols ? COLS : ROWS)) // as many after the last as there are
		items[ROW_AFTER].disabled = 1;
	cmd = menu_popup(items, target == ON_CELLS ? ROW_SEPARATOR : ROW_COUNT, x, y, T_MENU, draw_window);
	if (cmd == CMD_INSERT_BEFORE)
		table_insert(cols, first, n);
	else if (cmd == CMD_INSERT_AFTER)
		table_insert(cols, last + 1, n);
	else if (cmd == CMD_DELETE)
		table_insert(cols, first, -n);
	else if (cmd != MENU_NONE)
		run_command(cmd);
	draw_grid();
}
