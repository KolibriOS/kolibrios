#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

ksys_colors_table_t sys_color_table;

char statusbar[255];
ksys_thread_t proc_info;
char text_line[255];

enum BUTTONS {
    BTN_QUIT = 1,
    BTN_POP = 10,
    BTN_UNLOCK = 11
};

#define FONT_W 8
#define FONT_H 14
#define LINES 10

void draw_window()
{
    int win_hight, win_width, i, pos_y = _ksys_get_skin_height() + 36; // 60 == 24+36

    // start redraw
    _ksys_start_draw();
    // define&draw window
    _ksys_create_window(10, 40, 600, 400, "My window", sys_color_table.work_area, 0x13);
    _ksys_thread_info(&proc_info, -1);

        win_width
        = proc_info.winx_size;
    win_hight = proc_info.winy_size;

    _ksys_define_button(10, 30, 70, 20, BTN_POP, sys_color_table.work_button);
    _ksys_draw_text("BUTTON1", 15, 34, 0, 0x90000000 | sys_color_table.work_button_text); // 0x80000000 asciiz
    _ksys_define_button(100, 30, 80, 20, BTN_UNLOCK, sys_color_table.work_button);
    _ksys_draw_text("BUTTTON2", 110, 34, 0, 0x90000000 | sys_color_table.work_button_text);

    // display statusbar
    _ksys_draw_bar(6, win_hight - 17, win_width - 11, 12, 0x80000000 | sys_color_table.work_area); // 0x80000000 gradient
    _ksys_draw_text(statusbar, 10, win_hight - 15, 0, 0x80000000 | sys_color_table.work_text);

    // display strings
    for (i = LINES; i > 0; i--) {
        snprintf(text_line, sizeof text_line, "Line[%d]<<Just a text>>", i);

        text_line[(win_width - 10 - 5) / FONT_W + 1] = '\0'; // clip text size, seems to big lines crashing OS, and form len by window size
        _ksys_draw_text(text_line, 5, pos_y, 0, 0x90000000 | sys_color_table.work_text);
        pos_y += FONT_H;

        if (pos_y + 29 > win_hight)
            break; // 12 font + 12 statusbar + 5 border
    }
    // end redraw
    _ksys_end_draw();
}

int main()
{
    int gui_event;
    uint32_t pressed_button = 0, mouse_button;
    ksys_pos_t mouse_pos;
    strcpy(statusbar, "Program running...Double click on TEXT for details");

    _ksys_get_system_colors(&sys_color_table);
    _ksys_set_event_mask(0xC0000027); // mouse events only when focused window and mouse inside

    do {
        gui_event = _ksys_get_event();
        switch (gui_event) {
        case KSYS_EVENT_NONE:
            break;
        case KSYS_EVENT_REDRAW:
            draw_window();
            break;
        case KSYS_EVENT_KEY:
            break;
        case KSYS_EVENT_BUTTON:
            pressed_button = _ksys_get_button();
            switch (pressed_button) {
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
        case KSYS_EVENT_MOUSE:
            mouse_pos = _ksys_get_mouse_pos(KSYS_MOUSE_WINDOW_POS); // window relative
            mouse_button = _ksys_get_mouse_eventstate();
            debug_printf("mouse ev (%d,%d)%x\n", mouse_pos.x, mouse_pos.y, mouse_button);
            if (mouse_button & (1 << 24)) // double click
            {
                int n = (mouse_pos.y - 60) / FONT_H;
                if (n < 0 || n >= LINES)
                    break;
                debug_printf("click on str(%d), clip slot(%d)\n", n, LINES - n - 1);
                sprintf(statusbar, "click on str(%d), clip slot(%d)\n", n, LINES - n - 1);
                draw_window();
            }
            break;
        }
    } while (1); /* End of main activity loop */

    return 0;
}
