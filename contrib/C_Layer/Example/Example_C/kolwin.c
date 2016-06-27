#include "kolibri_gui.h"

int main()
{
  /* Load all libraries, initialize global tables like system color table and 
     operations table. kolibri_gui_init() will EXIT with mcall -1 if it fails
     to do it's job. This is all you need to call and all libraries and GUI
     elements can be used after a successful call to this function
  */
  kolibri_gui_init();

  /* Set gui_event to REDRAW so that window is drawn in first iteration  */
  unsigned int gui_event = KOLIBRI_EVENT_REDRAW;

  struct kolibri_window *main_window = kolibri_new_window(50, 50, 400, 100, "BoardMsg: Send msg to debug board");
  struct check_box *checkbox = kolibri_new_check_box(20, 40, 12, 12, "Append BOARDMSG to entered message.");
  struct edit_box *textbox = kolibri_new_edit_box(20, 55, 40);
  struct kolibri_button *button = kolibri_new_button(310, 55, 14, 14, 0x00123456, kolibri_color_table.color_work_button);

  kolibri_window_add_element(main_window, KOLIBRI_EDIT_BOX, textbox);
  kolibri_window_add_element(main_window, KOLIBRI_CHECK_BOX, checkbox);
  kolibri_window_add_element(main_window, KOLIBRI_BUTTON, button);

  do  /* Start of main activity loop */
    {
      if(gui_event == KOLIBRI_EVENT_REDRAW)
	{
	  kolibri_handle_event_redraw(main_window);
	}
      else if(gui_event == KOLIBRI_EVENT_KEY)
	{
	  kolibri_handle_event_key(main_window);
	}
      else if(gui_event == KOLIBRI_EVENT_BUTTON)
	{
	  unsigned int pressed_button = kolibri_button_get_identifier();

	  if(pressed_button = 0x00123456) /* Our button was pressed */
	    {
	      if(checkbox -> flags & CHECKBOX_IS_SET) /* Append BoardMsg checkbox is set */
		debug_board_write_str("BOARDMSG: ");

	      debug_board_write_str(textbox->text);
	      debug_board_write_str("\n");
	    }
	}
      else if(gui_event == KOLIBRI_EVENT_MOUSE)
	{
	  kolibri_handle_event_mouse(main_window);
	}

    } while(gui_event = get_os_event()); /* End of main activity loop */

  /* kolibri_quit(); */

  return 0;
}
