// The help: a window of its own thread with the formulas by examples and
// their values, in groups as group_box of DOCPACK draws them, in two
// columns to fit a screen of 640x480; the titles and a few texts in the
// language of the build. One at a time: the button again brings it to the
// front. The close button or Esc closes it.

#include "dialogs.h"

#define LINE_H       FONT_H     // a line of the text
#define KEY_GAP      GAP        // from the longest key to the texts
#define COLUMNS      2

// a line: a formula and its value, the same in every language, or what
// it does in the language of the build; a heading, the title of a group
// of the lines after it, has no formula; the line with neither ends a
// column, the last one too. The groups side by side have as many lines:
// the blocks of the columns line up.
#define KEY_MAX   14 // "=DEGREES(PI())"
#define VALUE_MAX 8  // "3.142..."

typedef struct {
	char key[KEY_MAX + 1], value[VALUE_MAX + 1]; // value "": texts[text]
	int text;
} HelpLine;

#define VALUE(key, value) { key, value, 0 }
#define TEXT(key, text)   { key, "", text }
#define HEADING(text)     { "", "", text }
#define COLUMN_BREAK      T_COUNT // a line with no text: the end of a column
#define END_COLUMN        { "", "", COLUMN_BREAK }

static const HelpLine lines[] = {
	HEADING(T_ARITHMETIC),
	VALUE("=2 + 3 * 4",     "14"),
	VALUE("=2 ^ 10",        "1024"),
	VALUE("=50%",           "0.5"),
	VALUE("=MOD(7, 3)",     "1"),
	HEADING(T_LOGIC),
	VALUE("=3 > 2",         "1"),
	VALUE("=3 <= 2",        "0"),
	VALUE("=3 <> 3",        "0"),
	VALUE("=NOT(0)",        "1"),
	VALUE("=IF(0, 1, 2)",   "2"),
	VALUE("=AND(1, 0)",     "0"),
	VALUE("=OR(1, 0)",      "1"),
	VALUE("=XOR(1, 1)",     "0"),
	HEADING(T_CELLS),
	TEXT("=A1 + B1",        T_MOVES),
	TEXT("=$A$1",           T_FIXED),
	TEXT("=$A1, =A$1",      T_COLUMN_ROW),
	TEXT("=SUM(A1:B3)",     T_RANGE),
	TEXT("=ISNUMBER(A1)",   T_IS_NUMBER),
	END_COLUMN,
	HEADING(T_FUNCTIONS),
	VALUE("=SUM(1, 2, 3)",  "6"),
	VALUE("=AVERAGE(2, 4)", "3"),
	VALUE("=MIN(4, 1, 3)",  "1"),
	VALUE("=MAX(4, 1, 3)",  "4"),
	HEADING(T_MATH),
	VALUE("=SQRT(16)",      "4"),
	VALUE("=ABS(-5)",       "5"),
	VALUE("=LN(EXP(2))",    "2"),
	VALUE("=LOG(1000)",     "3"),
	VALUE("=ROUND(2.5, 0)", "3"),
	VALUE("=FLOOR(3, 2)",   "2"),
	VALUE("=CEILING(3, 2)", "4"),
	VALUE("=RAND()",        "0 .. 1"),
	HEADING(T_TRIGONOMETRY),
	VALUE("=SIN(PI() / 2)", "1"),
	VALUE("=COS(PI())",     "-1"),
	VALUE("=TAN(0)",        "0"),
	VALUE("=RADIANS(180)",  "3.142..."),
	VALUE("=DEGREES(PI())", "180"),
	END_COLUMN,
};

#define LINES ((int)(sizeof(lines) / sizeof(lines[0])))

static const char *line_text(const HelpLine *l)
{
	return l->value[0] ? l->value : texts[l->text];
}

#define INSET        (GROUP_LINES + GAP) // the lines inside a group, EDGE under them

static int help_h;
// the columns, left[COLUMNS] the right edge; their keys and texts as wide as
// the longest
static int left[COLUMNS + 1] = { EDGE }, key_w[COLUMNS], text_w[COLUMNS];
static int help_tid;                          // the thread of the window, the last one opened
static char stack[THREAD_STACK];

// the lines down the columns, a heading closing the group before it and
// opening its own; drawn, or only measured: the widths of the columns and
// the height of the inside, help_h for the drawing
static int help_lines(int draw)
{
	const char *group = NULL; // the title of the group open
	int i, col = 0, top = 0, y = 0, bottom = 0, height = 0;

	for (i = 0; i < LINES; i++) {
		const HelpLine *l = &lines[i];
		int x = left[col], column_ends = l->text == COLUMN_BREAK;

		if (l->key[0]) {
			if (draw) {
				draw_text(l->key, x + INSET, y, sc.work_text);
				draw_text(line_text(l), x + INSET + key_w[col] + KEY_GAP, y, sc.work_text);
			} else {
				key_w[col] = max(key_w[col], utf8_chars(l->key) * FONT_W);
				text_w[col] = max(text_w[col], utf8_chars(line_text(l)) * FONT_W);
			}
			y += LINE_H;
			continue;
		}

		// a heading or the end of a column: the group open ends; the
		// last one of a column down to the bottom of the highest, measured
		// before
		if (group) {
			bottom = y + EDGE + GROUP_LINES;
			if (draw && column_ends)
				bottom = help_h - EDGE;
			if (draw)
				draw_group(x, top, left[col + 1] - EDGE - x, bottom - top, group);
			height = max(height, bottom + EDGE);
			group = NULL;
		}
		if (column_ends) {
			if (!draw) // the next column after it
				left[col + 1] = x + INSET + key_w[col] + KEY_GAP + text_w[col] + INSET + EDGE;
			col++;
			bottom = 0;
		} else {
			group = texts[l->text];
			top = bottom + GAP + GROUP_TITLE_ABOVE;
			y = top + GROUP_TITLE_BELOW + GAP;
		}
	}
	return height;
}

static void draw_help(void)
{
	_ksys_start_draw();
	child_window(left[COLUMNS], help_h, T_HELP);
	help_lines(1);
	_ksys_end_draw();
}

static void help_thread(void)
{
	_ksys_set_event_mask(KSYS_EVM_REDRAW | KSYS_EVM_KEY | KSYS_EVM_BUTTON);
	for (;;)
		switch (_ksys_wait_event()) {
		case KSYS_EVENT_REDRAW:
			draw_help();
			break;
		case KSYS_EVENT_KEY:
			if (_ksys_get_key().ctrl_key == KSYS_SCANCODE_ESC)
				_ksys_exit();
			break;
		case KSYS_EVENT_BUTTON:
			if (_ksys_get_button() == CLOSE_BUTTON)
				_ksys_exit();
		}
}

// the slot of the thread of the window, 0 when it has ended or there is
// none (function 18.21): a new one may then take its stack
static int help_slot(void)
{
	return help_tid > 0 ? _ksys_get_thread_slot(help_tid) : 0;
}

void help_close(void)
{
	if (help_slot())
		_ksys_kill_by_pid(help_tid);
}

void help_open(void)
{
	int slot = help_slot();

	if (slot) {
		_ksys_focus_window(slot); // to the front
		return;
	}
	help_h = help_lines(0);
	help_tid = _ksys_create_thread(help_thread, stack + THREAD_STACK);
}
