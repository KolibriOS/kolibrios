#ifndef KOLIBRI_GUI_H
#define KOLIBRI_GUI_H

#include "stdlib.h" /* for malloc() */
#include <kos32sys.h>

#include "kolibri_debug.h" /* work with debug board */

/* boxlib loader */
#include "kolibri_boxlib.h"

/* All supported GUI elements included */
#include "kolibri_gui_elements.h"

enum KOLIBRI_GUI_EVENTS {
  KOLIBRI_EVENT_REDRAW = 1,   /* Window and window elements should be redrawn */
  KOLIBRI_EVENT_KEY = 2,      /* A key on the keyboard was pressed */
  KOLIBRI_EVENT_BUTTON = 3,   /* A button was clicked with the mouse */
  KOLIBRI_EVENT_MOUSE = 6     /* Mouse activity (movement, button press) was detected */
};

void kolibri_handle_event_redraw(struct kolibri_window* some_window)
{
  /*  Draw windows with system color table. */

  BeginDraw();

  DrawWindow(some_window->topleftx, some_window->toplefty, 
	     some_window->sizex, some_window->sizey,
	     some_window->window_title,
	     kolibri_color_table.color_work_area, some_window->XY);
  
  /* Enumerate and draw all window elements here */
  if(some_window->elements) /* Draw all elements added to window */
    {
      struct kolibri_window_element* current_element = some_window -> elements; 
      
      do
	{
	  /* The redraw_fn serves as draw_fn on initial draw */
	  if(kolibri_gui_op_table[current_element -> type].redraw_fn)
	    kolibri_gui_op_table[current_element -> type].redraw_fn(current_element -> element);
	  
	  switch(current_element -> type)
	    {
	    case KOLIBRI_EDIT_BOX:
	    case KOLIBRI_CHECK_BOX:
	      __asm__ volatile("push $0x13371337"::); /* Random value pushed to balance stack */
						      /* otherwise edit_box_draw leaves stack unbalanced */
						      /* and GCC jumps like a crazy motha' fucka' */
	      break;
	    }

	  current_element = current_element -> next;
	  
	} while(current_element != some_window->elements); /* Have we covered all elements? */
    }
}

void kolibri_handle_event_key(struct kolibri_window* some_window)
{
  /* Enumerate and trigger key handling functions of window elements here */
  if(some_window->elements) 
    {
      struct kolibri_window_element *current_element = some_window -> elements; 

      do
	{
	  /* Only execute if the function pointer isn't NULL */
	  if(kolibri_gui_op_table[current_element -> type].key_fn)
	    kolibri_gui_op_table[current_element -> type].key_fn(current_element -> element);
	  
	  current_element = current_element -> next;
	} while(current_element != some_window->elements); /* Have we covered all elements? */
    }
}

void kolibri_handle_event_mouse(struct kolibri_window* some_window)
{
  /* Enumerate and trigger mouse handling functions of window elements here */
  if(some_window->elements) 
    {
      struct kolibri_window_element *current_element = some_window -> elements; 

      do
	{
	  
	  if(kolibri_gui_op_table[current_element -> type].mouse_fn)
	    kolibri_gui_op_table[current_element -> type].mouse_fn(current_element -> element);

	  current_element = current_element -> next;
	} while(current_element != some_window->elements); /* Have we covered all elements? */
    }
}

void kolibri_exit(void)
{
  __asm__ volatile ("int $0x40"::"a"(-1));
}

int kolibri_gui_init(void)
{
  int boxlib_init_status = kolibri_boxlib_init();

  if(boxlib_init_status == 0) 
    debug_board_write_str("ashmew2 is happy: Kolibri GUI Successfully Initialized.\n");
  else 
    {
      debug_board_write_str("ashmew2 is sad: Kolibri GUI Failed to initialize.\n");
      kolibri_exit();
    }

  /* Initialize the global operation table which handles event functions of */
  /* each individual element type */
  kolibri_init_gui_op_table();

  /* Get the current color table for Kolibri and store in global table*/
  kolibri_get_system_colors(&kolibri_color_table);

  /* Set up system events for buttons, mouse and keyboard and redraw */
  /* Also set filters so that window receives mouse events only when active
     and mouse inside window */
  __asm__ volatile("int $0x40"::"a"(40), "b"(0xC0000027)); 
}

/* Note: The current implementation tries to automatically colors 
   GUI elements with system theme */

#endif /* KOLIBRI_GUI_H */
