#include "kolibri_gui.h"
#include "kolibri_kmenu.h"
#include "kolibri_libimg.h"

int main()
{
  /* Load all libraries, initialize global tables like system color table and
     operations table. kolibri_gui_init() will EXIT with mcall -1 if it fails
     to do it's job. This is all you need to call and all libraries and GUI
     elements can be used after a successful call to this function
  */
  kolibri_gui_init();
  kolibri_kmenu_init();
  kolibri_libimg_init();
  /* Set gui_event to REDRAW so that window is drawn in first iteration  */
  unsigned int gui_event = KOLIBRI_EVENT_REDRAW;
  oskey_t key;

  struct kolibri_window *main_window = kolibri_new_window(50, 50, 420, 350, "Example 2");
  struct check_box *checkbox = kolibri_new_check_box(20, 30, 12, 12, "Print to BOARD selected menu item.");
  
  kolibri_window_add_element(main_window, KOLIBRI_CHECK_BOX, checkbox);
    
  extern volatile unsigned press_key;
  
  ufile_t load_f;
  load_f = load_file("/kolibrios/111.png");
  void* image_data_rgb = (void*)malloc(load_f.size);
  void* image_data = img_decode(load_f.data, load_f.size, 0);
  img_to_rgb2(image_data, image_data_rgb);
  //img_destroy(image_data);	
  
  void* main_menu;
  void* main_menu_file;
  void* main_menu_edit;
  kmenu_init(NULL);
  main_menu = ksubmenu_new();
  main_menu_file = ksubmenu_new();
  main_menu_edit = ksubmenu_new();
  ksubmenu_add(main_menu_file, kmenuitem_new(0, "Open", 110));
  ksubmenu_add(main_menu_file, kmenuitem_new(0, "Save", 111));
  ksubmenu_add(main_menu_file, kmenuitem_new(2, NULL, 112));
  ksubmenu_add(main_menu_file, kmenuitem_new(0, "Exit", 113));
  ksubmenu_add(main_menu, kmenuitem__submenu_new(1, "File", main_menu_file));
  ksubmenu_add(main_menu_edit, kmenuitem_new(0, "Vertical flip", 120));
  ksubmenu_add(main_menu_edit, kmenuitem_new(0, "Horizontal flip", 121));
  ksubmenu_add(main_menu, kmenuitem__submenu_new(1, "Edit", main_menu_edit));
  
  do  /* Start of main activity loop */
    {
      if(gui_event == KOLIBRI_EVENT_REDRAW)
	{
	  kolibri_handle_event_redraw(main_window);
	  kmainmenu_draw(main_menu);
	  draw_bitmap(image_data_rgb, 5, 60, 400, 250);
	}
      else if(gui_event == KOLIBRI_EVENT_KEY)
	{
	  key = get_key();
	  kolibri_handle_event_key(main_window);
	}
      else if(gui_event == KOLIBRI_EVENT_BUTTON)
	{
	  unsigned int pressed_button = kolibri_button_get_identifier();
	  switch (pressed_button)
	  {
	      case 0x00000001:
			kolibri_exit();
			break;
		  case 110:
		    if(checkbox -> flags & CHECKBOX_IS_SET) debug_board_write_str("Open\n");
		    break;
		  case 111:
		    if(checkbox -> flags & CHECKBOX_IS_SET) debug_board_write_str("Save\n");
		    break;
		  case 113:
		    if(checkbox -> flags & CHECKBOX_IS_SET) debug_board_write_str("Exit\n");
		    kolibri_exit();
		    break;
		  case 120:
		    if(checkbox -> flags & CHECKBOX_IS_SET) debug_board_write_str("Vertical flip\n");
		    img_flip(image_data, 0x01);
		    img_to_rgb2(image_data, image_data_rgb);
		    draw_bitmap(image_data_rgb, 5, 30, 400, 250);
		    break;
		  case 121:
		    if(checkbox -> flags & CHECKBOX_IS_SET) debug_board_write_str("Horizontal flip\n");
		    img_flip(image_data, 0x02);
		    img_to_rgb2(image_data, image_data_rgb);
		    draw_bitmap(image_data_rgb, 5, 30, 400, 250);
		    break;
	  }
	 }
      else if(gui_event == KOLIBRI_EVENT_MOUSE)
	{
	  kolibri_handle_event_mouse(main_window);
	  kmainmenu_dispatch_cursorevent(main_menu);
	}

    } while(gui_event = get_os_event()); /* End of main activity loop */

  /* kolibri_quit(); */

  return 0;
}
