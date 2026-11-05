// The popup menu of menu.h in the look of the menus of Eolite: a double
// 3D frame round it, the rows flat as tall as in @MENU, the one under the
// mouse or chosen by the arrow keys in work_light, a groove between groups.

#include "menu.h"

#define SEP_H          2          // a separator: a groove of two lines
#define MIN_W          192
#define QUESTION_TOP   4          // a question: its frame under the title of its window
// the sides of a row: 8 px from the outside with the frame and the empty
// column of an image or a letter
#define SIDE           (8 - MENU_FRAME - 1)
#define IMAGE_Y        ((MENU_ITEM_H - ICON_SIZE) / 2)
#define TEXT_X         (SIDE + ICON_SIZE + 6) // a space after the image
#define TEXT_Y         CAPS_Y(MENU_ITEM_H)
#define HOTKEY_GAP     24         // from the longest text to the hotkeys
// function 8: a button not drawn, its edge inverted when pressed; it
// takes one pixel more than its size right and below, so the edge is on
// the outer line of the frame
#define NO_DRAW_BUTTON 0x40000000

static Menu menu; // the popup or the question
static int menu_title; // in texts[]: a thread shows on the taskbar, its window has a name
static int menu_x, menu_y; // on the screen
static int asking; // a question in a window of the skin, not a popup
static volatile int result; // the id of the item chosen, or MENU_NONE
static char stack[THREAD_STACK];

// the width of a menu with its frame: the longest text, then the
// longest hotkey
static int menu_width(const Menu *m)
{
	int i, text = 0, hotkey = 0;

	for (i = 0; i < m->count; i++) {
		if (m->items[i].text != MENU_SEPARATOR)
			text = max(text, utf8_chars(texts[m->items[i].text]));
		hotkey = max(hotkey, utf8_chars(m->items[i].hotkey));
	}
	return MENU_FRAME + TEXT_X + text * FONT_W + (hotkey ? HOTKEY_GAP + hotkey * FONT_W : 0) + SIDE + MENU_FRAME;
}

// the double 3D frame round a box: outside work_dark on the top and left,
// work_graph on the bottom and right; inside work_light and work_dark
static void draw_frame(int x, int y, int w, int h)
{
	draw_bevel(x, y, w, h, sc.work_dark, sc.work_graph);
	draw_bevel(x + 1, y + 1, w - 2, h - 2, sc.work_light, sc.work_dark);
}

// the top of item i in the window, of the end for count
static int item_y(const Menu *m, int i)
{
	int k, y = m->y + MENU_FRAME;

	for (k = 0; k < i; k++)
		y += m->items[k].text != MENU_SEPARATOR ? MENU_ITEM_H : SEP_H;
	return y;
}

// item i can be chosen: not a separator, not greyed
static int enabled(const Menu *m, int i)
{
	return m->items[i].text != MENU_SEPARATOR && !m->items[i].disabled;
}

// the next enabled item after hover, dir 1 down or -1 up, round the end;
// with no hover the first or the last one
static int next_item(int dir)
{
	int k, i = menu.hover;

	if (i == MENU_NONE)
		i = dir == 1 ? menu.count - 1 : 0;
	for (k = 0; k < menu.count; k++) {
		i = (i + dir + menu.count) % menu.count;
		if (enabled(&menu, i))
			return i;
	}
	return MENU_NONE;
}

// item i inside the frame: a row, or the groove of a separator
static void draw_item(const Menu *m, int i)
{
	const MenuItem *it = &m->items[i];
	int x = m->x + MENU_FRAME, y = item_y(m, i), w = m->w - 2 * MENU_FRAME;
	uint32_t back = i == m->hover ? sc.work_light : sc.work;
	// the hotkey a step paler than the text: work_text and work_graph, a
	// disabled item work_graph and work_dark
	uint32_t text = sc.work_text, hotkey = sc.work_graph;

	if (it->text == MENU_SEPARATOR) {
		_ksys_draw_bar(x, y, w, 1, sc.work_dark);
		_ksys_draw_bar(x, y + 1, w, 1, sc.work_light);
		return;
	}
	_ksys_draw_bar(x, y, w, MENU_ITEM_H, back);
	draw_icon(it->image, x + SIDE, y + IMAGE_Y, back);
	if (it->disabled) {
		text = sc.work_graph;
		hotkey = sc.work_dark;
	}
	draw_text(texts[it->text], x + TEXT_X, y + TEXT_Y, text);
	// none, "", draws nothing
	draw_text(it->hotkey, x + w - SIDE - utf8_chars(it->hotkey) * FONT_W, y + TEXT_Y, hotkey);
}

void menu_place(Menu *m, const MenuItem *items, int count, int x, int y)
{
	m->items = items;
	m->count = count;
	m->x = x;
	m->y = y;
	m->w = menu_width(m);
	m->h = item_y(m, count) - y + MENU_FRAME;
	m->hover = MENU_NONE;
}

void menu_draw(const Menu *m, int id)
{
	int i;

	draw_frame(m->x, m->y, m->w, m->h);
	for (i = 0; i < m->count; i++)
		draw_item(m, i);
	_ksys_define_button(m->x, m->y, m->w - 2, m->h - 2, id | NO_DRAW_BUTTON, 0);
}

