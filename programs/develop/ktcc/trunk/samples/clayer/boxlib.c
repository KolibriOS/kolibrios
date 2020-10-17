// WRITED BY MAXCODEHACK
// Need to other sample, this sample is very shitcode)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "kos32sys1.h"
#include <clayer/boxlib.h>

struct kolibri_system_colors sys_color_table;

char statusbar[255];
char proc_info[1024];
char text_line[255];

enum BUTTONS
{
    BTN_QUIT = 1,
    BTN_POP = 10,
    BTN_UNLOCK = 11
};

#define FONT_W 8
#define FONT_H 14
#define LINES 10

void draw_window()
{
    int win_hight, win_width, i, pos_y = get_skin_height() + 36;  // 60 == 24+36

    // start redraw
    begin_draw();
	// define&draw window
	sys_create_window(10, 40, 600, 400, "My window", /*sys_color_table.work_area*/0xFFFFFF, 0x13);

    // end redraw
    end_draw();
}
scrollbar scroll_ver = {15, 100 - 26, 100 - 29, 0, 0, 2 /* type */, 115, 15, 0,0x353B47,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};
int main()
{
    kolibri_boxlib_init();
    int gui_event;
    uint32_t pressed_button = 0, mouse_button;
    pos_t   mouse_pos;

    get_system_colors(&sys_color_table);
    set_event_mask(0xC0000027); // mouse events only when focused window and mouse inside

    do
    {
        gui_event = get_os_event(); 
        switch(gui_event)
        {
        case KOLIBRI_EVENT_NONE:
            // background work
			break;
        case KOLIBRI_EVENT_REDRAW:
            draw_window();
            scrollbar_v_draw(&scroll_ver);
			break;
        case KOLIBRI_EVENT_KEY:
            // scroll
			break;
        case KOLIBRI_EVENT_BUTTON:
            
            break;
        case KOLIBRI_EVENT_MOUSE:
            scrollbar_v_mouse(&scroll_ver);
            break;
        }
    } while(1);

  return 0;
}
