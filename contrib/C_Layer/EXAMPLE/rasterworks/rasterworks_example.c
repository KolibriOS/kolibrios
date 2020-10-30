#include <stdlib.h>
#include <string.h>
#include <kos32sys.h>
#include <kolibri_gui.h>
#include <kolibri_rasterworks.h>

int main()
{
  /* Load all libraries, initialize global tables like system color table and
     operations table. kolibri_gui_init() will EXIT with mcall -1 if it fails
     to do it's job. This is all you need to call and all libraries and GUI
     elements can be used after a successful call to this function
  */
  kolibri_gui_init();
  kolibri_rasterworks_init();
  /* Set gui_event to REDRAW so that window is drawn in first iteration  */
  unsigned int gui_event = KOLIBRI_EVENT_REDRAW;
  oskey_t key;

  kolibri_window *main_window = kolibri_new_window(50, 50, 800, 300, "rasterworks example");
  
  unsigned press_key;
  
  int ln_str = countUTF8Z("Пример работы", -1);
  void *buffi = malloc(768*256*3 * sizeof(char));
  
   *((int*)buffi) = 768;
   *((int*)buffi+1) = 256;
  
  memset((char*)buffi+8, (char)-1, 768*256*3);
    
  debug_board_printf("String len: %d \n", ln_str);
  
  drawText(buffi, 0, 0, "Пример работы", ln_str, 0xFF000000, 0x30C18);
  drawText(buffi, 0, 32, "Пример работы", ln_str, 0xFF000000, 0x1030C18);
  drawText(buffi, 0, 64, "Пример работы", ln_str, 0xFF000000, 0x2030C18);
  drawText(buffi, 0, 96, "Пример работы", ln_str, 0xFF000000, 0x4030C18);
  drawText(buffi, 0, 128, "Пример работы", ln_str, 0xFF000000, 0x8030C18);
  drawText(buffi, 0, 160, "Пример работы", ln_str, 0xFF000000, 0x0F031428);
  
  do  /* Start of main activity loop */
    {
      if(gui_event == KOLIBRI_EVENT_REDRAW)
	{
	  kolibri_handle_event_redraw(main_window);
	  DrawBitmap(buffi+8, 5, 5, 768, 256);
	}
      else if(gui_event == KOLIBRI_EVENT_KEY)
	{
	  key = get_key();
	  switch (key.code)
	  {
	  }
	  press_key = key.val;

	  kolibri_handle_event_key(main_window, key);
	}
      else if(gui_event == KOLIBRI_EVENT_BUTTON)
	{
	  unsigned int pressed_button = kolibri_button_get_identifier();
	  switch (pressed_button)
	  {
	      case BUTTON_CLOSE:
			kolibri_exit();
	  }
	 }
      else if(gui_event == KOLIBRI_EVENT_MOUSE)
	{
	  kolibri_handle_event_mouse(main_window);
	}

    } while((gui_event = get_os_event())); /* End of main activity loop */

  /* kolibri_quit(); */

  return 0;
}