static void set_hover(Menu *m, int i)
{
	int old = m->hover;

	if (i == old)
		return;
	m->hover = i;
	if (old != MENU_NONE)
		draw_item(m, old);
	if (i != MENU_NONE)
		draw_item(m, i);
}

void menu_mouse(Menu *m, int x, int y)
{
	int i;

	for (i = 0; i < m->count; i++)
		if (in_rect(x, y, m->x + MENU_FRAME, item_y(m, i), m->x + m->w - MENU_FRAME, item_y(m, i + 1)))
			break;
	// past the last row, a separator or a greyed item: none
	set_hover(m, i < m->count && enabled(m, i) ? i : MENU_NONE);
}

// the window: a popup at the point, inside the screen, or a question;
// the button over the menu chooses the row under the mouse, and the
// kernel takes no click for the title of a popup, which would drag it or
// roll it up
static void draw_menu(void)
{
	ksys_pos_t screen = _ksys_screen_size();
	int w = menu.w, h = menu.y + menu.h;

	_ksys_start_draw();
	if (asking)
		child_window(w, h, menu_title);
	else
		popup_window(min(menu_x, screen.x + 1 - w), min(menu_y, screen.y + 1 - h), w, h, menu_title);
	menu_draw(&menu, 0);
	_ksys_end_draw();
}

static void close_menu(int id)
{
	result = id;
	_ksys_exit();
}

// the item under the mouse or chosen by the arrow keys, if there is one
static void choose_hover(void)
{
	if (menu.hover != MENU_NONE)
		close_menu(menu.items[menu.hover].id);
}

static void menu_thread(void)
{
	ksys_pos_t m;

	// the mouse out of the window too: a click there closes a popup
	_ksys_set_event_mask(KSYS_EVM_REDRAW | KSYS_EVM_KEY | KSYS_EVM_BUTTON | KSYS_EVM_MOUSE);
	for (;;) {
		switch (_ksys_wait_event()) {
		case KSYS_EVENT_REDRAW:
			draw_menu();
			break;
		case KSYS_EVENT_BUTTON: // the close button of a question too
			if (_ksys_get_button() == CLOSE_BUTTON)
				close_menu(MENU_NONE);
			choose_hover();
			break;
		case KSYS_EVENT_KEY:
			switch (_ksys_get_key().ctrl_key) {
			case KSYS_SCANCODE_ESC:
				close_menu(MENU_NONE);
				break;
			case KSYS_SCANCODE_ENTER:
				choose_hover();
				break;
			case KSYS_SCANCODE_EXT_DOWN:
				set_hover(&menu, next_item(1));
				break;
			case KSYS_SCANCODE_EXT_UP:
				set_hover(&menu, next_item(-1));
				break;
			}
			break;
		case KSYS_EVENT_MOUSE:
			m = _ksys_get_mouse_pos(KSYS_MOUSE_WINDOW_POS);
			if (!asking && (_ksys_get_mouse_eventstate() & MOUSE_CLICK) && !in_rect(m.x, m.y, 0, 0, menu.w, menu.h))
				close_menu(MENU_NONE);
			menu_mouse(&menu, m.x, m.y);
			break;
		}
	}
}

// the menu of n items, a popup or a question, its size from them; waits
// until its thread has ended, so that the next menu may take its stack.
// Meanwhile redraw() draws the window of the program, whose keys and
// buttons are dropped; a click on it brings the menu back to the front, a
// popup closes on it all the same (function 18.3 takes no slot 0: none
// when the thread has ended)
static int menu_open(const MenuItem *it, int n, int title, int question, void (*redraw)(void))
{
	int tid;

	menu_title = title;
	asking = question;
	menu_place(&menu, it, n, 0, question ? QUESTION_TOP : 0); // a question: its frame under the title
	menu.w = max(menu.w, MIN_W);
	result = MENU_NONE;
	tid = _ksys_create_thread(menu_thread, stack + THREAD_STACK);
	if (tid == -1)
		return MENU_NONE;
	while (_ksys_get_thread_slot(tid)) // function 18.21: 0 when it has ended
		switch (_ksys_wait_event_timeout(THREAD_WAIT)) {
		case KSYS_EVENT_REDRAW:
			redraw();
			break;
		case KSYS_EVENT_KEY:
			_ksys_get_key();
			break;
		case KSYS_EVENT_BUTTON: // its close button too: the kernel gives no click for it
			_ksys_get_button();
			_ksys_focus_window(_ksys_get_thread_slot(tid));
			break;
		case KSYS_EVENT_MOUSE:
			if (_ksys_get_mouse_eventstate() & MOUSE_CLICK)
				_ksys_focus_window(_ksys_get_thread_slot(tid));
			break;
		}
	return result;
}

int menu_popup(const MenuItem *it, int n, int x, int y, int title, void (*redraw)(void))
{
	menu_x = x;
	menu_y = y;
	return menu_open(it, n, title, 0, redraw);
}

int menu_ask(const MenuItem *it, int n, int title, void (*redraw)(void))
{
	return menu_open(it, n, title, 1, redraw);
}
