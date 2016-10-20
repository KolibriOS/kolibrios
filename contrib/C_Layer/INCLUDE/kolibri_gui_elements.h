#ifndef KOLIBRI_GUI_ELEMENTS_H
#define KOLIBRI_GUI_ELEMENTS_H

#include "kolibri_colors.h"

/* enum KOLIBRI_GUI_ELEMENT_TYPE contains all available GUI items from box_lib */
/* More elements can be added from other libraries as required */
enum KOLIBRI_GUI_ELEMENT_TYPE {
  KOLIBRI_EDIT_BOX,
  KOLIBRI_CHECK_BOX,
  KOLIBRI_OPTIONGROUP,
  KOLIBRI_SCROLL_BAR_H,
  KOLIBRI_SCROLL_BAR_V,
  KOLIBRI_DYNAMIC_BUTTON,
  KOLIBRI_MENU_BAR,
  KOLIBRI_FILE_BROWSER,
  KOLIBRI_TREE_LIST,
  KOLIBRI_PATH_SHOW,
  KOLIBRI_TEXT_EDITOR,
  KOLIBRI_FRAME,
  KOLIBRI_PROGRESS_BAR,
  KOLIBRI_STATICTEXT,
  KOLIBRI_STATICNUM,
  KOLIBRI_BUTTON,
  KOLIBRI_D_BUTTON,
  KOLIBRI_PATHSHOW,
  KOLIBRI_FILEBROWSE,
  KOLIBRI_EDITOR,
  KOLIBRI_TREELIST,

  /* Add elements above this element in order to let KOLIBRI_NUM_GUI_ELEMENTS */
  /* stay at correct value */

  KOLIBRI_NUM_GUI_ELEMENTS
};

#define X_Y(x,y) (((x)<<16)|(y))

/* Linked list which connects together all the elements drawn inside a GUI window */
typedef struct{
  enum KOLIBRI_GUI_ELEMENT_TYPE type;
  void *element;
  void *next, *prev;
}kolibri_window_element;

typedef void (*cb_elem_boxlib)(void *) __attribute__((__stdcall__));
typedef void (*cbkey_elem_boxlib)(void *, oskey_t) __attribute__((__stdcall__));

/* Generic structure for supporting functions on various elements of Kolibri's GUI */
typedef struct {
 	cb_elem_boxlib 	redraw_fn;
 	cb_elem_boxlib 	mouse_fn;
 	cbkey_elem_boxlib 	key_fn;
}kolibri_element_operations;

/* Structure for a GUI Window on Kolibri. It also contains all the elements drawn in window */
typedef struct{
  unsigned int topleftx, toplefty;
  unsigned int sizex, sizey;
  char *window_title;

  /* Refer to sysfuncs, value to be stored in EDX (Function 0) */
  unsigned int XY;

  kolibri_window_element *elements;
}kolibri_window;

/*---------------------End of Structure and enum definitions---------------*/

void kolibri_window_add_element(kolibri_window *some_window, enum KOLIBRI_GUI_ELEMENT_TYPE element_type, void *some_gui_element); // forward declaration

/* GUI Elements being used */
#include "kolibri_button.h"
#include "kolibri_checkbox.h"
#include "kolibri_d_button.h"
#include "kolibri_editbox.h"
#include "kolibri_frame.h"
#include "kolibri_menubar.h"
#include "kolibri_optionbox.h"
#include "kolibri_pathshow.h"
#include "kolibri_progressbar.h"
#include "kolibri_scrollbar.h"
#include "kolibri_statictext.h"
#include "kolibri_filebrowse.h"
#include "kolibri_editor.h"
#include "kolibri_treelist.h"


/*---------------------Define various functions for initializing GUI-------*/

/* Master table containing operations for various GUI elements in one place */
kolibri_element_operations kolibri_gui_op_table[KOLIBRI_NUM_GUI_ELEMENTS];

