// TABLE - a spreadsheet: formulas, CSV files in UTF-8 or cp866.
//
// sheet/    the cells: their texts, computing, names, the changes of
//           ranges, undo
// formula/  the formulas
// files/    the table in CSV files, UTF-8 and cp866
// dialogs/  the look of the windows, the menus, the help
// window/   the main window: the grid, editing, the input, the commands
//
// sheet/, formula/ and files/ know no window and are tested on the host.

#include "window/window.h"

#define SCROLL_TICK 5 // hundredths of a second between steps of a drag past the edge

int main(int argc, char **argv)
{
	srand(_ksys_get_tick_count()); // RAND() of the formulas new each run
	texts_init();
	table_clear();
	mouse_init();
	place_window();
	files_init();

	if (argc > 1)
		load_file(argv[1]);

	_ksys_set_event_mask(KSYS_EVM_REDRAW | KSYS_EVM_KEY | KSYS_EVM_BUTTON | KSYS_EVM_MOUSE | KSYS_EVM_MOUSE_FILTER);
	for (;;) {
		// a drag of the selection or the fill: a tick too, to scroll on past
		// an edge while the mouse stands
		int scrolls = drag == DRAG_SELECT || drag == DRAG_FILL;
		switch (scrolls ? _ksys_wait_event_timeout(SCROLL_TICK) : _ksys_wait_event()) {
		case KSYS_EVENT_NONE:
			drag_tick();
			break;
		case KSYS_EVENT_REDRAW:
			read_colors();
			draw_window();
			break;
		case KSYS_EVENT_KEY:
			process_key();
			break;
		case KSYS_EVENT_BUTTON:
			process_button();
			break;
		case KSYS_EVENT_MOUSE:
			process_mouse();
			break;
		}
		toolbar_update();
	}
	return 0;
}
