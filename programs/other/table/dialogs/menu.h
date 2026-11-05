#ifndef DIALOGS_MENU_H
#define DIALOGS_MENU_H

#include "look.h"

// A popup menu: rows of an image of ICONS18 (of @RESHARE), a
// text and its hotkey on the right, separators between groups. A thread of
// its own with a window at a point of the screen, closed by a choice, a
// click out of it or Esc; or a question in a window of the skin in the
// middle, as the help, closed by a choice, its close button or Esc.

#define MENU_HOTKEY_MAX 6 // "Ctrl+X"

typedef struct {
	int id;             // what menu_popup gives for it
	int text;           // in texts[], or MENU_SEPARATOR
	char hotkey[MENU_HOTKEY_MAX + 1]; // UTF-8, "" for none
	int image;          // in ICONS18; a separator has none
	int disabled;       // shown greyed, not chosen
} MenuItem;

#define MENU_FRAME     2  // the frame round a menu: two lines
#define MENU_ITEM_H    26 // a row, as @MENU

#define MENU_SEPARATOR -1
#define MENU_NONE     -1 // no item chosen

// a menu drawn at x, y of a window, w x h with its frame; the item under
// the mouse in work_light
typedef struct {
	const MenuItem *items;
	int count, x, y, w, h, hover;
} Menu;

// its size from its items, no item under the mouse
void menu_place(Menu *m, const MenuItem *items, int count, int x, int y);
// drawn, and a button over it that is not drawn and has no frame when
// pressed: a click on it gives id
void menu_draw(const Menu *m, int id);
// the mouse at x, y of the window: the enabled item under it in work_light
void menu_mouse(Menu *m, int x, int y);

// the menu of count items with its top left at screen x, y, moved to stay
// on the screen, its window titled as child_title() makes it from
// texts[title]; waits for it: the id of the item chosen, or MENU_NONE.
// Meanwhile redraw() draws the window of the program, its keys and buttons
// are dropped
int menu_popup(const MenuItem *items, int count, int x, int y, int title, void (*redraw)(void));
// the menu as a question in a window of the skin
int menu_ask(const MenuItem *items, int count, int title, void (*redraw)(void));

#endif
