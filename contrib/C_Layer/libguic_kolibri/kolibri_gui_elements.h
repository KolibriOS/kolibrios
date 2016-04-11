#ifndef KOLIBRI_GUI_ELEMENTS_H
#define KOLIBRI_GUI_ELEMENTS_H

/* GUI Elements being used */
#include "kolibri_editbox.h"
#include "kolibri_checkbox.h"
#include "kolibri_button.h"

/* enum KOLIBRI_GUI_ELEMENT_TYPE contains all available GUI items from box_lib */
/* More elements can be added from other libraries as required */
enum KOLIBRI_GUI_ELEMENT_TYPE {
  KOLIBRI_EDIT_BOX,
  KOLIBRI_CHECK_BOX,
  KOLIBRI_RADIO_BUTTON,
  KOLIBRI_SCROLL_BAR,
  KOLIBRI_DYNAMIC_BUTTON,
  KOLIBRI_MENU_BAR,
  KOLIBRI_FILE_BROWSER,
  KOLIBRI_TREE_LIST,
  KOLIBRI_PATH_SHOW,
  KOLIBRI_TEXT_EDITOR,
  KOLIBRI_FRAME,
  KOLIBRI_PROGRESS_BAR,

  KOLIBRI_BUTTON,

  /* Add elements above this element in order to let KOLIBRI_NUM_GUI_ELEMENTS */
  /* stay at correct value */
  
  KOLIBRI_NUM_GUI_ELEMENTS 
};

/* Linked list which connects together all the elements drawn inside a GUI window */
struct kolibri_window_element {
  enum KOLIBRI_GUI_ELEMENT_TYPE type;
  void *element;
  struct kolibri_window_element *next, *prev;
};

/* Generic structure for supporting functions on various elements of Kolibri's GUI */
struct kolibri_element_operations {
  void (*redraw_fn)(void *);
  void (*mouse_fn)(void *);
  void (*key_fn)(void *);
};

/* Structure for a GUI Window on Kolibri. It also contains all the elements drawn in window */
struct kolibri_window {
  unsigned int topleftx, toplefty;
  unsigned int sizex, sizey;
  char *window_title;

  /* Refer to sysfuncs, value to be stored in EDX (Function 0) */
  unsigned int XY;

  struct kolibri_window_element *elements;
};

/*---------------------End of Structure and enum definitions---------------*/
/*---------------------Define various functions for initializing GUI-------*/

/* Master table containing operations for various GUI elements in one place */
struct kolibri_element_operations kolibri_gui_op_table[KOLIBRI_NUM_GUI_ELEMENTS];

void kolibri_init_gui_op_table(void)
{
/* Setting up functions for edit box GUI elements*/
kolibri_gui_op_table[KOLIBRI_EDIT_BOX].redraw_fn = edit_box_draw;
kolibri_gui_op_table[KOLIBRI_EDIT_BOX].mouse_fn = edit_box_mouse;
kolibri_gui_op_table[KOLIBRI_EDIT_BOX].key_fn = editbox_key;

/* Setting up functions for check box GUI elements*/
kolibri_gui_op_table[KOLIBRI_CHECK_BOX].redraw_fn = check_box_draw2;
kolibri_gui_op_table[KOLIBRI_CHECK_BOX].mouse_fn = check_box_mouse2;
kolibri_gui_op_table[KOLIBRI_CHECK_BOX].key_fn = NULL;

/* Setting up functions for Kolibri Buttons ( SysFunc 8 )*/
kolibri_gui_op_table[KOLIBRI_BUTTON].redraw_fn = draw_button;
kolibri_gui_op_table[KOLIBRI_BUTTON].mouse_fn = NULL;
kolibri_gui_op_table[KOLIBRI_BUTTON].key_fn = NULL;
}

/* Create a new main GUI window for KolibriOS */
/* tl stands for TOP LEFT. x and y are coordinates. */

struct kolibri_window * kolibri_new_window(int tlx, int tly, int sizex, int sizey, char *title)
{
  struct kolibri_window *new_win = (struct kolibri_window *)malloc(sizeof(struct kolibri_window));

  new_win->topleftx = tlx;
  new_win->toplefty = tly;
  new_win->sizex = sizex;
  new_win->sizey = sizey;
  new_win->window_title = title;
  new_win->XY = 0x00000013; /* All windows are skinned windows with caption for now */
  new_win->elements = NULL;
  
  return new_win;
}

/* Add an element to an existing window */
void kolibri_window_add_element(struct kolibri_window *some_window, enum KOLIBRI_GUI_ELEMENT_TYPE element_type, void *some_gui_element)
{
  struct kolibri_window_element *new_element = (struct kolibri_window_element *)malloc(sizeof(struct kolibri_window_element));
  
  new_element -> type = element_type;
  new_element -> element = some_gui_element;

  if(!(some_window->elements)) /* No elements in window yet */
    {
      some_window->elements = new_element;
      some_window->elements -> prev = some_window->elements;
      some_window->elements -> next = some_window->elements;
    }
  else
    {
      struct kolibri_window_element *last_element = some_window -> elements -> prev;
  
      last_element -> next = new_element;
      new_element -> next = some_window -> elements; /* start of linked list  */
      some_window -> elements -> prev = new_element;
      new_element -> prev = last_element;
    }
}

#endif /* KOLIBRI_GUI_ELEMENTS_H */
