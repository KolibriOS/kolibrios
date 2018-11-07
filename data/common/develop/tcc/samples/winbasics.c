/*
	newlib-style window example
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "kos32sys1.h"

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

    get_proc_info(proc_info);
    win_width = *(int*)(proc_info + 0x3E); // client, 2A windows
    win_hight = *(int*)(proc_info + 0x42); // client, 2E windows

	define_button((10 << 16) + 80, (30 << 16) + 20, BTN_POP, sys_color_table.work_button);
    draw_text_sys("BUTTON1", 15, 34, 0, 0x90000000 | sys_color_table.work_button_text);  //0x80000000 asciiz

	define_button((100 << 16) + 100, (30 << 16) + 20, BTN_UNLOCK, sys_color_table.work_button);
    draw_text_sys("BUTTTON2", 110, 34, 0, 0x90000000 | sys_color_table.work_button_text);

    // display statusbar
    draw_bar(6, win_hight - 17, win_width - 11, 12, 0x80000000 | sys_color_table.work_area);  //0x80000000 gradient
    draw_text_sys(statusbar, 10, win_hight - 15, 0, 0x80000000 | sys_color_table.work_text);

    // display strings
    for (i = LINES; i > 0; i--)
    {
        tiny_snprintf (text_line, sizeof text_line, "Line[%d]<<Just a text>>", i);

        text_line[(win_width - 10 - 5) / FONT_W + 1] = '\0'; // clip text size, seems to big lines crashing OS, and form len by window size
//        draw_number_sys(nbytes, 5, pos_y, 6, 0x10000000); 8x12 font
        draw_text_sys(text_line, 5, pos_y, 0, 0x90000000 /*| sys_color_table.work_text*/);
        pos_y += FONT_H;

        if(pos_y + 29 > win_hight) break; // 12 font + 12 statusbar + 5 border
    }

    // end redraw
    end_draw();
}

int main()
{
    int gui_event;
    uint32_t pressed_button = 0, mouse_button;
    pos_t   mouse_pos;
    strcpy(statusbar, "Program running...Double click on TEXT for details");

    get_system_colors(&sys_color_table);
    set_event_mask(0xC0000027); // mouse events only when focused window and mouse inside

    do  /* Start of main activity loop */
    {
//        gui_event = wait_for_event(10); // 100 = 1 sec, case you have background work
        gui_event = get_os_event(); 
        switch(gui_event)
        {
        case KOLIBRI_EVENT_NONE:
            // background work
			break;
        case KOLIBRI_EVENT_REDRAW:
            draw_window();
			break;
        case KOLIBRI_EVENT_KEY:
            // scroll
			break;
        case KOLIBRI_EVENT_BUTTON:
            pressed_button = get_os_button();
            switch (pressed_button)
            {
              case BTN_POP:
                strcpy(statusbar, "POP pressed....");
                draw_window();
                break;
              case BTN_UNLOCK:
                strcpy(statusbar, "UNLOCK pressed....");
                draw_window();
                break;
              case BTN_QUIT:
                return 0;
                break;
            }
            break;
        case KOLIBRI_EVENT_MOUSE:
            mouse_pos = get_mouse_pos(POS_WINDOW); // window relative
            mouse_button = get_mouse_eventstate();
            debug_board_printf("mouse ev (%d,%d)%x\n", mouse_pos.x, mouse_pos.y, mouse_button);
            if (mouse_button & (1<<24)) // double click
            {
                int n = (mouse_pos.y - 60) / FONT_H;
                if (n < 0 || n >= LINES) break;
                debug_board_printf("click on str(%d), clip slot(%d)\n", n, LINES - n - 1);
                tiny_sprintf(statusbar, "click on str(%d), clip slot(%d)\n", n, LINES - n - 1);
		draw_window();
            }
            // ignore
            break;
        }
    } while(1) ; /* End of main activity loop */

  return 0;
}



void __attribute__ ((noinline)) debug_board_write_str(const char* str){
  while(*str)
    debug_board_write_byte(*str++);
}

void __attribute__ ((noinline)) debug_board_printf(const char *format,...)
{
        va_list ap;
        char log_board[300];

        va_start (ap, format);
        tiny_vsnprintf(log_board, sizeof log_board, format, ap);
        va_end(ap);
        debug_board_write_str(log_board);

}
