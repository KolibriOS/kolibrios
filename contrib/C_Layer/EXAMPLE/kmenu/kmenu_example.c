#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <kos32sys.h>
#include <kolibri_gui.h>
#include <kolibri_kmenu.h>

int main()
{
  /* Load all libraries, initialize global tables like system color table and
     operations table. kolibri_gui_init() will EXIT with mcall -1 if it fails
     to do it's job. This is all you need to call and all libraries and GUI
     elements can be used after a successful call to this function
  */
  kolibri_gui_init();
  kolibri_kmenu_init();
  /* Set gui_event to REDRAW so that window is drawn in first iteration  */
  unsigned int gui_event = KOLIBRI_EVENT_REDRAW;
  oskey_t key;

  kolibri_window *main_window = kolibri_new_window(50, 50, 400, 200, "kmenu example");

  kmenu_init(NULL);

  ksubmenu_t *sub_menu1 = ksubmenu_new();
  ksubmenu_add(sub_menu1, kmenuitem_new(KMENUITEM_NORMAL, "Open", 101));
  ksubmenu_add(sub_menu1, kmenuitem_new(KMENUITEM_NORMAL, "Save", 102));
  ksubmenu_add(sub_menu1, kmenuitem_new(KMENUITEM_SEPARATOR, NULL, 0));
  ksubmenu_add(sub_menu1, kmenuitem_new(KMENUITEM_NORMAL, "Exit", 103));

  ksubmenu_t *sub_menu2 = ksubmenu_new();
  ksubmenu_add(sub_menu2, kmenuitem_new(KMENUITEM_NORMAL, "Find", 201));
  ksubmenu_add(sub_menu2, kmenuitem_new(KMENUITEM_NORMAL, "Replace", 202));

  ksubmenu_t *sub_menu22 = ksubmenu_new();
  ksubmenu_add(sub_menu22, kmenuitem_new(KMENUITEM_NORMAL, "CP1251", 211));
  ksubmenu_add(sub_menu22, kmenuitem_new(KMENUITEM_NORMAL, "UTF-8", 212));
  ksubmenu_add(sub_menu2, kmenuitem__submenu_new(KMENUITEM_SUBMENU, "Encoding", sub_menu22));

  ksubmenu_t *main_menu = ksubmenu_new();
  ksubmenu_add(main_menu, kmenuitem__submenu_new(KMENUITEM_SUBMENU, "File", sub_menu1));
  ksubmenu_add(main_menu, kmenuitem__submenu_new(KMENUITEM_SUBMENU, "Edit", sub_menu2));

  extern volatile unsigned press_key;

  do  /* Start of main activity loop */
    {
      if(gui_event == KOLIBRI_EVENT_REDRAW)

	{
	  kolibri_handle_event_redraw(main_window);
	  kmainmenu_draw(main_menu);
	}
      else if(gui_event == KOLIBRI_EVENT_KEY)
	{
	  key = get_key();
	  switch (key.code)
	  {
	  }
	  press_key = key.val;

	  kolibri_handle_event_key(main_window);
	}
      else if(gui_event == KOLIBRI_EVENT_BUTTON)
	{
	  unsigned int pressed_button = kolibri_button_get_identifier();
	  switch (pressed_button)
	  {
	      case 101:
	        debug_board_write_str("File->Open\n");
			break;
	      case 102:
	        debug_board_write_str("File->Save\n");
			break;
	      case 103:
	        debug_board_write_str("File->Exit\n");
			break;
	      case 201:
	        debug_board_write_str("Edit->Find\n");
			break;
	      case 202:
	        debug_board_write_str("Edit->Replace\n");
			break;
	      case 211:
	        debug_board_write_str("Edit->Encoding->cp1251\n");
			break;
	      case 212:
	        debug_board_write_str("Edit->Encoding->UTF-8\n");
			break;
	      case BUTTON_CLOSE:
			kolibri_exit();
	  }
	 }
      else if(gui_event == KOLIBRI_EVENT_MOUSE)
	{
	  kolibri_handle_event_mouse(main_window);
	  kmainmenu_dispatch_cursorevent(main_menu);
	}

    } while((gui_event = get_os_event())); /* End of main activity loop */

  /* kolibri_quit(); */

  return 0;
}