void kolibri_init_gui_op_table(void)
{
/* Setting up functions for edit box GUI elements*/
kolibri_gui_op_table[KOLIBRI_EDIT_BOX].redraw_fn = (cb_elem_boxlib)edit_box_draw;
kolibri_gui_op_table[KOLIBRI_EDIT_BOX].mouse_fn = (cb_elem_boxlib)edit_box_mouse;
kolibri_gui_op_table[KOLIBRI_EDIT_BOX].key_fn = (cbkey_elem_boxlib)editbox_key;

/* Setting up functions for check box GUI elements*/
kolibri_gui_op_table[KOLIBRI_CHECK_BOX].redraw_fn = (cb_elem_boxlib)check_box_draw2;
kolibri_gui_op_table[KOLIBRI_CHECK_BOX].mouse_fn = (cb_elem_boxlib)check_box_mouse2;
kolibri_gui_op_table[KOLIBRI_CHECK_BOX].key_fn = NULL;

/* Setting up functions for Kolibri Buttons ( SysFunc 8 )*/
kolibri_gui_op_table[KOLIBRI_BUTTON].redraw_fn = (cb_elem_boxlib)draw_button;
kolibri_gui_op_table[KOLIBRI_BUTTON].mouse_fn = NULL;
kolibri_gui_op_table[KOLIBRI_BUTTON].key_fn = NULL;

/* Setting up functions for progress bar GUI elements*/
kolibri_gui_op_table[KOLIBRI_PROGRESS_BAR].redraw_fn = (cb_elem_boxlib)progressbar_draw;
kolibri_gui_op_table[KOLIBRI_PROGRESS_BAR].mouse_fn = NULL;
kolibri_gui_op_table[KOLIBRI_PROGRESS_BAR].key_fn = NULL;

/* Setting up functions for frame GUI elements*/
kolibri_gui_op_table[KOLIBRI_FRAME].redraw_fn = (cb_elem_boxlib)frame_draw;
kolibri_gui_op_table[KOLIBRI_FRAME].mouse_fn = NULL;
kolibri_gui_op_table[KOLIBRI_FRAME].key_fn = NULL;

/* scrollbars */
kolibri_gui_op_table[KOLIBRI_SCROLL_BAR_H].redraw_fn = (cb_elem_boxlib)scrollbar_h_draw;
kolibri_gui_op_table[KOLIBRI_SCROLL_BAR_H].mouse_fn = (cb_elem_boxlib)scrollbar_h_mouse;
kolibri_gui_op_table[KOLIBRI_SCROLL_BAR_H].key_fn = NULL;

kolibri_gui_op_table[KOLIBRI_SCROLL_BAR_V].redraw_fn = (cb_elem_boxlib)scrollbar_v_draw;
kolibri_gui_op_table[KOLIBRI_SCROLL_BAR_V].mouse_fn = (cb_elem_boxlib)scrollbar_v_mouse;
kolibri_gui_op_table[KOLIBRI_SCROLL_BAR_V].key_fn = NULL;

kolibri_gui_op_table[KOLIBRI_STATICTEXT].redraw_fn = (cb_elem_boxlib)statictext_draw;
kolibri_gui_op_table[KOLIBRI_STATICTEXT].mouse_fn = NULL;
kolibri_gui_op_table[KOLIBRI_STATICTEXT].key_fn = NULL;

kolibri_gui_op_table[KOLIBRI_STATICNUM].redraw_fn = (cb_elem_boxlib)staticnum_draw;
kolibri_gui_op_table[KOLIBRI_STATICNUM].mouse_fn = NULL;
kolibri_gui_op_table[KOLIBRI_STATICNUM].key_fn = NULL;

kolibri_gui_op_table[KOLIBRI_OPTIONGROUP].redraw_fn = (cb_elem_boxlib)option_box_draw;
kolibri_gui_op_table[KOLIBRI_OPTIONGROUP].mouse_fn = (cb_elem_boxlib)option_box_mouse;
kolibri_gui_op_table[KOLIBRI_OPTIONGROUP].key_fn = NULL;

kolibri_gui_op_table[KOLIBRI_MENU_BAR].redraw_fn = (cb_elem_boxlib)menu_bar_draw;
kolibri_gui_op_table[KOLIBRI_MENU_BAR].mouse_fn = (cb_elem_boxlib)menu_bar_mouse;
kolibri_gui_op_table[KOLIBRI_MENU_BAR].key_fn = NULL;

kolibri_gui_op_table[KOLIBRI_D_BUTTON].redraw_fn = (cb_elem_boxlib)dynamic_button_draw;
kolibri_gui_op_table[KOLIBRI_D_BUTTON].mouse_fn = (cb_elem_boxlib)dynamic_button_mouse;
kolibri_gui_op_table[KOLIBRI_D_BUTTON].key_fn = NULL;

kolibri_gui_op_table[KOLIBRI_PATHSHOW].redraw_fn = (cb_elem_boxlib)path_show_draw;
kolibri_gui_op_table[KOLIBRI_PATHSHOW].mouse_fn = NULL;
kolibri_gui_op_table[KOLIBRI_PATHSHOW].key_fn = NULL;

kolibri_gui_op_table[KOLIBRI_FILEBROWSE].redraw_fn = (cb_elem_boxlib)filebrowse_draw;
kolibri_gui_op_table[KOLIBRI_FILEBROWSE].mouse_fn = (cb_elem_boxlib)filebrowse_mouse;
kolibri_gui_op_table[KOLIBRI_FILEBROWSE].key_fn = (cbkey_elem_boxlib)filebrowser_key;

kolibri_gui_op_table[KOLIBRI_EDITOR].redraw_fn = (cb_elem_boxlib)ted_draw;
kolibri_gui_op_table[KOLIBRI_EDITOR].mouse_fn = (cb_elem_boxlib)ted_mouse;
kolibri_gui_op_table[KOLIBRI_EDITOR].key_fn = (cbkey_elem_boxlib)editor_key;

kolibri_gui_op_table[KOLIBRI_TREELIST].redraw_fn = (cb_elem_boxlib)treelist_draw;
kolibri_gui_op_table[KOLIBRI_TREELIST].mouse_fn = (cb_elem_boxlib)tl_mouse;
kolibri_gui_op_table[KOLIBRI_TREELIST].key_fn = (cbkey_elem_boxlib)treelist_key;
debug_board_printf("KOLIBRI_TREELIST (%x,%x,%x)\n", treelist_draw, tl_mouse, treelist_key);
}

/* Create a new main GUI window for KolibriOS */
/* tl stands for TOP LEFT. x and y are coordinates. */

kolibri_window * kolibri_new_window(int tlx, int tly, int sizex, int sizey, char *title)
{
  kolibri_window *new_win = (kolibri_window *)malloc(sizeof(kolibri_window));

  new_win->topleftx = tlx;
  new_win->toplefty = tly;
  new_win->sizex = sizex;
  new_win->sizey = sizey;
  new_win->window_title = title;
  new_win->XY = 0x33;
  new_win->elements = NULL;

  return new_win;
}

/* Add an element to an existing window */
void kolibri_window_add_element(kolibri_window *some_window, enum KOLIBRI_GUI_ELEMENT_TYPE element_type, void *some_gui_element)
{
  kolibri_window_element *new_element = (kolibri_window_element *)malloc(sizeof(kolibri_window_element));

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
      kolibri_window_element *last_element = some_window -> elements -> prev;

      last_element -> next = new_element;
      new_element -> next = some_window -> elements; /* start of linked list  */
      some_window -> elements -> prev = new_element;
      new_element -> prev = last_element;
    }
}

#endif /* KOLIBRI_GUI_ELEMENTS_H */
