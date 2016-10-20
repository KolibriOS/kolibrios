#ifndef KOLIBRI_GUI_H
#define KOLIBRI_GUI_H

#include "kolibri_debug.h" /* work with debug board */

/* boxlib loader */
#include "kolibri_boxlib.h"

/* All supported GUI elements included */
#include "kolibri_gui_elements.h"

enum KOLIBRI_GUI_EVENTS {
    KOLIBRI_EVENT_NONE = 0,     /* Event queue is empty */
    KOLIBRI_EVENT_REDRAW = 1,   /* Window and window elements should be redrawn */
    KOLIBRI_EVENT_KEY = 2,      /* A key on the keyboard was pressed */
    KOLIBRI_EVENT_BUTTON = 3,   /* A button was clicked with the mouse */
    KOLIBRI_EVENT_DESKTOP = 5,  /* Desktop redraw finished */
    KOLIBRI_EVENT_MOUSE = 6,    /* Mouse activity (movement, button press) was detected */
    KOLIBRI_EVENT_IPC = 7,      /* Interprocess communication notify */
    KOLIBRI_EVENT_NETWORK = 8,  /* Network event */
    KOLIBRI_EVENT_DEBUG = 9,    /* Debug subsystem event */
    KOLIBRI_EVENT_IRQBEGIN = 16 /* 16..31 IRQ0..IRQ15 interrupt =IRQBEGIN+IRQn */
};

#define BUTTON_CLOSE 0x1
#define BTN_QUIT 1

void kolibri_handle_event_redraw(kolibri_window* some_window)
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
      kolibri_window_element* current_element = some_window -> elements;

      do
	{
	  /* The redraw_fn serves as draw_fn on initial draw */
	  if((int)kolibri_gui_op_table[current_element -> type].redraw_fn > 0)  // -1 if DLL link fail
	    kolibri_gui_op_table[current_element -> type].redraw_fn(current_element -> element);

//sie after fixing calling conventions no more needed
/*
	  switch(current_element -> type)
	    {
	    case KOLIBRI_EDIT_BOX:
	    case KOLIBRI_CHECK_BOX:
	      __asm__ volatile("push $0x13371337"::); / * Random value pushed to balance stack * /
						      / * otherwise edit_box_draw leaves stack unbalanced * /
						      / * and GCC jumps like a crazy motha' fucka' * /

	      break;
	    }
*/
	  current_element = current_element -> next;

	} while(current_element != some_window->elements); /* Have we covered all elements? */
    }
}

void kolibri_handle_event_key(kolibri_window* some_window, oskey_t key)
{
  /* Enumerate and trigger key handling functions of window elements here */
  if(some_window->elements)
    {
      kolibri_window_element *current_element = some_window -> elements;

      do
	{
	  /* Only execute if the function pointer isn't NULL, or -1 (fail to find in export table) */
	  if((int)kolibri_gui_op_table[current_element -> type].key_fn > 0)
	    kolibri_gui_op_table[current_element -> type].key_fn(current_element -> element, key);

	  current_element = current_element -> next;
	} while(current_element != some_window->elements); /* Have we covered all elements? */
    }
}

void kolibri_handle_event_mouse(kolibri_window* some_window)
{
  /* Enumerate and trigger mouse handling functions of window elements here */
  if(some_window->elements)
    {
      kolibri_window_element *current_element = some_window -> elements;

      do
	{
	  if((int)kolibri_gui_op_table[current_element -> type].mouse_fn > 0)
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

  return boxlib_init_status;
}

/* Note: The current implementation tries to automatically colors
   GUI elements with system theme */

#endif /* KOLIBRI_GUI_H */
