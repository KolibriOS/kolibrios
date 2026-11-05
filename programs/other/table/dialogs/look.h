#ifndef DIALOGS_LOOK_H
#define DIALOGS_LOOK_H

// The look and the system things every window of the program shares:
// the skin colours, the 8x16 font, the margins, the icons of ICONS18, the
// texts in the language of the build, the windows of its threads.

#include "../sheet/cells.h" // min(), max()

#include <sys/ksys.h>

#define VERSION      "1.0"
#define CLOSE_BUTTON 1    // function 17: the close button of a skinned window
#define THREAD_WAIT  5    // hundredths of a second between looks at a thread the program waits for
#define THREAD_STACK 8192 // a window of a thread of the program: the menu, the help

// function 37.3: the buttons of the mouse pressed just now
#define MOUSE_CLICK        (7 << 8) // any
#define MOUSE_RIGHT_CLICK  (1 << 9)
#define MOUSE_DOUBLE_CLICK (1 << 24) // the left one twice

#define SKIN_BORDER  5  // the frame of a skinned window on the left, right and bottom
#define EDGE         8  // to the left, right and bottom edges of the window
#define GAP          12 // to the top edge, between the buttons and the rows

#define FONT_W       8
#define FONT_H       16
#define FONT_CAPS_Y  2  // the capitals of the 8x16 font are its rows 2..11
#define FONT_CAPS_H  10
#define CAPS_Y(h)    (((h) - FONT_CAPS_H) / 2 - FONT_CAPS_Y) // the text y: its capitals in the middle of h

// function 4 fonts, 8x16: cp866 of a given length for the cells, UTF-8
// zero terminated for the texts of the window
#define FONT_CP866   0x10000000
#define FONT_UTF8_Z  0xB0000000

// the system colours with the names function 48.3 gives them;
// ksys_colors_table_t still has the old ones for work_dark and work_light
typedef struct {
	uint32_t frame, grab, work_dark, work_light, grab_text;
	uint32_t work, work_button, work_button_text, work_text, work_graph;
} Skin;

extern Skin sc;
void read_skin(void);

// x, y in the box from left, top to right, bottom: right and bottom out of it
int in_rect(int x, int y, int left, int top, int right, int bottom);
void draw_text(const char *s, int x, int y, uint32_t color); // UTF-8, ended by a zero
// a line round a box, one colour on the top and left, one on the bottom
// and right
void draw_bevel(int x, int y, int w, int h, uint32_t top_left, uint32_t bottom_right);

// a group as group_box draws it: a line of work_graph, one of work_light
// inside it, the capitals of the title s on the middle of the top lines
#define GROUP_LINES       2
#define GROUP_TITLE_DY    (FONT_CAPS_Y + (FONT_CAPS_H - GROUP_LINES) / 2) // the title over the top line
#define GROUP_TITLE_ABOVE (GROUP_TITLE_DY - FONT_CAPS_Y)                  // its capitals over the top line
#define GROUP_TITLE_BELOW (FONT_CAPS_Y + FONT_CAPS_H - GROUP_TITLE_DY)    // and from it down
void draw_group(int x, int y, int w, int h, const char *s);

/* ============================== icons ============================= */

#define ICON_SIZE 18 // an icon of ICONS18
void draw_icon(int icon, int x, int y, uint32_t back);

// the images of the menu rows in ICONS18
#define ICON_NEW       2
#define ICON_OPEN      70
#define ICON_UNDO      37
#define ICON_REDO      36
#define ICON_HELP      69
#define ICON_SAVE      5
#define ICON_DONT_SAVE 63 // a bin
#define ICON_CANCEL    67 // a cross
#define ICON_CUT       55
#define ICON_COPY      20
#define ICON_PASTE     56
#define ICON_CLEAR     63 // a bin
#define ICON_LEFT      30 // arrows: insert on that side
#define ICON_RIGHT     31
#define ICON_UP        32
#define ICON_DOWN      33
#define ICON_DELETE    67 // a cross

/* ============================== texts ============================= */

// in the language of the build: LANG_RUS, LANG_SPA or else English, from
// LANG of the Tupfile; UTF-8
enum {
	T_TITLE, T_NEW, T_OPEN, T_SAVE, T_NO_FILE, T_PART, T_NOT_SAVED,
	T_DONT_SAVE, T_CANCEL, T_SAVED,
	T_MENU, T_CUT, T_COPY, T_PASTE, T_CLEAR, T_UNDO, T_REDO, T_HELP,
	T_INSERT_LEFT, T_INSERT_RIGHT, T_INSERT_ABOVE, T_INSERT_BELOW, T_DELETE,
	T_ARITHMETIC, T_LOGIC, T_CELLS, T_FUNCTIONS, T_MATH, T_TRIGONOMETRY, // the help
	T_MOVES, T_FIXED, T_COLUMN_ROW, T_RANGE, T_IS_NUMBER, T_COUNT
};

extern const char *texts[T_COUNT];
void texts_init(void);

// the characters of a UTF-8 text: a lead byte each
int utf8_chars(const char *s);

/* ============================= windows ============================ */

// the work area: the screen without the taskbar
typedef struct {
	int left, top, w, h;
} Area;

void work_area(Area *a);
// the window of a thread of the program, as the help: fixed, its client
// area w x h, in the middle of the work area, titled as child_title()
void child_window(int w, int h, int title);
// the window of a thread of the program, as a menu: w x h at screen x, y,
// no frame, no title bar, all of it drawn by the thread; titled as child_title()
void popup_window(int x, int y, int w, int h, int title);
// the title of the window being defined: s in an encoding KSYS_TITLE_*
// (function 71.2); the kernel keeps s, not a copy of it
void window_title(const char *s, int encoding);
// of a thread: texts[text]
void child_title(int text);

#endif
